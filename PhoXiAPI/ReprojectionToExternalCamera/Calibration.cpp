#include "Calibration.h"

#include "Utils/Scanner.h"
#include "Utils/Util.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

namespace reprojectionToExternalCamera {

pho::api::AdditionalCameraCalibration calibrate(
        pho::api::PPhoXi device,
        const std::vector<pho::api::Texture32f>& mainDeviceImages,
        const std::vector<pho::api::Texture32f>& externalDeviceImages,
        const CalibrationSettings& settings) {
    std::cout << std::endl << std::endl;
    std::cout << "Starting calibration..." << std::endl;

    pho::api::AdditionalCameraCalibration calibration;
    pho::api::AdditionalCamera::Calibrator Calibrator(device);

    std::string errorMessage;
    bool ok = Calibrator.Calibrate(mainDeviceImages, externalDeviceImages, settings.focalLength,
            settings.pixelSize, settings.markersPositionsPath, calibration, &errorMessage);

    if (ok) {
        return calibration;
    } else {
        throw CalibrationError(errorMessage);
    }
}


/**
 * Perform the calibration and save the calibration result in the project dir.
 *
 * Used by calibrateInteractive.
 */
void calibrateAndSave(
        pho::api::PPhoXi device,
        const CalibrationSettings &settings,
        std::vector<pho::api::Texture32f> mainDeviceImages,
        std::vector<pho::api::Texture32f> externalDeviceImages) {
    try {
        auto calibration = calibrate(device, mainDeviceImages, externalDeviceImages, settings);
        std::cout << "External camera calibration was successfull!" << std::endl;
        calibration.SaveToFile(utils::Path::join(utils::Path::projectFolder(), "calibration.txt"));
    }
    catch (CalibrationError& e) {
        std::cout << "External camera calibration was NOT successfull!" << std::endl;
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

// Set maximum resolution and LED texture source to have the best calibration results
void prepareDevice(pho::api::PPhoXi& device) {
    const bool motionCam = device->GetType() == pho::api::PhoXiDeviceType::MotionCam3D;
    if (motionCam) {
        device->MotionCam->OperationMode = pho::api::PhoXiOperationMode::Scanner;
        device->MotionCamScannerMode->TextureSource = pho::api::PhoXiTextureSource::LED;
    } else {
        const auto& supportedCapturingModes = device->SupportedCapturingModes.GetValue();
        if (!supportedCapturingModes.empty()) {
            device->CapturingMode = supportedCapturingModes[0];
        }
        device->CapturingSettings->TextureSource = pho::api::PhoXiTextureSource::LED;
    }
    device->CoordinatesSettings->CameraSpace = pho::api::PhoXiCameraSpace::PrimaryCamera;
    device->CoordinatesSettings->RecognizeMarkers = false;
}

void calibrateInteractive(
        pho::api::PhoXiFactory& factory) {
    CalibrationSettings settings;
    settings.markersPositionsPath = utils::Path::join(utils::Path::settingsFolder(), "MarkersPositions.txt");
    {
        // Check if marker positions file exists before interactive scanning
        std::ifstream file(settings.markersPositionsPath);
        if (!file) {
            std::cout << "'MarkersPositions.txt' file doesn't exist!" << std::endl;
            std::cout << "Application expects the 'MarkersPositions.txt' file to be on "
                "this path: " << settings.markersPositionsPath << std::endl;
            return;
        }
    }

    std::cout << std::endl;
    auto device = utils::selectAndConnectDevice(factory, "main");
    prepareDevice(device);

    std::cout << std::endl;
    auto extDevice = utils::selectAndConnectDevice(factory, "external");
    prepareDevice(extDevice);

    settings.focalLength = extDevice->CalibrationSettings->FocusLength;
    settings.pixelSize = extDevice->CalibrationSettings->PixelSize.Width;

    std::vector<pho::api::Texture32f> mainDeviceImages;
    std::vector<pho::api::Texture32f> externalDeviceImages;

    // NOTE: The constant is copied from Camera3DApp::CalibrateAdditionalCamera
    const std::size_t minimalImagesCount = 5;
    auto shouldContinue = [&mainDeviceImages, minimalImagesCount]() -> bool {
        if (mainDeviceImages.size() < minimalImagesCount) {
            return 1 == utils::ask<int>("What do you want to do next?", {
            {1, "Trigger a new scan from both cameras (" +
                    std::to_string(minimalImagesCount - mainDeviceImages.size()) + " more scans needed)"},
            {2, "Cancel the calibration."}
        });
        }
        return 1 == utils::ask<int>("What do you want to do next?", {
            {1, "Trigger an additional scan from both cameras"},
            {2, "Compute the calibration"}
        });
    };

    while (shouldContinue()) {
        // Get one frame from scanner and image from external camera
        mainDeviceImages.push_back(utils::triggerScanAndGetFrame(device)->Texture);
        externalDeviceImages.push_back(utils::triggerScanAndGetFrame(extDevice)->Texture);
    }

    if (mainDeviceImages.size() < minimalImagesCount) {
        std::cout << "Calibration canceled." << std::endl;
    } else {
        calibrateAndSave(device, settings, mainDeviceImages, externalDeviceImages);
    }

    utils::disconnectOrLogOut(device);
    utils::disconnectOrLogOut(extDevice);
}

} // namespace reprojectionToExternalCamera

#include "Reprojection.h"

#include "Utils/Calibration.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"

#include <PhoXiAdditionalCamera.h>

namespace reprojectionToExternalCamera {

void reprojectionInteractive(
        pho::api::PhoXiFactory& factory) {
    // Load calibration info
    auto calibration = utils::loadCalibration();
    utils::printCalibration(calibration);

    const auto &externalToDepthCamTransformation = calibration.CoordinateTransformation;
    const auto depthToExternalCamTransformation = utils::invert(externalToDepthCamTransformation);

    // Connect to a scanner
    auto device = utils::selectAndConnectDevice(factory);

    device->CoordinatesSettings->CameraSpace = pho::api::PhoXiCameraSpace::CustomCamera;
    if (device->CoordinatesSettings->CameraSpace != pho::api::PhoXiCameraSpace::CustomCamera) {
        std::cout << "ERROR: Selected device doesn't support reprojection to CustomCamera (FW v. 1.14.0 is needed, " <<
            device->Info().FirmwareVersion << " is actual)" << std::endl;
        return;
    }

    // Set up the Custom Camera
    device->CoordinatesSettings->RecognizeMarkers = false;
    device->CoordinatesSettings->CustomCamera.Resolution =
        calibration.CameraResolution;
    device->CoordinatesSettings->CustomCamera.ProjectionMode =
        pho::api::PhoXiProjectionMode::Perspective;
    device->CoordinatesSettings->CustomCamera.PerspectiveSettings.CameraMatrix =
        calibration.CalibrationSettings.CameraMatrix;
    device->CoordinatesSettings->CustomCamera.PerspectiveSettings.DistortionCoefficients =
        calibration.CalibrationSettings.DistortionCoefficients;
    device->CoordinatesSettings->CustomCamera.WorldToCameraCoordinates =
        depthToExternalCamTransformation;

    auto shouldContinue = []() {
        return  1 == utils::ask<int>("Do you want to continue?", {
            {1, "Yes, trigger a new scan"},
            {2, "No, disconnect the scanner"}
        });
    };

    while (shouldContinue()) {
        // NOTE: You can read the frame returned from following function
        // or just see it in PhoXiControl
        utils::triggerScanAndGetFrame(device);
        std::cout << "You can see the scan in the PhoXiControl" << std::endl;
        std::cout << std::endl;
    }

    utils::disconnectOrLogOut(device);
}

} // namespace reprojectionToExternalCamera

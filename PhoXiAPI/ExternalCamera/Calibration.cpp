#include "Calibration.h"

#include "ExternalCamera.h"
#include "Utils/FileCamera.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

namespace externalCamera {

CalibrationSettings CalibrationSettings::load() {
    CalibrationSettings settings;
    std::ifstream focalLengthFile(utils::Path::join(utils::Path::settingsFolder(), "FocalLength.txt"));
    if (focalLengthFile.is_open()) {
        auto line = std::string();
        std::getline(focalLengthFile, line);
        settings.focalLength = std::stod(line);
    }
    std::cerr << "FocalLength " << settings.focalLength << "\n";

    std::ifstream pixelSizeFile(utils::Path::join(utils::Path::settingsFolder(), "PixelSize.txt"));
    if (pixelSizeFile.is_open()) {
        auto line = std::string();
        std::getline(pixelSizeFile, line);
        settings.pixelSize = std::stod(line);
    }

    settings.markersPositions = utils::Path::join(utils::Path::settingsFolder(), "MarkersPositions.txt");

    std::cout << "Loaded following settings:" << std::endl;
    std::cout << "	Focal length: "
            << settings.focalLength << " mm" << std::endl;
    std::cout << "	Pixel size: "
            << settings.pixelSize << " mm" << std::endl;
    std::cout << "	Markers positions: "
            << settings.markersPositions << std::endl << std::endl;

    return settings;
}

pho::api::AdditionalCameraCalibration calibrate(
        pho::api::PPhoXi device,
        const std::vector<pho::api::Texture32f>& frames,
        const std::vector<pho::api::Texture32f>& images,
        const CalibrationSettings& settings) {
    std::cout << std::endl << std::endl;
    std::cout << "Starting calibration..." << std::endl;

    pho::api::AdditionalCameraCalibration calibration;
    pho::api::AdditionalCamera::Calibrator Calibrator(device);

    std::string errorMessage;
    bool ok = Calibrator.Calibrate(frames, images, settings.focalLength,
            settings.pixelSize, settings.markersPositions, calibration, &errorMessage);

    if (ok) {
        return calibration;
    } else {
        throw CalibrationError(errorMessage);
    }
}


pho::api::Texture32f loadTexture32f(std::string name) {
    const auto cvImage = cv::imread(name, cv::IMREAD_GRAYSCALE);
    if (cvImage.empty())
        throw std::runtime_error("Error loading image " + name);

    pho::api::Texture32f texture;
    pho::api::ConvertOpenCVMatToMat2D(cvImage, texture);
    return texture;
}

/**
 * Perform the calibration and save the calibration result
 * in the project dir.
 *
 * Used by calibrateFromFiles and calibrateInteractive.
 */
void calibrateAndSave(
        pho::api::PPhoXi device,
        std::vector<pho::api::Texture32f> frames,
        std::vector<pho::api::Texture32f> images) {
    // Load calibration settings from the Settings directory
    auto settings = CalibrationSettings::load();

    try {
        auto calibration = calibrate(device, frames, images, settings);
        std::cout << "External camera calibration was successfull!" << std::endl;
        calibration.SaveToFile(utils::Path::join(utils::Path::projectFolder(), "calibration.txt"));
    }
    catch (CalibrationError& e) {
        std::cout << "External camera calibration was NOT successfull!" << std::endl;
        std::cout << "Exception: " << e.what() << std::endl;
    }
}


/**
 * Calibrate external camera from a set of files
 */
void calibrateFromFiles(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]) {
    // string::startsWith and ::endsWith would be nice, but...
    std::regex prawRe("\\.praw$");
    std::regex frameRe("\\bframe");
    std::regex imageRe("\\bimage");

    std::vector<std::string> prawNames;
    std::vector<std::string> frameNames;
    std::vector<std::string> imageNames;

    std::for_each(argv, argv + argc, [&](const char* name) {
        if (std::regex_search(name, prawRe)) {
            prawNames.push_back(name);
        } else if (std::regex_search(name, frameRe)) {
            frameNames.push_back(name);
        } else if (std::regex_search(name, imageRe)) {
            imageNames.push_back(name);
        } else {
            throw std::runtime_error(std::string("Unknown file: ") + name);
        }
    });

    if (frameNames.size() != imageNames.size())
        throw std::runtime_error("Need an equal number of frame and image files");
    if (frameNames.empty())
        throw std::runtime_error("Need at least one frame / image pair");


    // Load frames and images
    std::vector<pho::api::Texture32f> frames;
    std::transform(frameNames.begin(), frameNames.end(),
            std::back_inserter(frames), loadTexture32f);

    std::vector<pho::api::Texture32f> images;
    std::transform(imageNames.begin(), imageNames.end(),
            std::back_inserter(images), loadTexture32f);

    // Use Data/1.praw if no praw file specified
    if (!prawNames.size())
        prawNames.push_back(utils::Path::join(utils::Path::projectFolder(), "1.praw"));

    // Attach praw file as FileCamera
    utils::AttachedFileCamera fileCamera{factory, prawNames};
    auto device = fileCamera.connect();

    calibrateAndSave(device, frames, images);

    // Log out the device from PhoXi Control
    device->Disconnect(true);
}

/**
 * Interactively get frames / images from a connected scanner and external
 * camera and perform calibration on them.
 */
void calibrateInteractive(
        pho::api::PhoXiFactory& factory) {
    // Connect to a scanner
    auto device = utils::selectAndConnectDevice(factory);

    // Iinitialize external camera
    auto extCamera = ExternalCamera();

    std::vector<pho::api::Texture32f> frames;
    std::vector<pho::api::Texture32f> images;

    auto shouldContinue = []() {
        return  1 == utils::ask<int>("Do you want to continue?", {
            {1, "Yes, trigger a new scan a get image from external camera"},
            {2, "No, compute the calibration"}
        });
    };

    while (shouldContinue()) {
        // Get one frame from scanner and image from external camera
        frames.push_back(utils::triggerScanAndGetFrame(device)->Texture);
        images.push_back(extCamera.getCalibrationImage());
    }

    calibrateAndSave(device, frames, images);

    utils::disconnectOrLogOut(device);
}

} // namespace externalCamera

#include "Calibration.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <regex>
#include <PhoXiOpenCVSupport.h>
#include "ExternalCamera.h"
#include "Utils/FileCamera.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"

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

pho::api::AdditionalCameraCalibration loadCalibration() {
    const auto calibrationPath = utils::Path::join(utils::Path::projectFolder(), "calibration.txt");

    if (!utils::Path::readable(calibrationPath))
        throw MissingCalibrationFile(
                "Calibration file not found", calibrationPath);

    pho::api::AdditionalCameraCalibration calibration;
    calibration.LoadFromFile(calibrationPath);

    auto isCorrect = true
            && calibration.CalibrationSettings.DistortionCoefficients.size() > 4
            && calibration.CameraResolution.Width != 0
            && calibration.CameraResolution.Height != 0;

    if (!isCorrect)
        throw BadCalibrationFile(
                "Calibration information is incorrect", calibrationPath);
    return calibration;
}

template <typename T>
std::ostream& printMat2D(std::ostream& os, const pho::api::Mat2D<T>& mat) {
    for (int32_t y = 0; y < mat.Size.Height; ++y) {
        for (int32_t x = 0; x < mat.Size.Width; ++x)
            std::cout << std::to_string(mat[y][x]) << " ";
        std::cout << std::endl;
    }
    return os;
}

void printCalibration(
        const pho::api::AdditionalCameraCalibration& calibration) {
    std::cout << "Calibration:" << std::endl;
    std::cout << "Camera Matrix" << std::endl;
    printMat2D(std::cout, calibration.CalibrationSettings.CameraMatrix);
    std::cout << std::endl;

    std::cout << "Distortion Coefficients" << std::endl;
    for (const auto c :
            calibration.CalibrationSettings.DistortionCoefficients) {
        std::cout << std::to_string(c) << " ";
    }
    std::cout << std::endl;

    std::cout << "Rotation Matrix" << std::endl;
    printMat2D(std::cout, calibration.CoordinateTransformation.Rotation);
    std::cout << std::endl;

    std::cout << "Translation vector" << std::endl;
    {
        const auto& t = calibration.CoordinateTransformation.Translation;
        std::cout << std::to_string(t.x)
            << " " << std::to_string(t.y)
            << " " << std::to_string(t.z)
            << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Camera Resolution" << std::endl;
    {
        const auto& res = calibration.CameraResolution;
        std::cout << std::to_string(res.Width) << "x" << std::to_string(res.Height);
    }
    std::cout << std::endl << std::endl;
}

void printCalibrationError(const BadCalibrationFile& e) {
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Calibration saved in 'calibration.txt' is incorrect." << std::endl;
    std::cout << "Please make sure you have correct settings in 'Settings' "
            "folder and the 'MarkersPositions.txt' contains path to the file "
            "containing markers positions." << std::endl;
    std::cout << "After you have correct settings, start calibration again"
            "and upon success start the testing of depth map." << std::endl;
}

void printCalibrationError(const MissingCalibrationFile& e) {
    std::cout << "Calibration file 'calibration.txt' is missing." << std::endl;
    std::cout << "Application expects the 'calibration.txt' file to be on "
            "this path: " << e.path << std::endl;
    std::cout << "CMake settings may change where the 'calibration.txt' "
            "file is created. Change CMake settings or copy 'calibration.txt' "
            "to the aformentioned path and restart the testing of depth map."
            << std::endl;
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

    bool ok = Calibrator.Calibrate(frames, images, settings.focalLength,
            settings.pixelSize, settings.markersPositions, calibration);

    if (ok) {
        return calibration;
    } else {
        throw CalibrationError("Camera calibration failed");
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
        std::cout << "External camera calibration was NOT successfull! exception:" << e.what() << std::endl;
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

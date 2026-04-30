#pragma once
#define PHOXI_OPENCV_SUPPORT
#include <PhoXi.h>
#include <PhoXiAdditionalCamera.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace externalCamera {

struct CalibrationSettings {
    double focalLength = 0;
    double pixelSize = 0;
    std::string markerBoardRecipeFilePath;

    static CalibrationSettings load();
};

struct CalibrationError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * Perform the actual calibration.
 *
 * @param device the PhoXi device to perform the calibration on
 * @param frames a list of frames from the scanner
 * @param images a list of images from the external camera
 * @param settings  calibration settings loaded from the Settings folder
 *
 * @returns the calculated calibration
 *
 * Throws CalibrationError if the calibration is not successful.
 */
pho::api::AdditionalCameraCalibration calibrate(
        pho::api::PPhoXi device,
        const std::vector<pho::api::Texture32f>& frames,
        const std::vector<pho::api::Texture32f>& images,
        const CalibrationSettings& settings);

/**
 * Perform calibration on files specified on commandline
 *
 * @param factory the PhoXi Factory used to create devices
 * @param argc,argv the rest of the commandline containing the calibration files
 */
void calibrateFromFiles(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]);

/**
 * Interactively get frames / images from a connected scanner and external
 * camera and perform calibration on them.
 *
 * @param factory the PhoXi Factory used to create devices
 */
void calibrateInteractive(
        pho::api::PhoXiFactory& factory);

}  // namespace externalCamera

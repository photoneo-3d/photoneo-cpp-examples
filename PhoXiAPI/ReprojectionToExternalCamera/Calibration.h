#pragma once

#include <PhoXi.h>
#include <PhoXiAdditionalCamera.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace reprojectionToExternalCamera {

struct CalibrationSettings {
    double focalLength = 0;
    double pixelSize = 0;
    std::string markersPositionsPath;
};

struct CalibrationError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * Perform the actual calibration.
 *
 * @param device the PhoXi device to perform the calibration on
 * @param mainDeviceImages a list of images from the main camera
 * @param externalDeviceImages a list of images from the external camera
 * @param settings  calibration settings loaded from the Settings folder
 *
 * @returns the calculated calibration
 *
 * Throws CalibrationError if the calibration is not successful.
 */
pho::api::AdditionalCameraCalibration calibrate(
        pho::api::PPhoXi device,
        const std::vector<pho::api::Texture32f>& mainDeviceImages,
        const std::vector<pho::api::Texture32f>& externalDeviceImages,
        const CalibrationSettings& settings);

/**
 * Interactively get images from the main and the external camera
 * and perform calibration on them.
 *
 * @param factory the PhoXi Factory used to create devices
 */
void calibrateInteractive(
        pho::api::PhoXiFactory& factory);

}  // namespace reprojectionToExternalCamera

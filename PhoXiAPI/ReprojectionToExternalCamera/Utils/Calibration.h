#pragma once

#include <PhoXiAdditionalCamera.h>

namespace utils {

struct FileError : public std::runtime_error {
    FileError(const std::string& what, const std::string& path_)
        : std::runtime_error(what)
        , path(path_)
    {
    }

    std::string path;
};

struct IncorrectCalibrationFile : public FileError {
    using FileError::FileError;
};

struct MissingCalibrationFile : public FileError {
    using FileError::FileError;
};

/**
 * Load (already calculated) calibration from the project folder.
 *
 * @returns the loaded calibration
 *
 * Throws one IncorrectCalibrationFile or MissingCalibrationFile error
 * according to the problem with the calibration file.
 */
pho::api::AdditionalCameraCalibration loadCalibration();

void printCalibration(const pho::api::AdditionalCameraCalibration& calibration);

void printCalibrationError(const IncorrectCalibrationFile& e);

void printCalibrationError(const MissingCalibrationFile& e);

} // namespace utils

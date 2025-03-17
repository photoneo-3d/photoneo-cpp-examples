#include "Calibration.h"

#include <iostream>

#include "Util.h"

namespace utils {

pho::api::AdditionalCameraCalibration loadCalibration() {
    const auto calibrationPath = utils::Path::join(utils::Path::projectFolder(), "calibration.txt");

    if (!utils::Path::readable(calibrationPath))
        throw MissingCalibrationFile(
                "Calibration file not found", calibrationPath);

    pho::api::AdditionalCameraCalibration calibration;
    calibration.LoadFromFile(calibrationPath);

    const bool isCorrect =
            calibration.CalibrationSettings.DistortionCoefficients.size() > 4
            && calibration.CameraResolution.Width != 0
            && calibration.CameraResolution.Height != 0;

    if (!isCorrect)
        throw IncorrectCalibrationFile(
                "Calibration information is incorrect", calibrationPath);
    return calibration;
}

void printCalibration(
        const pho::api::AdditionalCameraCalibration& calibration) {
    std::cout << "Calibration:" << std::endl;
    std::cout << "Camera Matrix" << std::endl;
    std::cout << calibration.CalibrationSettings.CameraMatrix << std::endl;

    std::cout << "Distortion Coefficients" << std::endl;
    for (const auto c :
            calibration.CalibrationSettings.DistortionCoefficients) {
        std::cout << std::to_string(c) << " ";
    }
    std::cout << std::endl << std::endl;

    std::cout << "Rotation Matrix" << std::endl;
    std::cout << calibration.CoordinateTransformation.Rotation << std::endl;

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

void printCalibrationError(const IncorrectCalibrationFile& e) {
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

} // namespace utils

#include "Util.h"

#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#elif defined (__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

namespace utils {

std::string Path::projectFolderPath;

#ifdef __linux__
    std::string getExecutablePath() {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::string(result, (count > 0) ? count : 0);
    }
#else
    std::string getExecutablePath() {
        char rawPathName[MAX_PATH];
        GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
        return std::string(rawPathName);
    }
#endif

const std::string& Path::setProjectFolder(std::string path) {
    if (path.empty()) {
        path = getExecutablePath();
        projectFolderPath = path.substr(0, path.find_last_of(delimiter()));
    }
    else {
        projectFolderPath = std::move(path);
    }
    std::cout << "Project directory identified as: " << projectFolder() << std::endl;

    return projectFolderPath;
}

bool Path::readable(const std::string& path) {
    std::ifstream stream(path);
    return stream.good();
}

pho::api::Point3_64f multiply(
        const pho::api::RotationMatrix64f &rotationMatrix, const pho::api::Point3_64f &vector) {
    pho::api::Point3_64f result;
    result.x = rotationMatrix[0][0] * vector.x +
               rotationMatrix[0][1] * vector.y +
               rotationMatrix[0][2] * vector.z;
    result.y = rotationMatrix[1][0] * vector.x +
               rotationMatrix[1][1] * vector.y +
               rotationMatrix[1][2] * vector.z;
    result.z = rotationMatrix[2][0] * vector.x +
               rotationMatrix[2][1] * vector.y +
               rotationMatrix[2][2] * vector.z;
    return result;
}

pho::api::PhoXiCoordinateTransformation invert(
        const pho::api::PhoXiCoordinateTransformation &input) {
    pho::api::PhoXiCoordinateTransformation result;

    // Transpose the rotation part
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            result.Rotation[y][x] = input.Rotation[x][y];
        }
    }

    // Apply the transposed rotation to the translation vector and negate the result
    const pho::api::Point3_64f negativeTranslation = multiply(result.Rotation, input.Translation);
    result.Translation.x = -negativeTranslation.x;
    result.Translation.y = -negativeTranslation.y;
    result.Translation.z = -negativeTranslation.z;

    return result;
}

pho::api::PhoXiCoordinateTransformation compose(
        const pho::api::PhoXiCoordinateTransformation &a,
        const pho::api::PhoXiCoordinateTransformation &b) {
    pho::api::PhoXiCoordinateTransformation result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.Rotation[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                result.Rotation[i][j] += a.Rotation[i][k] * b.Rotation[k][j];
            }
        }
    }
    result.Translation = multiply(a.Rotation, b.Translation);
    result.Translation.x += a.Translation.x;
    result.Translation.y += a.Translation.y;
    result.Translation.z += a.Translation.z;
    return result;
}

} // namespace utils

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

} // namespace utils

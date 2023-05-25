#pragma once

#include <PhoXi.h>

#include <iostream>
#include <string>

namespace utils {

class Path {

private:
    static std::string projectFolderPath;

    static inline std::string delimiter()
    {
    #ifdef __linux__
        return "/";
    #endif
    #ifdef _WIN32
        return "\\";
    #endif
    #ifdef _WIN64
        return "\\";
    #endif
        return "/";
    }

public:

    static inline std::string join(const std::string& p1, const std::string& p2)
    {
        return p1 + delimiter() + p2;
    }

    static inline std::string projectFolder()
    {
        if (projectFolderPath.empty()) {
            throw std::runtime_error(
                "Please call utils::Path::setProjectFolder(); in the main(...)");
        }

        return projectFolderPath;
    }

    static inline std::string dataFolder()
    {
        if (projectFolderPath.empty()) {
            throw std::runtime_error(
                "Please call utils::Path::setProjectFolder(); in the main(...)");
        }

        return join(projectFolderPath, "Data");
    }

    static inline std::string settingsFolder()
    {
        if (projectFolderPath.empty()) {
            throw std::runtime_error(
                "Please call utils::Path::setProjectFolder(); in the main(...)");
        }

        return join(projectFolderPath, "Settings");
    }

    static const std::string& setProjectFolder(std::string path);

    static bool readable(const std::string& path);
};


template<typename Value>
struct AskEntry {
    Value value;
    std::string description;
};

/**
 * Print out multiple options and let the user select one of them.
 */
template <typename Value>
Value ask(
        const std::string& message,
        std::initializer_list<AskEntry<Value>> entries) {
    
    std::cout << message << std::endl;
    for (const auto& entry : entries) {
        std::cout << entry.value << "  " << entry.description << std::endl;
    }

    while (true) {
        Value value;
        if (std::cin >> value) {
            for (auto it = entries.begin(); it != entries.end(); ++it) {
                if (it->value == value)
                    return value;
            }
        }
    }
}


inline void saveFrameToPly(pho::api::PFrame& frame, const std::string& path) {
    if (frame->SaveAsPly(path)) {
        std::cout << "Saved frame as PLY to: " << path << std::endl;
    }
    else {
        std::cout << "Could not save frame as PLY to " << path << " !" << std::endl;
    }
}

} // namespace utils

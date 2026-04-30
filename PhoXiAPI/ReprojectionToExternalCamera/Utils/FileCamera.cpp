#include "FileCamera.h"

#include <iostream>

namespace utils {

AttachedFileCamera::AttachedFileCamera(
        pho::api::PhoXiFactory& factory_,
        const std::vector<std::string>& filePaths)
    : factory(factory_)
{
    std::cout << "Attaching FileCamera with files:" << std::endl;
    for(const auto& p : filePaths)
        std::cout << "\t" << p << std::endl;

    name = factory.AttachFileCamera(FileCameraHwIdentification, filePaths);
    if (name.empty())
        throw std::runtime_error("Error attaching FileCamera praw files");
}

AttachedFileCamera::~AttachedFileCamera() {
    if (!name.empty()) {
        std::cout << "Detaching FileCamera" << std::endl;
        if (!factory.DetachFileCamera(name))
            std::cout << "Failed to detach FileCamera" << std::endl;
    }
}

pho::api::PPhoXi AttachedFileCamera::connect() {
    return factory.CreateAndConnect(name, pho::api::PhoXiTimeout::Infinity);
}

}   // namespace utils

#include "Scanning.h"

#include "Utils/Scanner.h"


namespace jointMarkerSpace {

void savePointCloud(
        const std::string& path,
        const pho::api::PFrame frame) {
    if (!frame->SaveAsPly(
            path, true, true, true, true, true, true, true, true /*unordered*/, false /*metadata*/)) {
        throw std::runtime_error(
                "Failed saving point cloud to file " + path);
    }

    std::cout << "Point cloud saved to " << path << std::endl;
}

void trigAndSavePointClouds(
        pho::api::PPhoXi& primaryDevice, pho::api::PPhoXi& secondaryDevice, const int counter) {
    std::string filePrefix = "point_cloud_";
    std::string filePrefixPrimary = filePrefix + "primary_";
    std::string filePrefixSecondary = filePrefix + "secondary_";
    const std::string fileSuffix = std::to_string(counter) + ".ply";

    auto framePrimary = utils::triggerScanAndGetFrame(primaryDevice);
    auto frameSecondary = utils::triggerScanAndGetFrame(secondaryDevice);

    savePointCloud(filePrefixPrimary + fileSuffix, framePrimary);
    savePointCloud(filePrefixSecondary + fileSuffix, frameSecondary);
}

} // namespace jointMarkerSpace

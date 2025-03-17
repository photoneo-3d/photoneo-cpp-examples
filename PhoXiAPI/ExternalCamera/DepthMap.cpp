#include "DepthMap.h"

#include "Utils/Calibration.h"
#include "Utils/FileCamera.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"

#include <PhoXiAdditionalCamera.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace externalCamera {

pho::api::DepthMap32f getDepthMap(
        pho::api::PPhoXi device,
        const pho::api::AdditionalCameraCalibration& calibration) {
    pho::api::DepthMap32f depthMap;

    pho::api::AdditionalCamera::Aligner aligner(device, calibration);
    if (!aligner.GetAlignedDepthMap(depthMap))
        throw std::runtime_error("Computation of aligned depth map failed!");

    return depthMap;
}

void saveDepthMap(
        const std::string& path,
        const pho::api::DepthMap32f& depthMap) {
    cv::Mat cvDepthMap;
    pho::api::ConvertMat2DToOpenCVMat(depthMap, cvDepthMap);

    cv::Mat cvDepthMapInt;
    cvDepthMap.convertTo(cvDepthMapInt, CV_16U);
    if (!imwrite(path, cvDepthMapInt))
        throw std::runtime_error("Failed saving depth map to file " + path);

    std::cout << "Depth map saved to " << path << std::endl;
}

void depthMapFromFile(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]) {
    std::vector<std::string> prawNames;
    std::string outputPath = "fileCamera.tif";

    if (argc > 0)
        prawNames.push_back(argv[0]);

    if (argc > 1)
        outputPath = argv[1];

    // Use Data/1.praw if no praw file specified
    if (!prawNames.size())
        prawNames.push_back(
            utils::Path::join(utils::Path::dataFolder(), "1.praw"));

    // Load calibration info
    auto calibration = utils::loadCalibration();
    utils::printCalibration(calibration);

    // Attach praw file as FileCamera
    utils::AttachedFileCamera fileCamera{factory, prawNames};
    auto device = fileCamera.connect();

    // We just need to trigger a scan, but don't need the result, because the
    // aligner will use the data present in PhoXiControl
    utils::triggerScanAndGetFrame(device);

    auto depthMap = getDepthMap(device, calibration);
    saveDepthMap(outputPath, depthMap);

    // Log out the device from PhoXi Control
    device->Disconnect(true);
}

void depthMapInteractive(
        pho::api::PhoXiFactory& factory) {
    std::string filePrefix = "device_";
    int count = 0;

    // Load calibration info
    auto calibration = utils::loadCalibration();
    utils::printCalibration(calibration);

    // Connect to a scanner
    auto device = utils::selectAndConnectDevice(factory);

    auto shouldContinue = []() {
        return  1 == utils::ask<int>("Do you want to continue?", {
            {1, "Yes, trigger a new scan and compute depth map"},
            {2, "No, disconnect the scanner"}
        });
    };

    while (shouldContinue()) {
        // We just need to trigger a scan, but don't need the result, because
        // the aligner will use the data present in PhoXiControl
        utils::triggerScanAndGetFrame(device);
        auto depthMap = getDepthMap(device, calibration);

        auto outputPath = filePrefix + std::to_string(++count) + ".tif";
        saveDepthMap(outputPath, depthMap);
    }

    utils::disconnectOrLogOut(device);
}

} // namespace externalCamera

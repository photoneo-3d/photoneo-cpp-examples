#include <PhoXi.h>
#include <thread>
#include "Calibration.h"
#include "ColorPointCloud.h"
#include "DepthMap.h"
#include "Utils/Util.h"

namespace externalCamera {

void interactive(
        pho::api::PhoXiFactory& factory) {
    int i = 0;

    auto selectAction = []() {
        return utils::ask<int>(
                "Please select which action would you like to perform:",
                {
                        {1, "Calibration"},
                        {2, "Calculate DepthMap"},
                        {3, "Calculate Color Point Cloud Texture"},
                        {4, "Exit"}
                });
    };
    while (true) {
        i = selectAction();
        if (i == 4)
            break;
        switch (i) {
        case 1:
            calibrateInteractive(factory);
            break;
        case 2:
            depthMapInteractive(factory);
            break;
        case 3:
            colorPointCloudInteractive(factory);
            break;
        }
    }
    std::cout << "Exiting application..." << std::endl;
}

} // namespace externalCamera

int main(int argc, char *argv[]) {
    using namespace externalCamera;
    pho::api::PhoXiFactory factory;

    std::cout << "External Camera Example" << std::endl;
    std::cout << std::endl;

    std::cout << "Waiting for PhoXi Control" <<std::endl;
    while (!factory.isPhoXiControlRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "PhoXi Control Version: "
            << factory.GetPhoXiControlVersion() << std::endl;
    std::cout << "PhoXi API Version: "
            << factory.GetAPIVersion() << std::endl << std::endl;

    utils::Path::setProjectFolder("");

    try {
        if (argc > 1) {
            if (!strcmp("--calibrate", argv[1])) {
                calibrateFromFiles(
                        factory, argc - 2, argv + 2);
            }
            else if (!strcmp("--depthmap", argv[1])) {
                depthMapFromFile(
                        factory, argc - 2, argv + 2);
            }
            else if (!strcmp("--colorpc", argv[1])) {
                colorPointCloudFromFile(
                        factory, argc - 2, argv + 2);
            }
        } else {
            interactive(factory);
        }
    }
    catch (MissingCalibrationFile& e) {
        printCalibrationError(e);
    }
    catch (BadCalibrationFile& e) {
        printCalibrationError(e);
    }
    catch (std::runtime_error& e) {
        std::cout << "Error occured: " << std::endl;
        std::cout << "\t" << e.what() << std::endl;;
    }
    return 0;
}

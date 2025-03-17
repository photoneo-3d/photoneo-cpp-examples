#include <PhoXi.h>

#include "MarkerSpaceRecognition.h"
#include "Scanning.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"


namespace jointMarkerSpace {

void interactive(pho::api::PPhoXi& primaryDevice, pho::api::PPhoXi& secondaryDevice) {
    int pointCloudCounter = 0;
    while (true) {
        std::cout << std::endl;
        const int actionId = utils::ask<int>(
            "Please select which action would you like to perform:",
            {
                {1, "Marker Space Recognition"},
                {2, "Scanning"},
                {3, "Exit"}
            });
        if (actionId == 3) {
            break;
        }
        switch (actionId) {
        case 1:
            recognizeMarkerAndSetupDevices(primaryDevice, secondaryDevice);
            break;
        case 2:
            ++pointCloudCounter;
            trigAndSavePointClouds(primaryDevice, secondaryDevice, pointCloudCounter);
            break;
        }
    }
    std::cout << "Exiting application..." << std::endl;
}

} // namespace jointMarkerSpace

int main(int argc, char *argv[]) {
    using namespace jointMarkerSpace;
    pho::api::PhoXiFactory factory;

    std::cout << "Joint Marker Space Example" << std::endl;
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
        // Firstly, connect to two devices since all actions will be made with them
        auto devicePrimary = utils::selectAndConnectDevice(factory, "primary");
        auto deviceSecondary = utils::selectAndConnectDevice(factory, "secondary");

        interactive(devicePrimary, deviceSecondary);

        utils::disconnectOrLogOut(devicePrimary);
        utils::disconnectOrLogOut(deviceSecondary);
        return 0;
    }
    catch (std::runtime_error& e) {
        std::cout << "Error occured:" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
    }
    return 1;
}

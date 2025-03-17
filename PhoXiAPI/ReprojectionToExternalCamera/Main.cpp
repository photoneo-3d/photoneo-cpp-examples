#include <PhoXi.h>

#include "Calibration.h"
#include "Reprojection.h"
#include "Utils/Calibration.h"
#include "Utils/Util.h"

#include <thread>

namespace reprojectionToExternalCamera {

void interactive(pho::api::PhoXiFactory& factory) {
    while (true) {
        const int actionId = utils::ask<int>(
            "Please select which action would you like to perform:",
            {
                {1, "Calibration"},
                {2, "Reprojection"},
                {3, "Exit"}
            });
        if (actionId == 3) {
            break;
        }
        switch (actionId) {
        case 1:
            calibrateInteractive(factory);
            break;
        case 2:
            reprojectionInteractive(factory);
            break;
        }
    }
    std::cout << "Exiting application..." << std::endl;
}

} // namespace reprojectionToExternalCamera

int main(int argc, char *argv[]) {
    using namespace reprojectionToExternalCamera;
    pho::api::PhoXiFactory factory;

    std::cout << "Reprojection to External Camera Example" << std::endl;
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
        interactive(factory);
        return 0;
    }
    catch (utils::MissingCalibrationFile& e) {
        utils::printCalibrationError(e);
    }
    catch (utils::IncorrectCalibrationFile& e) {
        utils::printCalibrationError(e);
    }
    catch (std::runtime_error& e) {
        std::cout << "Error occured: " << std::endl;
        std::cout << "\t" << e.what() << std::endl;
    }
    return 1;
}

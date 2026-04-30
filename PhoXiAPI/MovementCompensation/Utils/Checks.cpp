#include "Checks.h"

#include <iostream>
#include <thread>

namespace utils {

#define LOCAL_CROSS_SLEEP(Millis) std::this_thread::sleep_for(std::chrono::milliseconds(Millis));

void PhoXiControlRunning(pho::api::PhoXiFactory& factory) {

    if (!factory.isPhoXiControlRunning()) {
        std::cout << std::endl << "Waiting for PhoXi Control ..." << std::endl;
    
        //Wait for the PhoXiControl
        while (!factory.isPhoXiControlRunning())
        {
            LOCAL_CROSS_SLEEP(100);
        }
    }

    std::cout << "PhoXi Control Version: " << factory.GetPhoXiControlVersion() << std::endl;
    std::cout << "PhoXi API Version: " << factory.GetAPIVersion() << std::endl;
}

} // namespace utils
/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"
#include <iomanip>
#include <string>

using namespace pho;

int main (int argc, char **argv)
{
    if(argc < 2) {
        std::cerr << "Provide device IP as a parameter!" << std::endl;
        return 1;
    }

    const char* deviceIp = argv[1];
    bool toggle = false;

    if (argc > 2) {
        std::string toggleArg = argv[2];
        if (toggleArg == "--enable") {
            toggle = true;
        } else if (toggleArg == "--disable") {
            toggle = false;
        } else {
            std::cerr << "Invalid argument for toggle. Use --enable or --disable." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "No toggle argument provided. Use --enable or --disable." << std::endl;
        return 1;
    }

    GError *error = nullptr;

    /* Connect to the first available camera */
    auto camera = create_gobject_unique(arv_camera_new (deviceIp, &error));
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if(!ARV_IS_CAMERA(camera.get())) {
        std::cerr << "Error: Not a camera instance!";
        return 1;
    }

    std::cout << "Connected to camera: " << arv_camera_get_model_name (camera.get(), nullptr) << std::endl;
    std::cout << (toggle ? "Enabling" : "Disabling") << " JumboFrames..." << std::endl;

    // Please note that after executing this command, the network interface is reset,
    // and the device will not be available for a few seconds. 
    // As a result, the following commands may time out, and a reconnect procedure should be followed.
    arv_camera_set_boolean(camera.get(), "EnableJumboFrames", toggle, &error);
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    } else {
        std::cout << "JumboFrames " << (toggle ? "enabled" : "disabled") << "!" << std::endl;
    }

    return 0;
}
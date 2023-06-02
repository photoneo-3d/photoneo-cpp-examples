/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"
#include <iomanip>

using namespace pho;

void printHelp() {
    std::cout << "To find available devices and their device ids, use 'discover' command.\n";
    std::cout << "Note: Current device IP can be also used as device id if known.\n\n";
    std::cout << "Usage: [COMMAND] [OPTIONS]\n";
    std::cout << "Commands:\n";

    std::cout << std::left << std::setw(15) << "discover";
    std::cout << std::left << std::setw(40) << "";
    std::cout << "Finds available devices on the network\n";

    std::cout << std::left << std::setw(15) << "static";
    std::cout << std::left << std::setw(40) << "[DEVICE ID] [IP] [MASK] [GATEWAY]";
    std::cout << "Sets static IP configuration\n";

    std::cout << std::left << std::setw(15) << "dhcp";
    std::cout << std::left << std::setw(40) << "[DEVICE ID]";
    std::cout << "Sets DHCP configuration\n";

    std::cout << std::endl;
}

void discovery() {
    arv_update_device_list();
    auto devicesFound = arv_get_n_devices();
    for(unsigned int index = 0; index < devicesFound; ++index) {
        std::cout << "Device " << index << ":\n";
        std::cout << std::left << std::setw(15) << "DeviceId:" << arv_get_device_id(index) << "\n";
        std::cout << std::left << std::setw(15) << "PhysicalId:" << arv_get_device_physical_id(index) << "\n";
        std::cout << std::left << std::setw(15) << "Address:" << arv_get_device_address(index) << "\n";
        std::cout << std::left << std::setw(15) << "Vendor:" << arv_get_device_vendor(index) << "\n";
        std::cout << std::left << std::setw(15) << "Man. info:" << arv_get_device_manufacturer_info(index) << "\n";
        std::cout << std::left << std::setw(15) << "Model:" << arv_get_device_model(index) << "\n";
        std::cout << std::left << std::setw(15) << "Serial:" << arv_get_device_serial_nbr(index) << "\n";
        std::cout << std::left << std::setw(15) << "Protocol:" << arv_get_device_protocol(index) << "\n";
        std::cout << "\n";
    }
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        printHelp();
        return 1;
    }

    const std::string command = argv[1];
    if(command == "discover") {
        discovery();
        return 0;
    }

    if(argc < 3) {
        printHelp();
        return 1;
    }

    const char* deviceId = argv[2];
    GError *error = nullptr;

    //Connect to the camera using device id or current IP
    auto camera = create_gobject_unique(arv_camera_new (deviceId, &error));
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if(command == "static") {
        if(argc < 6) {
            std::cerr << "Error: Invalid number of arguments provided!\n\n";
            printHelp();
            return 1;
        }

        const char* ip = argv[3];
        const char* mask = argv[4];
        const char* gateway = argv[5];

        //Set persistent static configuration
        arv_camera_gv_set_persistent_ip_from_string(camera.get(), ip, mask, gateway, &error);
        if(error) {
            std::cerr << "Error: " << error->message << std::endl;
            return 1;
        }

        //Change current mode to the persistent ip mode
        arv_camera_gv_set_ip_configuration_mode(camera.get(), ARV_GV_IP_CONFIGURATION_MODE_PERSISTENT_IP, &error);
        if(error) {
            std::cerr << "Error: " << error->message << std::endl;
            return 1;
        }
    }
    else if(command == "dhcp") {
        //Change current mode to the DHCP mode
        arv_camera_gv_set_ip_configuration_mode(camera.get(), ARV_GV_IP_CONFIGURATION_MODE_DHCP, &error);
        if(error) {
            std::cerr << "Error: " << error->message << std::endl;
            return 1;
        }
    }

    //Reset device to set new device configuration
    arv_camera_execute_command(camera.get(), "DeviceReset", &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    return 0;
}

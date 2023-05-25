/*
* Photoneo's API Example - MaintenanceCommandsExample.cpp
* Defines the entry point for the console application.
* Demonstrates the maintenance functionality of PhoXiFactory class like reboot, shutdown and factory reset of the device.
*/

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "PhoXi.h"

#if defined(_WIN32)
    #define DELIMITER "\\"
#elif defined (__linux__)
    #define DELIMITER "/"
#endif
#define LOCAL_CROSS_SLEEP(Millis) std::this_thread::sleep_for(std::chrono::milliseconds(Millis));

template<class T>
bool ReadLine(T &Output)
{
    std::string Input;
    std::getline(std::cin, Input);
    std::stringstream InputSteam(Input);
    return (InputSteam >> Output) ? true : false;
}
bool ReadLine(std::string &Output)
{
    std::getline(std::cin, Output);
    return true;
}

void printDeviceInfo(const pho::api::PhoXiDeviceInformation& deviceInfo) 
{
    std::cout << "  Name:                    " << deviceInfo.Name << std::endl;
    std::cout << "  Hardware Identification: " << deviceInfo.HWIdentification << std::endl;
    std::cout << "  Type:                    " << std::string(deviceInfo.Type) << std::endl;
    std::cout << "  Firmware version:        " << deviceInfo.FirmwareVersion << std::endl;
    std::cout << "  Variant:                 " << deviceInfo.Variant << std::endl;
    std::cout << "  IsFileCamera:            " << (deviceInfo.IsFileCamera ? "Yes" : "No") << std::endl;
    std::cout << "  Feature-Alpha:           " << (deviceInfo.CheckFeature("Alpha") ? "Yes" : "No") << std::endl;
    std::cout << "  Feature-Color:           " << (deviceInfo.CheckFeature("Color") ? "Yes" : "No") << std::endl;
    std::cout << "  Status:                  "
        << (deviceInfo.Status.Attached ? "Attached to PhoXi Control. " : "Not Attached to PhoXi Control. ")
        << (deviceInfo.Status.Ready    ? "Ready to connect"            : "Occupied")
        << std::endl << std::endl;    
}

int main(int argc, char *argv[])
{
    pho::api::PhoXiFactory factory;

    //Wait for the PhoXiControl
    while (!factory.isPhoXiControlRunning())
    {
        LOCAL_CROSS_SLEEP(100);
    }

    std::cout << "PhoXi Control Version: " << factory.GetPhoXiControlVersion() << std::endl;
    std::cout << "PhoXi API Version: " << factory.GetAPIVersion() << std::endl;

    const std::vector<pho::api::PhoXiDeviceInformation> deviceList = factory.GetDeviceList();
    std::cout << "PhoXi Factory found " << deviceList.size() << " devices." << std::endl << std::endl;
    
    for (std::size_t i = 0; i < deviceList.size(); ++i)
    {
        std::cout << "Device: " << i << std::endl;
        printDeviceInfo(deviceList[i]);
    }

    std::size_t selectedIndex;
    while (true)
    {
        std::cout << "Please enter device Index from the list: ";
        if (!ReadLine(selectedIndex) || selectedIndex >= deviceList.size())
        {
            std::cout << "Incorrect input!" << std::endl;
            continue;
        }

        break;
    }

    const pho::api::PhoXiDeviceInformation& selectedDeviceInformation = deviceList[selectedIndex];

    std::cout << "Selected device: " << selectedDeviceInformation.Name << std::endl;
    std::cout << "Do you want to:" << std::endl;
    std::cout << "  0: Exit application" << std::endl;
    std::cout << "  1: Reboot device" << std::endl;

    int selectedOperation;
    while (true)
    {
        std::cout << "Please enter your choice: ";
        if (!ReadLine(selectedOperation) || selectedOperation > 1)
        {
            std::cout << "Incorrect input!" << std::endl;
            continue;
        }

        break;
    }

    switch (selectedOperation)
    {
    default:
    case 0:
    {
        break;
    }
    case 1:
    {
        if (selectedDeviceInformation.IsFileCamera)             
        {
            std::cout << "File camera can not be rebooted!" << std::endl;
            break;
        }

        //Function call does not wait for the device to reboot, reboot command is sent only, which means the device will reboot after some time.
        if (factory.Reboot(selectedDeviceInformation.HWIdentification)) 
        {
            std::cout << "Reboot command sent to the device " << selectedDeviceInformation.Name << ". Device will reboot shortly." << std::endl;
        }
        else             
        {
            std::cout << "Failed to send reboot command to the device " << selectedDeviceInformation.Name << "!" << std::endl;
        }

        break;
    }
    }

    return 0;
}
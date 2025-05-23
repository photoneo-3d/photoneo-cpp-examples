/*
* Photoneo's API Example - AutonomousMaintenanceExample.cpp
* Defines the entry point for the console application.
* Demonstrates the atonomous maintenance functionality that checks and improves consistency of the device calibration.
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

void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList)
{
    for (std::size_t i = 0; i < DeviceList.size(); ++i)
    {
        std::cout << "Device: " << i << std::endl;
        printDeviceInfo(DeviceList[i]);
    }
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
    if (deviceList.empty())
    {
        std::cout << "PhoXi Factory has found 0 devices" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "PhoXi Factory found " << deviceList.size() << " devices." << std::endl << std::endl;
    }

    printDeviceInfoList(deviceList);

    //Try to connect device opened in PhoXi Control, if any
    pho::api::PPhoXi phoXiDevice = factory.CreateAndConnectFirstAttached();
    if (phoXiDevice)
    {
        std::cout << "You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: "
            << (std::string)phoXiDevice->HardwareIdentification << std::endl;
    }
    else
    {
        std::cout << "You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to first device in device list" << std::endl;
        phoXiDevice = factory.CreateAndConnect(deviceList.front().HWIdentification);
    }

    //Check if device was created
    if (!phoXiDevice)
    {
        std::cout << "Your device was not created!" << std::endl;
        return 0;
    }

    //Check if device is connected
    if (!phoXiDevice->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 0;
    }
    std::cout << std::endl << std::endl;

    //Check if the MotionCam are Enabled and Can be Set
    if (!phoXiDevice->MotionCam.isEnabled() || !phoXiDevice->MotionCam.CanSet() || !phoXiDevice->MotionCam.CanGet())
    {
        std::cout << "MotionCam not supported by the Device Hardware, or are Read only on the specific device" << std::endl;
        return 0;
    }

    phoXiDevice->MotionCam->MaintenanceMode = pho::api::PhoXiMaintenanceMode::Auto;
    if (phoXiDevice->MotionCam->MaintenanceMode != pho::api::PhoXiMaintenanceMode::Auto)
    {
        std::cout << "Device does not support MaintenanceMode" << std::endl;
        return 0;
    }
    int frameID = phoXiDevice->TriggerFrame();
    pho::api::PFrame frame = phoXiDevice->GetSpecificFrame(frameID);
    if (frame)
    {
        for (const auto& msg : frame->Messages)
        {
            std::cout << msg.Text << std::endl;
        }
    }
    else {
        std::cout << "Failed to retrieve the frame!";
    }

    //Disconnect PhoXi device
    phoXiDevice->Disconnect();
    return 0;
}

/*
 * Photoneo's API Example - PTPTime.cpp
 * pho::api::Frame::FrameStartTime processing
 */

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "PhoXi.h"

int main(int argc, char *argv[]) {
    pho::api::PhoXiFactory Factory;

    // Check if the PhoXi Control Software is running
    if (!Factory.isPhoXiControlRunning()) {
        std::cout << "PhoXi Control Software is not running" << std::endl;
        return 1;
    }

    // Get List of available devices on the network
    std::vector<pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
    if (DeviceList.empty()) {
        std::cout << "PhoXi Factory has found 0 devices" << std::endl;
        return 1;
    }

    // Try to connect device opened in PhoXi Control, if any
    pho::api::PPhoXi PhoXiDevice = Factory.CreateAndConnectFirstAttached();
    if (PhoXiDevice) {
        std::cout << "You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: " << (std::string)PhoXiDevice->HardwareIdentification
                  << std::endl;
    } else {
        std::cout << "You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to first device in device list" << std::endl;
        PhoXiDevice = Factory.CreateAndConnect(DeviceList.front().HWIdentification);
    }

    // Check if device was created
    if (!PhoXiDevice) {
        std::cout << "Your device was not created!" << std::endl;
        return 1;
    }

    // Check if device is connected
    if (!PhoXiDevice->isConnected()) {
        std::cout << "Your device is not connected" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
    if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
        std::cout << "Failed to set trigger mode to Software!" << std::endl;
        return 1;
    }

    if (!PhoXiDevice->isAcquiring()) {
        if (!PhoXiDevice->StartAcquisition()) {
            std::cout << "Failed to start acquisition!" << std::endl;
            return 1;
        }
    }

    int frameIndex1 = PhoXiDevice->TriggerFrame();
    if (frameIndex1 < 0) {
        std::cout << "Failed to trigger frame!" << std::endl;
        return 1;
    }

    pho::api::PFrame frame1 = PhoXiDevice->GetSpecificFrame(frameIndex1);
    if (!frame1) {
        std::cout << "Failed to getting frame!" << std::endl;
        return 1;
    }

    int frameIndex2 = PhoXiDevice->TriggerFrame();
    if (frameIndex2 < 0) {
        std::cout << "Failed to trigger frame!" << std::endl;
        return 1;
    }

    pho::api::PFrame frame2 = PhoXiDevice->GetSpecificFrame(frameIndex2);
    if (!frame2) {
        std::cout << "Failed to getting frame!" << std::endl;
        return 1;
    }

    // Get two frames and calculate delay between them
    pho::api::PhoXiPTPTime ptpTime1 = frame1->Info.FrameStartTime;
    pho::api::PhoXiPTPTime ptpTime2 = frame2->Info.FrameStartTime;

    if (!ptpTime1.IsValid() || !ptpTime1.IsValid()) {
        std::cout << "Frame doesn't contains valid start acquisition time!" << std::endl;
        return 1;
    }

    std::cout << "Frame 1 start acquisition time is " << ptpTime1.TimeAsString("%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "Frame 2 start acquisition time is " << ptpTime2.TimeAsString("%Y-%m-%d %H:%M:%S") << std::endl;

    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(ptpTime2.Time - ptpTime1.Time);
    std::cout << "Duration between frames is " << duration.count() << " ms" << std::endl;

    PhoXiDevice->StopAcquisition();
    // Disconnect and keep PhoXiControl connected
    PhoXiDevice->Disconnect();
    return 0;
}
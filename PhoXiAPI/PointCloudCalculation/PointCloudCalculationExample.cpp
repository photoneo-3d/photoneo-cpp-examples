/*
* Photoneo's API Example - PointCloudCalculation.cpp
* Defines the entry point for the console application.
* Demonstrates the basic functionality of PhoXi device.
* This example shows how to calculate point cloud from
* depth map and reprojection map
*/

#include <vector>
#include <string>
#include <iostream>

#include "PhoXi.h"

//Print out list of device info to standard output
void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList);
//Print out device info to standard output
void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo);
//Calculate point colud using depth map and reprojection map
bool calculatePointCloud(pho::api::PointCloud32f& pointCloud,
    pho::api::DepthMap32f depth,
    pho::api::PhoXiReprojectionMap reprojection);

int main(int argc, char *argv[])
{
    pho::api::PhoXiFactory Factory;

    //Check if the PhoXi Control Software is running
    if (!Factory.isPhoXiControlRunning())
    {
        std::cout << "PhoXi Control Software is not running" << std::endl;
        return 0;
    }

    //Get List of available devices on the network
    std::vector <pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
    if (DeviceList.empty())
    {
        std::cout << "PhoXi Factory has found 0 devices" << std::endl;
        return 0;
    }
    printDeviceInfoList(DeviceList);

    //Try to connect device opened in PhoXi Control, if any
    pho::api::PPhoXi PhoXiDevice = Factory.CreateAndConnectFirstAttached();
    if (PhoXiDevice)
    {
        std::cout << "You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: "
            << (std::string)PhoXiDevice->HardwareIdentification << std::endl;
    }
    else
    {
        std::cout << "You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to last device in device list" << std::endl;
        PhoXiDevice = Factory.CreateAndConnect(DeviceList.back().HWIdentification);
    }

    //Check if device was created
    if (!PhoXiDevice)
    {
        std::cout << "Your device was not created!" << std::endl;
        return 0;
    }

    //Check if device is connected
    if (!PhoXiDevice->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 0;
    }

    //if your device is motion cam, you can choose topology
    if (PhoXiDevice->GetType() == pho::api::PhoXiDeviceType::MotionCam3D) {
        pho::api::PhoXiMotionCamCameraMode mode = PhoXiDevice->MotionCamCameraMode;
        mode.OutputTopology = pho::api::PhoXiOutputTopology::RegularGrid;
        PhoXiDevice->MotionCamCameraMode = mode;
    }
    pho::api::PhoXiReprojectionMap map = PhoXiDevice->ReprojectionMap;
    PhoXiDevice->StartAcquisition();
    if (!PhoXiDevice->isAcquiring())
    {
        std::cout << "Your device could not start acquisition!" << std::endl;
        return 0;
    }
    int id = PhoXiDevice->TriggerFrame();
    if (id < 0) {
        std::cout << "Trigger frame unsuccessful" << std::endl;
    }
    auto frame = PhoXiDevice->GetSpecificFrame(id);
    if (frame) {
        auto depth = frame->DepthMap;
        pho::api::PointCloud32f pointCloud;
        pointCloud.Resize(depth.Size);
        calculatePointCloud(pointCloud, depth, map);
        //pointCloud now contains your point cloud data
    }

    return 0;
}

bool calculatePointCloud(pho::api::PointCloud32f& pointCloud,
    pho::api::DepthMap32f depth,
    pho::api::PhoXiReprojectionMap reprojection) {
    if (pointCloud.Size != reprojection.Map.Size) {
        return false;
    }
    for (int row = 0; row < pointCloud.Size.Height; ++row) {
        for (int col = 0; col < pointCloud.Size.Width; ++col) {
            if (depth.At(row, col) > 0.0f) {
                pointCloud.At(row, col).x = depth.At(row, col) * reprojection.Map.At(row, col).x;
                pointCloud.At(row, col).y = depth.At(row, col) * reprojection.Map.At(row, col).y;
                pointCloud.At(row, col).z = depth.At(row, col) * reprojection.Map.At(row, col).z;
            }
            else {
                pointCloud.At(row, col).x = 0.0f;
                pointCloud.At(row, col).y = 0.0f;
                pointCloud.At(row, col).z = 0.0f;
            }
        }
    }
    return true;
}

void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList)
{
    for (std::size_t i = 0; i < DeviceList.size(); ++i)
    {
        std::cout << "Device: " << i << std::endl;
        printDeviceInfo(DeviceList[i]);
    }
}

void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo)
{
    std::cout << "  Name:                    " << DeviceInfo.Name << std::endl;
    std::cout << "  Hardware Identification: " << DeviceInfo.HWIdentification << std::endl;
    std::cout << "  Type:                    " << std::string(DeviceInfo.Type) << std::endl;
    std::cout << "  Firmware version:        " << DeviceInfo.FirmwareVersion << std::endl;
    std::cout << "  Variant:                 " << DeviceInfo.Variant << std::endl;
    std::cout << "  IsFileCamera:            " << (DeviceInfo.IsFileCamera ? "Yes" : "No") << std::endl;
    std::cout << "  Feature-Alpha:           " << (DeviceInfo.CheckFeature("Alpha") ? "Yes" : "No") << std::endl;
    std::cout << "  Feature-Color:           " << (DeviceInfo.CheckFeature("Color") ? "Yes" : "No") << std::endl;
    std::cout << "  Status:                  "
        << (DeviceInfo.Status.Attached ? "Attached to PhoXi Control. " : "Not Attached to PhoXi Control. ")
        << (DeviceInfo.Status.Ready ? "Ready to connect" : "Occupied")
        << std::endl << std::endl;
}

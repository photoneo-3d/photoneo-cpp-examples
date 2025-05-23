/*
* Photoneo's API Example - MarkerDotCorrectionExample.cpp
* Makes a marker dot reference frame and a test frame.
*/

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "PhoXi.h"

//Print out list of device info to standard output
void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList);
//Print out device info to standard output
void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo);

float distance(const pho::api::Point2_32f& a, const pho::api::Point2_32f& b)
{
    return std::sqrt((a.x - b.x) * (a.x - b.x) +
                     (a.y - b.y) * (a.y - b.y));
}

std::ostream& operator<<(std::ostream& os, const pho::api::Point2_32f& p)
{
    return os << "(" << p.x << ", " << p.y << ")";
}

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
    std::vector<pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
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
            << (std::string) PhoXiDevice->HardwareIdentification << std::endl;
        if (!PhoXiDevice->isAcquiring()) {
            PhoXiDevice->StartAcquisition();
        }
    }
    else
    {
        std::cout << "You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to first device in device list" << std::endl;
        PhoXiDevice = Factory.CreateAndConnect(DeviceList.front().HWIdentification);
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
    std::cout << std::endl << std::endl;

    if (PhoXiDevice->Info().Type == pho::api::PhoXiDeviceType::MotionCam3D)
    {
        if (PhoXiDevice->MotionCam->OperationMode == pho::api::PhoXiOperationMode::Camera)
        {
            // Marker dot recognition is not supported in Raw nor IrregularGrid topologies
            PhoXiDevice->MotionCamCameraMode->OutputTopology = pho::api::PhoXiOutputTopology::RegularGrid;
        }
    }

    //Record reference
    PhoXiDevice->ProcessingSettings->MarkerDotCorrection.OperationMode = pho::api::PhoXiMarkerDotCorrectionOperationMode::ReferenceRecording;
    int FrameId = PhoXiDevice->TriggerFrame();
    if (FrameId < 0)
    {
        std::cout << "Failed to capture the marker dot reference frame! code=" << FrameId << std::endl;
        return 0; 
    }
    auto Frame = PhoXiDevice->GetSpecificFrame(FrameId);
    const auto Status = Frame->Info.MarkerDots.Status;
    if (Status != pho::api::PhoXiMarkerDots::MarkerDotStatus::REFERENCE_SUCCESSFULLY_RECORDED)
    {
        std::cout << "Recording of the marker dot reference failed!\n" << Status << std::endl;
        return 0;
    }

    const auto ReferenceDots = Frame->Info.MarkerDots.RecognizedMarkerDots;
    for (std::size_t i = 0; i < ReferenceDots.size(); ++i)
    {
        std::cout << "Marker dot " << i << " recorded in the reference at " << ReferenceDots[i].reference2d << std::endl;
    }
    std::cout << std::endl;

    //Test frame
    PhoXiDevice->ProcessingSettings->MarkerDotCorrection.OperationMode = pho::api::PhoXiMarkerDotCorrectionOperationMode::Active;

    FrameId = PhoXiDevice->TriggerFrame();
    if (FrameId < 0) {
        std::cout << "Failed to capture the test frame! code=" << FrameId << std::endl;
        return 0; 
    }
    Frame = PhoXiDevice->GetSpecificFrame(FrameId);

    const auto ObservedDots = Frame->Info.MarkerDots.RecognizedMarkerDots;
    for (std::size_t i = 0; i < ReferenceDots.size(); ++i)
    {
        const auto RefDots = ReferenceDots[i].reference2d;
        bool FoundInFrame = false;

        std::cout << "Marker dot " << i << " seen in the reference at " << RefDots;
        for (std::size_t j = 0; j < ObservedDots.size(); ++j)
        {
            if (ObservedDots[j].reference2d == RefDots)
            {
                const float Displacement = distance(ObservedDots[j].observation2d, RefDots);
                std::cout << " was recognized at " << ObservedDots[j].observation2d
                        << ", displaced by " << Displacement << " px" << std::endl;
                FoundInFrame = true;
                break;
            }
        }
        if (!FoundInFrame)
        {
            std::cout << " was not recognized in the current frame." << std::endl;
        }
    }

    //Disconnect PhoXi device
    PhoXiDevice->Disconnect();
    return 0;
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

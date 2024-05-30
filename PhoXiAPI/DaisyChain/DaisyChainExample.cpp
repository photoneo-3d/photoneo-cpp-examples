///
/// Photoneo's API Example - DaisyChainExample.cpp
///
/// This is a simple example of how to connect two MotionCam-3D devices into
/// a daisy chain setup - when they trigger scanning on each other sequentially.
///
/// For more information about how to set up daisy chain wiring see device
/// manual Appendix 3.

#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "PhoXi.h"

// Connect to device and verify that it's suitable for a daisy chain
pho::api::PPhoXi connectAndVerify(pho::api::PhoXiFactory &Factory, const std::string& DeviceSerialNumber);
//Run software trigger example
void startSoftwareTriggerDaisyChain(pho::api::PPhoXi &Primary, pho::api::PPhoXi &Secondary);
//Run free run example
void startFreeRunDaisyChain(pho::api::PPhoXi &Primary, pho::api::PPhoXi &Secondary);
//Print out list of device info to standard output
void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList);
//Print out device info to standard output
void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo);
//Print out frame info to standard output
void printFrameInfo(const pho::api::PFrame &Frame);
//Print out frame data to standard output
void printFrameData(const pho::api::PFrame &Frame);

enum class TriggerType {
    SOFTWARE,
    HARDWARE
};

int main(int argc, char *argv[])
{

    if (argc < 3) {
        std::cerr << "This example requires serial numbers of two devices" << std::endl;
        return 1;
    }

    std::string PrimaryDeviceSerialNumber(argv[1]);
    std::string SecondaryDeviceSerialNumber(argv[2]);

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

    pho::api::PPhoXi Primary = connectAndVerify(Factory, PrimaryDeviceSerialNumber);
    pho::api::PPhoXi Secondary = connectAndVerify(Factory, SecondaryDeviceSerialNumber);

    //Check if devices were created
    if (!Primary)
    {
        std::cout << "Your primary device was not created!" << std::endl;
        return 0;
    }
    if (!Secondary)
    {
        std::cout << "Your secondary device was not created!" << std::endl;
        return 0;
    }

    //Check if device is connected
    if (!Primary->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 0;
    }
    if (!Secondary->isConnected())
    {
        std::cout << "Your secondary device is not connected" << std::endl;
        return 0;
    }
    

    startSoftwareTriggerDaisyChain(Primary, Secondary);

    startFreeRunDaisyChain(Primary, Secondary);

    //Disconnect PhoXi devices
    Primary->Disconnect();
    Secondary->Disconnect();
    return 0;
}

pho::api::PPhoXi connectAndVerify(pho::api::PhoXiFactory &Factory, const std::string& DeviceSerialNumber)
{
    auto Device = Factory.CreateAndConnect(DeviceSerialNumber);
    if (!Device) {
        std::cout << "Factory cannot create and connect to " << DeviceSerialNumber << std::endl;
        return nullptr;
    }
    if (!Device->MotionCam.isEnabled()) {
        std::cout << "The device " << DeviceSerialNumber << " is not a MotionCam-3D" << std::endl;
        return nullptr;
    }
    return Device;
}

void configureRunMode(
        pho::api::PPhoXi& Device,
        pho::api::PhoXiTriggerMode RunMode)
{
    if (Device->isAcquiring()) {
        if (!Device->StopAcquisition()) {
            std::cout << "Warning: cannot stop acquisition on device "
                      << Device->HardwareIdentification.GetStoredValue() << std::endl;
        }
    }
    Device->TriggerMode = RunMode;
    if (!Device->TriggerMode.isLastOperationSuccessful()) {
        std::cout << "Warning: cannot set run mode on device "
                  << Device->HardwareIdentification.GetStoredValue() << std::endl;
    }
}

void configureTrigger(
        pho::api::PPhoXi& Device,
        pho::api::PhoXiHardwareTriggerSignal Signal,
        TriggerType HwTrigger)
{
    pho::api::PhoXiMotionCam mc = Device->MotionCam;

    mc.HardwareTriggerSignal = Signal;

    if (HwTrigger == TriggerType::HARDWARE) {
        mc.HardwareTrigger = true;
    } else {
        mc.HardwareTrigger = false;
    }

    Device->MotionCam = mc;
    if (!Device->MotionCam.isLastOperationSuccessful()) {
        std::cout << "Warning: cannot configure hardware trigger on device "
                  << Device->HardwareIdentification.GetStoredValue() << std::endl;
    }
}

void startSoftwareTriggerDaisyChain(
        pho::api::PPhoXi &Primary,
        pho::api::PPhoXi &Secondary)
{
    configureRunMode(Primary, pho::api::PhoXiTriggerMode::Software);
    configureRunMode(Secondary, pho::api::PhoXiTriggerMode::Freerun);

    // If you use pull-up logic then signal goes on the rising edge.
    // If you use pull-down logic then signal goes on the falling edge.
    // The Primary device is triggered by this API program. The Secondary device
    // is triggered by signal from the Primary device.
    configureTrigger(Primary, pho::api::PhoXiHardwareTriggerSignal::Rising, TriggerType::SOFTWARE);
    configureTrigger(Secondary, pho::api::PhoXiHardwareTriggerSignal::Rising, TriggerType::HARDWARE);

    Primary->StartAcquisition();
    if (!Primary->isAcquiring())
    {
        std::cout << "Primary device could not start acquisition!" << std::endl;
        return;
    }

    Secondary->StartAcquisition();
    if (!Secondary->isAcquiring())
    {
        std::cout << "Secondary device could not start acquisition!" << std::endl;
        return;
    }

    // At this point the Secondary device waits for signals from the Primary
    // device. Once we trigger the Primary we may get frames from both devices.
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Triggering the " << i << "-th frame" << std::endl;
        int FrameID = Primary->TriggerFrame();
        if (FrameID < 0)
        {
            //If negative number is returned trigger was unsuccessful
            std::cout << "Trigger was unsuccessful! code=" << FrameID << std::endl;
            continue;
        }
        else
        {
            std::cout << "Frame was triggered, Frame Id: " << FrameID << std::endl;
        }

        std::cout << "Waiting for frame " << i << std::endl;
        pho::api::PFrame Frame = Primary->GetSpecificFrame(FrameID, pho::api::PhoXiTimeout::Infinity);

        if (Frame)
        {
            std::cout << "Primary device frame " << i << std::endl;
            printFrameInfo(Frame);
            printFrameData(Frame);
        }
        else
        {
            std::cout << "Failed to retrieve frame from the Primary device!" << std::endl;
        }

        Frame = Secondary->GetFrame(pho::api::PhoXiTimeout::Infinity);
        if (Frame)
        {
            std::cout << "Secondary device frame " << i << std::endl;
            printFrameInfo(Frame);
            printFrameData(Frame);
        }
        else
        {
            std::cout << "Failed to retrieve frame from the Secondary device!" << std::endl;
        }
    }
    Primary->StopAcquisition();
    Secondary->StopAcquisition();
}

void startFreeRunDaisyChain(
        pho::api::PPhoXi &Primary,
        pho::api::PPhoXi &Secondary)
{
    configureRunMode(Primary, pho::api::PhoXiTriggerMode::Freerun);
    configureRunMode(Secondary, pho::api::PhoXiTriggerMode::Freerun);
    configureTrigger(Primary, pho::api::PhoXiHardwareTriggerSignal::Rising, TriggerType::HARDWARE);
    configureTrigger(Secondary, pho::api::PhoXiHardwareTriggerSignal::Rising, TriggerType::HARDWARE);

    Primary->StartAcquisition();
    if (!Primary->isAcquiring())
    {
        std::cout << "Primary device could not start acquisition!" << std::endl;
        return;
    }

    Secondary->StartAcquisition();
    if (!Secondary->isAcquiring())
    {
        std::cout << "Secondary device could not start acquisition!" << std::endl;
        return;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // At this point both devices are waiting for a HW trigger input impulse.
    // Depending on your needs you can provide it by wiring an external trigger
    // source or you can trigger the Primary device via software, like we do in
    // this example.
    Primary->TriggerFrame(false, false);

    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Waiting for frame " << i << std::endl;
        pho::api::PFrame Frame = Primary->GetFrame(pho::api::PhoXiTimeout::Infinity);

        if (Frame)
        {
            std::cout << "Primary device frame " << i << std::endl;
            printFrameInfo(Frame);
            printFrameData(Frame);
        }
        else
        {
            std::cout << "Failed to retrieve frame from the Primary device!" << std::endl;
        }

        Frame = Secondary->GetFrame(pho::api::PhoXiTimeout::Infinity);
        if (Frame)
        {
            std::cout << "Secondary device frame " << i << std::endl;
            printFrameInfo(Frame);
            printFrameData(Frame);
        }
        else
        {
            std::cout << "Failed to retrieve frame from the Secondary device!" << std::endl;
        }
    }
    Primary->StopAcquisition();
    Secondary->StopAcquisition();
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

void printFrameInfo(const pho::api::PFrame &Frame)
{
    const pho::api::FrameInfo &FrameInfo = Frame->Info;
    std::cout << "  Frame params: " << std::endl;
    std::cout << "    Frame Index: "                << FrameInfo.FrameIndex << std::endl;
    std::cout << "    Frame Timestamp: "            << FrameInfo.FrameTimestamp << " ms" << std::endl;
    std::cout << "    Frame Acquisition duration: " << FrameInfo.FrameDuration << " ms" << std::endl;
    std::cout << "    Frame Computation duration: " << FrameInfo.FrameComputationDuration << " ms" << std::endl;
    std::cout << "    Frame Transfer duration: "    << FrameInfo.FrameTransferDuration << " ms" << std::endl;
    std::cout << "    Frame Acquisition time (PTP): " << FrameInfo.FrameStartTime.TimeAsString("%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "    Sensor Position: ["
        << FrameInfo.SensorPosition.x << "; "
        << FrameInfo.SensorPosition.y << "; "
        << FrameInfo.SensorPosition.z << "]"
        << std::endl;
    std::cout << "    Total scan count: "           << FrameInfo.TotalScanCount << std::endl;
    std::cout << "    Color Camera Position: ["
        << FrameInfo.ColorCameraPosition.x << "; "
        << FrameInfo.ColorCameraPosition.y << "; "
        << FrameInfo.ColorCameraPosition.z << "]"
        << std::endl;
    std::cout << "    Current Camera Position: ["
        << FrameInfo.CurrentCameraPosition.x << "; "
        << FrameInfo.CurrentCameraPosition.y << "; "
        << FrameInfo.CurrentCameraPosition.z << "]"
        << std::endl;
    std::cout << "   FilenamePath: " << FrameInfo.FilenamePath << std::endl;
    std::cout << "   HWIdentification: " << FrameInfo.HWIdentification << std::endl;
}

void printFrameData(const pho::api::PFrame &Frame)
{
    if (Frame->Empty())
    {
        std::cout << "Frame is empty.";
        return;
    }
    std::cout << "  Frame data: " << std::endl;
    if (!Frame->PointCloud.Empty())
    {
        std::cout << "    PointCloud:    ("
            << Frame->PointCloud.Size.Width << " x "
            << Frame->PointCloud.Size.Height << ") Type: "
            << Frame->PointCloud.GetElementName()
            << std::endl;
    }
    if (!Frame->NormalMap.Empty())
    {
        std::cout << "    NormalMap:     ("
            << Frame->NormalMap.Size.Width << " x "
            << Frame->NormalMap.Size.Height << ") Type: "
            << Frame->NormalMap.GetElementName()
            << std::endl;
    }
    if (!Frame->DepthMap.Empty())
    {
        std::cout << "    DepthMap:      ("
            << Frame->DepthMap.Size.Width << " x "
            << Frame->DepthMap.Size.Height << ") Type: "
            << Frame->DepthMap.GetElementName()
            << std::endl;
    }
    if (!Frame->ConfidenceMap.Empty())
    {
        std::cout << "    ConfidenceMap: ("
            << Frame->ConfidenceMap.Size.Width << " x "
            << Frame->ConfidenceMap.Size.Height << ") Type: "
            << Frame->ConfidenceMap.GetElementName()
            << std::endl;
    }
    if (!Frame->Texture.Empty())
    {
        std::cout << "    Texture:       ("
            << Frame->Texture.Size.Width << " x "
            << Frame->Texture.Size.Height << ") Type: "
            << Frame->Texture.GetElementName()
            << std::endl;
    }
    if (!Frame->TextureRGB.Empty())
    {
        std::cout << "    TextureRGB:       ("
            << Frame->TextureRGB.Size.Width << " x "
            << Frame->TextureRGB.Size.Height << ") Type: "
            << Frame->TextureRGB.GetElementName()
            << std::endl;
    }
    if (!Frame->ColorCameraImage.Empty())
    {
        std::cout << "    ColorCameraImage:       ("
            << Frame->ColorCameraImage.Size.Width << " x "
            << Frame->ColorCameraImage.Size.Height << ") Type: "
            << Frame->ColorCameraImage.GetElementName()
            << std::endl;
    }
}




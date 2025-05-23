/*
 * Photoneo's API Example - ReadPointCloudExample.cpp
 * Defines the entry point for the console application.
 * Demonstrates how to get work with Point cloud of
 * received frames.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>

#include "PhoXi.h"

#if defined(_WIN32)
    #define DELIMITER "\\"
#elif defined(__linux__) || defined(__APPLE__)
    #define DELIMITER "/"
#endif
#define LOCAL_CROSS_SLEEP(Millis) std::this_thread::sleep_for(std::chrono::milliseconds(Millis));

class ReadPointCloudExample {
private:
    pho::api::PhoXiFactory Factory;
    pho::api::PPhoXi PhoXiDevice;
    pho::api::PFrame SampleFrame;
    std::vector<pho::api::PhoXiDeviceInformation> DeviceList;
    std::string OutputFolder = "";

    void GetAvailableDevicesExample();
    void ConnectPhoXiDeviceBySerialExample();
    void SoftwareTriggerExample();
    void DataHandlingExample();
    void SavePointCloudToPTS(const pho::api::PFrame &Frame, const std::string &ptsFilepath);
    void CorrectDisconnectExample();

    void PrintFrameInfo(const pho::api::PFrame &Frame);
    void PrintFrameData(const pho::api::PFrame &Frame);

    template <class T> bool ReadLine(T &Output) const {
        std::string Input;
        std::getline(std::cin, Input);
        std::stringstream InputSteam(Input);
        return (InputSteam >> Output) ? true : false;
    }
    bool ReadLine(std::string &Output) const {
        std::getline(std::cin, Output);
        return true;
    }

public:
    ReadPointCloudExample(){};
    ~ReadPointCloudExample(){};
    void Run();
};

void ReadPointCloudExample::GetAvailableDevicesExample() {
    // Wait for the PhoXiControl
    while (!Factory.isPhoXiControlRunning()) {
        LOCAL_CROSS_SLEEP(100);
    }
    std::cout << "PhoXi Control Version: " << Factory.GetPhoXiControlVersion()
              << std::endl;
    std::cout << "PhoXi API Version: " << Factory.GetAPIVersion() << std::endl;

    DeviceList = Factory.GetDeviceList();
    std::cout << "PhoXi Factory found " << DeviceList.size() << " devices."
              << std::endl
              << std::endl;
    pho::api::PhoXiDeviceInformation *DeviceInfo;
    for (std::size_t i = 0; i < DeviceList.size(); ++i) {
        DeviceInfo = &DeviceList[i];
        std::cout << "Device: " << i << std::endl;
        std::cout << "  Name:                    " << DeviceInfo->Name << std::endl;
        std::cout << "  Hardware Identification: " << DeviceInfo->HWIdentification << std::endl;
        std::cout << "  Type:                    " << std::string(DeviceInfo->Type) << std::endl;
        std::cout << "  Firmware version:        " << DeviceInfo->FirmwareVersion << std::endl;
        std::cout << "  Variant:                 " << DeviceInfo->Variant << std::endl;
        std::cout << "  IsFileCamera:            " << (DeviceInfo->IsFileCamera ? "Yes" : "No") << std::endl;        
        std::cout << "  Feature-Alpha:           " << (DeviceInfo->CheckFeature("Alpha") ? "Yes" : "No") << std::endl;
        std::cout << "  Feature-Color:           " << (DeviceInfo->CheckFeature("Color") ? "Yes" : "No") << std::endl;
        std::cout << "  Status:                  "
                  << (DeviceInfo->Status.Attached
                              ? "Attached to PhoXi Control. "
                              : "Not Attached to PhoXi Control. ")
                  << (DeviceInfo->Status.Ready ? "Ready to connect"
                                               : "Occupied")
                  << std::endl
                  << std::endl;
    }
}

void ReadPointCloudExample::ConnectPhoXiDeviceBySerialExample() {
    std::cout << std::endl
              << "Please enter the Hardware Identification Number: ";
    std::string HardwareIdentification;
    if (!ReadLine(HardwareIdentification)) {
        std::cout << "Incorrect input!" << std::endl;
        return;
    }

    pho::api::PhoXiTimeout Timeout = pho::api::PhoXiTimeout::ZeroTimeout;
    PhoXiDevice = Factory.CreateAndConnect(HardwareIdentification, Timeout);
    if (PhoXiDevice) {
        std::cout << "Connection to the device " << HardwareIdentification
                  << " was Successful!" << std::endl;
    } else {
        std::cout << "Connection to the device " << HardwareIdentification
                  << " was Unsuccessful!" << std::endl;
    }
}

void ReadPointCloudExample::CorrectDisconnectExample() {
    if (PhoXiDevice) {
        // The whole API is designed on C++ standards, using smart pointers and
        // constructor/destructor logic All resources will be closed automatically,
        // but the device state will not be affected -> it will remain connected in
        // PhoXi Control and if in freerun, it will remain Scanning. To Stop the
        // acquisition, just call
        PhoXiDevice->StopAcquisition();
        // If you want to disconnect and logout the device from PhoXi Control, so it
        // will then be available for other devices, call
        std::cout << "Do you want to logout the device? (0 if NO / 1 if YES) ";
        bool wantsToLogOut = true;
        if (!ReadLine(wantsToLogOut)) {
            wantsToLogOut = true;
        }
        PhoXiDevice->Disconnect(wantsToLogOut);
        // The call PhoXiDevice without Logout will be called automatically by
        // destructor
    }
}

void ReadPointCloudExample::PrintFrameInfo(const pho::api::PFrame &Frame) {
    const pho::api::FrameInfo &FrameInfo = Frame->Info;
    std::cout << "  Frame params: " << std::endl;
    std::cout << "    Frame Index: " << FrameInfo.FrameIndex << std::endl;
    std::cout << "    Frame Timestamp: " << FrameInfo.FrameTimestamp << " ms"
              << std::endl;
    std::cout << "    Frame Acquisition duration: " << FrameInfo.FrameDuration
              << " ms" << std::endl;
    std::cout << "    Frame Computation duration: "
              << FrameInfo.FrameComputationDuration << " ms" << std::endl;
    std::cout << "    Frame Transfer duration: "
              << FrameInfo.FrameTransferDuration << " ms" << std::endl;
    std::cout << "    Frame Acquisition time (PTP): "
              << FrameInfo.FrameStartTime.TimeAsString("%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "    Sensor Position: ["
              << FrameInfo.SensorPosition.x << "; "
              << FrameInfo.SensorPosition.y << "; "
              << FrameInfo.SensorPosition.z << "]" << std::endl;
    std::cout << "    Total scan count: " << FrameInfo.TotalScanCount << std::endl;
    std::cout << "    Color Camera Position: ["
              << FrameInfo.ColorCameraPosition.x << "; "
              << FrameInfo.ColorCameraPosition.y << "; "
              << FrameInfo.ColorCameraPosition.z << "]" << std::endl;
    std::cout << "    Current Camera Position: ["
              << FrameInfo.CurrentCameraPosition.x << "; "
              << FrameInfo.CurrentCameraPosition.y << "; "
              << FrameInfo.CurrentCameraPosition.z << "]"
              << std::endl;
    std::cout << "    FilenamePath: " << FrameInfo.FilenamePath << std::endl;
    std::cout << "    HWIdentification: " << FrameInfo.HWIdentification << std::endl;
    std::cout << "    Marker dot status: " << FrameInfo.MarkerDots.Status << std::endl;
}

void ReadPointCloudExample::PrintFrameData(const pho::api::PFrame &Frame) {
    if (Frame->Empty()) {
        std::cout << "Frame is empty.";
        return;
    }
    std::cout << "  Frame data: " << std::endl;
    if (!Frame->PointCloud.Empty()) {
        std::cout << "    PointCloud:    (" << Frame->PointCloud.Size.Width
                  << " x " << Frame->PointCloud.Size.Height
                  << ") Type: " << Frame->PointCloud.GetElementName()
                  << std::endl;
    }
    if (!Frame->NormalMap.Empty()) {
        std::cout << "    NormalMap:     (" << Frame->NormalMap.Size.Width
                  << " x " << Frame->NormalMap.Size.Height
                  << ") Type: " << Frame->NormalMap.GetElementName()
                  << std::endl;
    }
    if (!Frame->DepthMap.Empty()) {
        std::cout << "    DepthMap:      (" << Frame->DepthMap.Size.Width
                  << " x " << Frame->DepthMap.Size.Height
                  << ") Type: " << Frame->DepthMap.GetElementName()
                  << std::endl;
    }
    if (!Frame->ConfidenceMap.Empty()) {
        std::cout << "    ConfidenceMap: (" << Frame->ConfidenceMap.Size.Width
                  << " x " << Frame->ConfidenceMap.Size.Height
                  << ") Type: " << Frame->ConfidenceMap.GetElementName()
                  << std::endl;
    }
    if (!Frame->Texture.Empty()) {
        std::cout << "    Texture:       (" << Frame->Texture.Size.Width
                  << " x " << Frame->Texture.Size.Height
                  << ") Type: " << Frame->Texture.GetElementName() << std::endl;
    }
    if (!Frame->TextureRGB.Empty()) {
        std::cout << " TextureRGB:       (" << Frame->TextureRGB.Size.Width
                  << " x " << Frame->TextureRGB.Size.Height
                  << ") Type: " << Frame->TextureRGB.GetElementName() << std::endl;
    }
    if (!Frame->ColorCameraImage.Empty()) {
        std::cout << " ColorCameraImage:       (" << Frame->ColorCameraImage.Size.Width
                  << " x " << Frame->ColorCameraImage.Size.Height
                  << ") Type: " << Frame->ColorCameraImage.GetElementName() << std::endl;
    }
}

void ReadPointCloudExample::SoftwareTriggerExample() {
    // Check if the device is connected
    if (!PhoXiDevice || !PhoXiDevice->isConnected()) {
        std::cout << "Device is not created, or not connected!" << std::endl;
        return;
    }
    // If it is not in Software trigger mode, we need to switch the modes
    if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Software) {
        std::cout << "Device is not in Software trigger mode" << std::endl;
        if (PhoXiDevice->isAcquiring()) {
            std::cout << "Stopping acquisition" << std::endl;
            // If the device is in Acquisition mode, we need to stop the
            // acquisition
            if (!PhoXiDevice->StopAcquisition()) {
                throw std::runtime_error("Error in StopAcquisition");
            }
        }
        std::cout << "Switching to Software trigger mode " << std::endl;
        // Switching the mode is as easy as assigning of a value, it will call
        // the appropriate calls in the background
        PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
        // Just check if did everything run smoothly
        if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
            throw std::runtime_error(PhoXiDevice->TriggerMode.GetLastErrorMessage().c_str());
        }
    }

    // Start the device acquisition, if necessary
    if (!PhoXiDevice->isAcquiring()) {
        if (!PhoXiDevice->StartAcquisition()) {
            throw std::runtime_error("Error in StartAcquisition");
        }
    }
    // We can clear the current Acquisition buffer -- This will not clear Frames
    // that arrives to the PC after the Clear command is performed
    int ClearedFrames = PhoXiDevice->ClearBuffer();
    std::cout << ClearedFrames << " frames were cleared from the cyclic buffer" << std::endl;

    // While we checked the state of the StartAcquisition call, this check is
    // not necessary, but it is a good practice
    if (!PhoXiDevice->isAcquiring()) {
        std::cout << "Device is not acquiring" << std::endl;
        return;
    }
    for (std::size_t i = 0; i < 5; ++i) {
        std::cout << "Triggering the " << i << "-th frame" << std::endl;
        int FrameID = PhoXiDevice->TriggerFrame(
                /*If false is passed here, the device will reject the frame if it is not ready to be triggered, if true us supplied, it will wait for the trigger*/);
        if (FrameID < 0) {
            // If negative number is returned trigger was unsuccessful
            std::cout << "Trigger was unsuccessful! code=" << FrameID << std::endl;
            continue;
        } else {
            std::cout << "Frame was triggered, Frame Id: " << FrameID << std::endl;
        }

        std::cout << "Waiting for frame " << i << std::endl;
        // Wait for a frame with specific FrameID. There is a possibility, that
        // frame triggered before the trigger will arrive after the trigger
        // call, and will be retrieved before requested frame
        //  Because of this, the TriggerFrame call returns the requested frame
        //  ID, so it can than be retrieved from the Frame structure. This call
        //  is doing that internally in background
        pho::api::PFrame Frame =
                PhoXiDevice->GetSpecificFrame(FrameID /*, You can specify Timeout here - default is the Timeout stored in Timeout Feature -> Infinity by default*/);
        if (Frame) {
            PrintFrameInfo(Frame);
            PrintFrameData(Frame);
        } else {
            std::cout << "Failed to retrieve the frame!";
        }
        SampleFrame = Frame;
        DataHandlingExample();
    }
}

void ReadPointCloudExample::DataHandlingExample() {
    // Check if we have SampleFrame Data
    if (!SampleFrame || SampleFrame->Empty()) {
        std::cout << "Frame does not exist, or has no content!" << std::endl;
        return;
    }

    // We will count the number of measured points
    if (!SampleFrame->PointCloud.Empty()) {
        int MeasuredPoints = 0;
        pho::api::Point3_32f ZeroPoint(0.0f, 0.0f, 0.0f);
        for (int y = 0; y < SampleFrame->PointCloud.Size.Height; ++y) {
            for (int x = 0; x < SampleFrame->PointCloud.Size.Width; ++x) {
                if (SampleFrame->PointCloud[y][x] != ZeroPoint) {
                    MeasuredPoints++;
                }
            }
        }
        std::cout << "Your sample PointCloud has " << MeasuredPoints
                  << " measured points." << std::endl;

        float *MyLocalCopy =
                new float[SampleFrame->PointCloud.GetElementsCount() * 3];

        pho::api::Point3_32f *RawPointer = SampleFrame->PointCloud.GetDataPtr();
        memcpy(MyLocalCopy, RawPointer, SampleFrame->PointCloud.GetDataSize());
        // Data are organized as a matrix of X, Y, Z floats, see the
        // documentation for all other types

        delete[] MyLocalCopy;
        // Data from SampleFrame, or all other frames that are returned by the
        // device are copied from the Cyclic buffer and will remain in the
        // memory until the Frame will go out of scope You can specifically call
        // SampleFrame->PointCloud.Clear() to release some of the data
    }

    // You can store the Frame as a ply structure
    // If you don't specify Output folder the PLY file will be saved where
    // FullAPIExample_CSharp.exe is
    const auto outputFolder =
            OutputFolder.empty() ? std::string() : OutputFolder + DELIMITER;
    const auto sampleFramePly = outputFolder + "SampleFrame.ply";
    std::cout << "Saving frame as 'SampleFrame.ply'" << std::endl;
    if (SampleFrame->SaveAsPly(sampleFramePly, true, true)) {
        std::cout << "Saved sample frame as PLY to: " << sampleFramePly
                  << std::endl;
    } else {
        std::cout << "Could not save sample frame as PLY to " << sampleFramePly
                  << " !" << std::endl;
    }
    // You can save scans to any format, you only need to specify path + file
    // name API will look at extension and save the scan in the correct format
    // You can define which options to save (PointCloud, DepthMap, ...) in PhoXi
    // Control application -> Saving options, or set options directly from code
    // via optionsl 3rd parameter. This method has a an optional 2nd
    // parameter: FrameId Use this option to save other scans than the last one
    // Absolute path is prefered
    // If you don't specify Output folder the file will be saved to
    // %APPDATA%\PhotoneoPhoXiControl\ folder on Windows or
    // ~/.PhotoneoPhoXiControl/ on Linux
    const auto sampleFrameTiffFormat = outputFolder + "OtherSampleFrame.tif";
    if (PhoXiDevice->SaveLastOutput(sampleFrameTiffFormat)) {
        std::cout << "Saved sample frame to: " << sampleFrameTiffFormat
                  << std::endl;
    } else {
        std::cout << "Could not save sample frame to: " << sampleFrameTiffFormat
                  << " !" << std::endl;
    }
    // Overide saving options
    const auto sampleFramePrawFormat = outputFolder + "OtherSampleFrame.praw";
    const std::string jsonOptions = R"json({
        "UseCompression": true
    })json";
    if (PhoXiDevice->SaveLastOutput(sampleFramePrawFormat, -1, jsonOptions)) {
        std::cout << "Saved sample frame to: " << sampleFramePrawFormat
            << std::endl;
    }
    else {
        std::cout << "Could not save sample frame to: " << sampleFramePrawFormat
            << " !" << std::endl;
    }

    SavePointCloudToPTS(SampleFrame, outputFolder + "OtherSampleFrame.pts");

    // If you want OpenCV support, you need to link appropriate libraries and
    // add OpenCV include directory To add the support, add #define
    // PHOXI_OPENCV_SUPPORT before include of PhoXi include files For details
    // check also MinimalOpenCVExample
#ifdef PHOXI_OPENCV_SUPPORT
    if (!SampleFrame->PointCloud.Empty()) {
        cv::Mat PointCloudMat;
        if (SampleFrame->PointCloud.ConvertTo(PointCloudMat)) {
            cv::Point3f MiddlePoint = PointCloudMat.at<cv::Point3f>(
                    PointCloudMat.rows / 2, PointCloudMat.cols / 2);
            std::cout << "Middle point: " << MiddlePoint.x << "; "
                      << MiddlePoint.y << "; " << MiddlePoint.z;
        }
    }
#endif
    // If you want PCL support, you need to link appropriate libraries and add
    // PCL include directory To add the support, add #define PHOXI_PCL_SUPPORT
    // before include of PhoXi include files For details check also
    // MinimalPclExample
#ifdef PHOXI_PCL_SUPPORT
    // The PCL convert will convert the appropriate data into the pcl PointCloud
    // based on the Point Cloud type
    pcl::PointCloud<pcl::PointXYZRGBNormal> MyPCLCloud;
    SampleFrame->ConvertTo(MyPCLCloud);
#endif
}

static uint16_t normalization(uint16_t v, uint16_t min, uint16_t max) {
    const float range = 255;
    v = (uint16_t)(((float)v - min) / (max - min) * range);
    return v;
}

// https://paulbourke.net/dataformats/pts/
// A pts file is a simple text file used to store point data typically from LIDAR scanners.
// The first line gives the number of points to follow. Each subsequent line has 7 values,
// the first three are the (x,y,z) coordinates of the point, the fourth is an "intensity" value,
// and the last three are the (r,g,b) colour estimates. The (r,g,b) values range from 0 to 255 (single unsigned byte).
// The intensity value is an estimate of the fraction of incident radiation reflected by the surface at that point,
// 0 indicates is a very poor return while 255 is a very stong return.
// Example:
// Line1: 253730194
// Line2: -0.41025 -2.0806 8.00981 55 52 44 65
// Line3: -0.63016 -1.84527 6.59447 228 228 230 225
// ...
// LineN: -0.4766 -2.14446 7.91288 60 56 54 68
//
// This file can be opened in the CloudCompare tool.
void ReadPointCloudExample::SavePointCloudToPTS(const pho::api::PFrame &Frame, const std::string &ptsFilepath) {
    std::ofstream ptsFile(ptsFilepath);
    if (!ptsFile.is_open()) {
        std::cout << "Can't open " << ptsFilepath << " for writing!" << std::endl;
        return;
    }

    const pho::api::Point3_32f* pcPtr = Frame->PointCloud.GetDataPtr();
    // Extract texture data in RGB format independent if the texture is grayscale or RGB
    auto textureRGB = [Frame](size_t index, uint16_t &min, uint16_t& max) -> pho::api::ColorRGB_16 {
        if (Frame->TextureRGB.Empty()) {
            uint16_t gray = static_cast<uint16_t>(*(Frame->Texture.GetDataPtr() + index));
            min = std::min(min, gray);
            max = std::max(max, gray);
            return pho::api::ColorRGB_16(gray, gray, gray);
        } else {
            pho::api::ColorRGB_16& rgb = *(Frame->TextureRGB.GetDataPtr() + index);
            min = std::min(min, (uint16_t)rgb.r);
            max = std::max(max, (uint16_t)rgb.r);
            min = std::min(min, (uint16_t)rgb.g);
            max = std::max(max, (uint16_t)rgb.g);
            min = std::min(min, (uint16_t)rgb.b);
            max = std::max(max, (uint16_t)rgb.b);
            return rgb;
        }
    };

    pho::api::MatType<pho::api::float32_t> zero(0.);
    std::vector<std::pair<pho::api::Point3_32f, pho::api::ColorRGB_16>> pc;
    uint16_t textureMin = std::numeric_limits<uint16_t>::max();
    uint16_t textureMax = std::numeric_limits<uint16_t>::min();
    // Save only valid 3D points followed with color in RGB
    for (size_t i = 0; i < Frame->PointCloud.GetElementsCount(); ++i) {
        const pho::api::Point3_32f *pnt = pcPtr + i;
        if (pnt->x != zero && pnt->y != zero && pnt->z != zero) {
            const pho::api::ColorRGB_16 rgb = textureRGB(i, textureMin, textureMax);
            pc.push_back(std::make_pair(*pnt, rgb));
        }
    }

    const std::string defaultIntensity = "100";
    ptsFile << pc.size() << "\n";
    for (auto &vertex : pc) {
        const pho::api::Point3_32f& pnt = vertex.first;
        pho::api::ColorRGB_16 rgb = vertex.second;
        rgb.r = normalization(rgb.r, textureMin, textureMax);
        rgb.g = normalization(rgb.g, textureMin, textureMax);
        rgb.b = normalization(rgb.b, textureMin, textureMax);
        ptsFile << pnt.x << " " << pnt.y << " " << pnt.z << " " << rgb.r << " " << rgb.g << " " << rgb.b << " "
            << defaultIntensity << "\n";
    }

    std::cout << "Saved sample frame to: " << ptsFilepath << std::endl;

    ptsFile.close();
}

void ReadPointCloudExample::Run() {
    try {
        GetAvailableDevicesExample();
        ConnectPhoXiDeviceBySerialExample();
        SoftwareTriggerExample();
        CorrectDisconnectExample();
    } catch (std::runtime_error &InternalException) {
        std::cout << std::endl
                  << "Exception was thrown: " << InternalException.what()
                  << std::endl;
        if (PhoXiDevice->isConnected()) {
            PhoXiDevice->Disconnect(true);
        }
    }
}

int main(int argc, char *argv[]) {
    ReadPointCloudExample Example;
    Example.Run();
    return 0;
}

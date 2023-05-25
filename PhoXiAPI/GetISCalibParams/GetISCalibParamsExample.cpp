/*
* Photoneo's API Example - GetISCalibParams.cpp
* Prints out image sensor calibration parameters.
*/

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "PhoXi.h"

//Print out list of device info to standard output
void printDeviceInfoList(const std::vector<pho::api::PhoXiDeviceInformation> &DeviceList);
//Print out device info to standard output
void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo);
//Print out calibration parameters
void printCalibParams(pho::api::PPhoXi &PhoXiDevice);
//Print out calibration parameters
void printFrameCalibParams(pho::api::PPhoXi &PhoXiDevice);
//Print out scanning volume information
void printScanningVolumes(pho::api::PPhoXi& PhoXiDevice);
//Print out a 3x3 matrix with a given name
void printMatrix(const std::string &Name, const pho::api::CameraMatrix64f &Matrix);
// print out the distortion coefficients
void printDistortionCoefficients(const std::string& name, const std::vector<double>& distCoeffs);

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
            << (std::string) PhoXiDevice->HardwareIdentification << std::endl;
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

    //Print out calibration parameters
    printCalibParams(PhoXiDevice);

    if (PhoXiDevice->GetType() == pho::api::PhoXiDeviceType::MotionCam3D) {
        // Scanning volume for the motion camera devices is available after triggering the first frame
        int id = PhoXiDevice->TriggerFrame();
        if (id >= 0) {
            pho::api::PFrame frame = PhoXiDevice->GetSpecificFrame(id);
            frame;
        }
    }
    //Print out scanning volume information
    printScanningVolumes(PhoXiDevice);

    //Print out calibration parameters from the frame
    printFrameCalibParams(PhoXiDevice);

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

void printCalibParams(pho::api::PPhoXi &PhoXiDevice)
{
    pho::api::PhoXiCalibrationSettings CalibrationSettings = PhoXiDevice->CalibrationSettings;

    std::cout << "CalibrationSettings: " << std::endl;
    std::cout << "  FocusLength: " << CalibrationSettings.FocusLength << std::endl;
    std::cout << "  PixelSize: "
        << CalibrationSettings.PixelSize.Width << " x "
        << CalibrationSettings.PixelSize.Height
        << std::endl;
    printMatrix("CameraMatrix", CalibrationSettings.CameraMatrix);
    std::cout << "  DistortionCoefficients: " << std::endl;
    std::cout << "    Format is the following: " << std::endl;
    std::cout << "    (k1, k2, p1, p2[, k3[, k4, k5, k6[, s1, s2, s3, s4[, tx, ty]]]])" << std::endl;

    std::vector<double> distCoeffs = CalibrationSettings.DistortionCoefficients;
    std::stringstream currentDistCoeffsSS;
    std::size_t brackets = 0;
    currentDistCoeffsSS << "(";
    currentDistCoeffsSS << distCoeffs[0];
    for (std::size_t i = 1; i < distCoeffs.size(); ++i)
    {
        if (i == 4 || i == 5 || i == 8 || i == 12 || i == 14)
        {
            currentDistCoeffsSS << "[";
            ++brackets;
        }
        currentDistCoeffsSS << ", " << distCoeffs[i];
    }
    for (std::size_t j = 0; j < brackets; ++j)
    {
        currentDistCoeffsSS << "]";
    }
    currentDistCoeffsSS << ")";
    std::cout << "    " << currentDistCoeffsSS.str() << std::endl;
}

void printFrameCalibParams(pho::api::PPhoXi& PhoXiDevice)
{
    if (!PhoXiDevice->isAcquiring())
    {
        PhoXiDevice->StartAcquisition();
    }
    int FrameID = PhoXiDevice->TriggerFrame();
    if (FrameID < 0)
    {
        //If negative number is returned trigger was unsuccessful
        std::cout << "Trigger was unsuccessful! code=" << FrameID << std::endl;
        return;
    }

    pho::api::PFrame Frame = PhoXiDevice->GetSpecificFrame(FrameID);
    if (Frame)
    {
        printMatrix("Frame CameraMatrix", Frame->Info.CameraMatrix);
        printDistortionCoefficients("Frame DistortionCoefficients", Frame->Info.DistortionCoefficients);
        std::cout << "  Camera binning height: " << Frame->Info.CameraBinning.Height << std::endl;
        std::cout << "  Camera binning width: " << Frame->Info.CameraBinning.Width << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve the frame!";
    }


    pho::api::PhoXiCalibrationSettings CalibrationSettings = PhoXiDevice->CalibrationSettings;

    std::cout << "CalibrationSettings: " << std::endl;
    std::cout << "  FocusLength: " << CalibrationSettings.FocusLength << std::endl;
    std::cout << "  PixelSize: "
        << CalibrationSettings.PixelSize.Width << " x "
        << CalibrationSettings.PixelSize.Height
        << std::endl;
    printMatrix("CameraMatrix", CalibrationSettings.CameraMatrix);
    printDistortionCoefficients("DistortionCoefficients", CalibrationSettings.DistortionCoefficients);
}

void printDistortionCoefficients(const std::string &name, const std::vector<double> &distCoeffs)
{
    std::cout << "  " << name << ": " << std::endl;
    std::cout << "    Format is the following: " << std::endl;
    std::cout << "    (k1, k2, p1, p2[, k3[, k4, k5, k6[, s1, s2, s3, s4[, tx, ty]]]])" << std::endl;

    std::stringstream currentDistCoeffsSS;
    std::size_t brackets = 0;
    currentDistCoeffsSS << "(";
    currentDistCoeffsSS << distCoeffs[0];
    for (std::size_t i = 1; i < distCoeffs.size(); ++i)
    {
        if (i == 4 || i == 5 || i == 8 || i == 12 || i == 14)
        {
            currentDistCoeffsSS << "[";
            ++brackets;
        }
        currentDistCoeffsSS << ", " << distCoeffs[i];
    }
    for (std::size_t j = 0; j < brackets; ++j)
    {
        currentDistCoeffsSS << "]";
    }
    currentDistCoeffsSS << ")";
    std::cout << "    " << currentDistCoeffsSS.str() << std::endl;
}

void printScanningVolumes(pho::api::PPhoXi& PhoXiDevice)
{
    // Scanning volume for the motion camera devices is available after triggering the first frame
    if (PhoXiDevice->GetType() == pho::api::PhoXiDeviceType::MotionCam3D) {
        int id = PhoXiDevice->TriggerFrame();
        if (id >= 0) {
            auto frame = PhoXiDevice->GetSpecificFrame(id);
            frame && frame->Empty();
        }
    }

    pho::api::PhoXiScanningVolume ScanningVolume = PhoXiDevice->ScanningVolume;

    auto point2str = [](const pho::api::Point3_64f& point) -> std::string
    {
        std::stringstream stream;
        stream << "[" << point.x << ", " << point.y << ", " << point.z << "]";
        return stream.str();
    };

    const size_t cuttingPlanesCount = ScanningVolume.CuttingPlanes.size();
    std::cout << "ScanningVolume: " << std::endl;

    std::cout << "  CuttingPlanesCount: " << cuttingPlanesCount << std::endl;
    for (std::size_t i = 0; i < cuttingPlanesCount; ++i)
    {
        const auto& plane = ScanningVolume.CuttingPlanes[i];
        std::cout << "    Plane[" << i << "]: " << std::endl;
        std::cout << "      normal = " << point2str(plane.normal) << std::endl;
        std::cout << "      d = " << plane.d << std::endl;
    }

    const auto& geometry = ScanningVolume.ProjectionGeometry;
    std::cout << "  ProjectionGeometry: " << std::endl;
    std::cout << "      Origin = "
        << point2str(geometry.Origin) << std::endl;
    std::cout << "      TopLeftTangentialVector = " 
        << point2str(geometry.TopLeftTangentialVector) << std::endl;
    std::cout << "      TopRightTangentialVector = "
        << point2str(geometry.TopRightTangentialVector) << std::endl;
    std::cout << "      BottomLeftTangentialVector = "
        << point2str(geometry.BottomLeftTangentialVector) << std::endl;
    std::cout << "      BottomRightTangentialVector = "
        << point2str(geometry.BottomRightTangentialVector) << std::endl;

    const size_t topContourPointsCount = geometry.TopContourPoints.size();
    std::cout << std::endl << "      TopContourPointsCount: " << topContourPointsCount << std::endl;
    const auto& topContours = geometry.TopContourPoints;
    for (std::size_t i = 0; i < topContourPointsCount; ++i)
    {
        std::cout << "        [" << i << "]: " << point2str(topContours[i]) << std::endl;
    }

    const size_t bottomContourPointsCount = geometry.BottomContourPoints.size();
    std::cout << std::endl << "      BottomContourPointsCount: " << bottomContourPointsCount << std::endl;
    const auto& bottomContours = geometry.BottomContourPoints;
    for (std::size_t i = 0; i < bottomContourPointsCount; ++i)
    {
        std::cout << "        [" << i << "]: " << point2str(bottomContours[i]) << std::endl;
    }

    const auto& mesh = ScanningVolume.Mesh;
    if (mesh.IsValid()) {
        std::cout << std::endl << "      Scanning volume:" << std::endl;
        std::cout <<              "        Count of cross sections: " << mesh.CrossSectionCount() << std::endl;
        std::cout <<              "        Count of point in section: " << mesh.PointsPerSection << std::endl;
        std::cout <<              "        Points:" << std::endl;
        auto pointsInSectionCounter = 0;
        const auto verticesSize = static_cast<int>(mesh.Vertices.size());
        for (auto meshVerticesIdx = 0; meshVerticesIdx < verticesSize; ++meshVerticesIdx) {
            if (!pointsInSectionCounter || !(pointsInSectionCounter % mesh.PointsPerSection)) {
                std::cout << std::endl << "         Section #" << pointsInSectionCounter << ": ";
            }

            std::cout << "[" << mesh.Vertices[meshVerticesIdx].x << "; " << mesh.Vertices[meshVerticesIdx].y << "; " << mesh.Vertices[meshVerticesIdx].z << "], ";
            ++pointsInSectionCounter;
        }
        std::cout << std::endl;

        std::cout << std::endl << "       Mesh triangles:" << std::endl;
        std::cout << "[";
        for (auto meshVerticesIdx = 0; meshVerticesIdx < verticesSize - 3; meshVerticesIdx += 3) {
            std::cout << "[" << mesh.Vertices[mesh.Indices[meshVerticesIdx]].x << "; " << mesh.Vertices[mesh.Indices[meshVerticesIdx + 1]].y << "; "
                      << mesh.Vertices[mesh.Indices[meshVerticesIdx + 2]].z << "], ";
        }
        std::cout << "]" << std::endl;
    }
}

void printMatrix(const std::string &name, const pho::api::CameraMatrix64f &matrix)
{
    if (matrix.Empty()) {
        std::cout << "  " << name << ": empty"
            << std::endl;
    }
    else
    {
        std::cout << "  " << name << ": "
            << std::endl << "    ["
            << matrix[0][0] << ", "
            << matrix[0][1] << ", "
            << matrix[0][2] << "]"

            << std::endl << "    ["
            << matrix[1][0] << ", "
            << matrix[1][1] << ", "
            << matrix[1][2] << "]"

            << std::endl << "    ["
            << matrix[2][0] << ", "
            << matrix[2][1] << ", "
            << matrix[2][2] << "]"
            << std::endl;
    }
}


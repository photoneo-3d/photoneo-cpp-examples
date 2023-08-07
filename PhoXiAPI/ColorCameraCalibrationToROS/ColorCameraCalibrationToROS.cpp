/*
* Photoneo's API Example - ColorCameraCalibrationToROS.cpp
* Prints ColorCamera calibration parameters in ROS compatible yaml format.
* This is calibration for raw RGB image respecting currently set resolution.
* Note that this is different from device computed RGB texture.
*
* Modified from ExtendRobotics's FrameCalibrationToROS
*
* Mofification Contributors:
* - Bartosz Meglicki <bartosz.meglicki@extendrobotics.com> (2023)
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
void printFrameCalibParams(pho::api::PPhoXi &PhoXiDevice);
//Print out scanning volume information
void printMatrix(const std::string &Name, const pho::api::CameraMatrix64f &Matrix);
// print out the distortion coefficients
void printDistortionCoefficients(const std::vector<double>& distCoeffs);

int main(int argc, char *argv[])
{
    pho::api::PhoXiFactory Factory;

    //Check if the PhoXi Control Software is running
    if (!Factory.isPhoXiControlRunning())
    {
        std::cerr << "PhoXi Control Software is not running" << std::endl;
        return 0;
    }

    //Get List of available devices on the network
    std::vector <pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
    if (DeviceList.empty())
    {
        std::cerr << "PhoXi Factory has found 0 devices" << std::endl;
        return 0;
    }
    printDeviceInfoList(DeviceList);

    //Try to connect device opened in PhoXi Control, if any
    pho::api::PPhoXi PhoXiDevice = Factory.CreateAndConnectFirstAttached();
    if (PhoXiDevice)
    {
        std::cout << "# You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: "
            << (std::string) PhoXiDevice->HardwareIdentification << std::endl;
    }
    else
    {
        std::cout << "# You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to first device in device list" << std::endl;
        PhoXiDevice = Factory.CreateAndConnect(DeviceList.front().HWIdentification);
    }

    //Check if device was created
    if (!PhoXiDevice)
    {
        std::cerr << "Your device was not created!" << std::endl;
        return 0;
    }

    //Check if device is connected
    if (!PhoXiDevice->isConnected())
    {
        std::cerr << "Your device is not connected" << std::endl;
        return 0;
    }

    std::cout << std::endl;

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
        std::cout << "# Device: " << i << std::endl;
        printDeviceInfo(DeviceList[i]);
    }
}

void printDeviceInfo(const pho::api::PhoXiDeviceInformation &DeviceInfo)
{
    std::cout << "#  Name:                    " << DeviceInfo.Name << std::endl;
    std::cout << "#  Hardware Identification: " << DeviceInfo.HWIdentification << std::endl;
    std::cout << "#  Type:                    " << std::string(DeviceInfo.Type) << std::endl;
    std::cout << "#  Firmware version:        " << DeviceInfo.FirmwareVersion << std::endl;
    std::cout << "#  Variant:                 " << DeviceInfo.Variant << std::endl;
    std::cout << "#  IsFileCamera:            " << (DeviceInfo.IsFileCamera ? "Yes" : "No") << std::endl;
    std::cout << "#  Feature-Alpha:           " << (DeviceInfo.CheckFeature("Alpha") ? "Yes" : "No") << std::endl;
    std::cout << "#  Feature-Color:           " << (DeviceInfo.CheckFeature("Color") ? "Yes" : "No") << std::endl;
    std::cout << "#  Status:                  "
        << (DeviceInfo.Status.Attached ? "Attached to PhoXi Control. " : "Not Attached to PhoXi Control. ")
        << (DeviceInfo.Status.Ready ? "Ready to connect" : "Occupied")
        << std::endl << std::endl;
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
        std::cerr << "Trigger was unsuccessful! code=" << FrameID << std::endl;
        return;
    }

    pho::api::PFrame Frame = PhoXiDevice->GetSpecificFrame(FrameID);

    if(!Frame)
    {
        std::cerr << "Failed to retrieve the frame!" << std::endl;
        return;
    }

    const pho::api::TextureRGB16 &ColorFrame = Frame->ColorCameraImage;

    if(ColorFrame.Empty())
    {
        std::cerr << "Failed to retrieve the color camera image!" << std::endl
                  << "Make sure that ColorCameraImage transfer is enabled in PhoXiControl" << std::endl
                  << "Make sure that appriopriate settings for color camera are set" << std::endl;
        return;
    }

    std::cout << "# ColorCameraImage (raw RGB, this is not depth aligned RGB texture!)" << std::endl;
    std::cout << "# ColorCameraScale: " << Frame->Info.ColorCameraScale.Width << "x" << Frame->Info.ColorCameraScale.Height << std::endl;

    std::cout << "image_width: " << ColorFrame.Size.Width << std::endl;
    std::cout << "image_height: " << ColorFrame.Size.Height << std::endl;
    std::cout << "camera_name: " << (std::string) PhoXiDevice->HardwareIdentification << std::endl;

    printMatrix("camera_matrix", Frame->Info.ColorCameraMatrix);
    printDistortionCoefficients(Frame->Info.ColorCameraDistortionCoefficients);

    // rectification matrix - we are not stereo camera so we need identity
    pho::api::CameraMatrix64f ID(std::vector<int>{3, 3});
    ID[0][0] = 1.0, ID[0][1] = 0.0, ID[0][2] = 0.0;
    ID[1][0] = 0.0, ID[1][1] = 1.0, ID[1][2] = 0.0;
    ID[2][0] = 0.0, ID[2][1] = 0.0, ID[2][2] = 1.0;

    printMatrix("rectification_matrix", ID);

    // projection matrix - we don't have rectified image
    // so P[1:3,1:3] = K and Tx = Ty = 0 where K is camera_matrix
    pho::api::CameraMatrix64f P(std::vector<int>{3, 4});
    const pho::api::CameraMatrix64f &K = Frame->Info.CameraMatrix;
    for(int r=0;r<K.Size.Height;++r)
      for(int c=0;c<K.Size.Width;++c)
        P[r][c]=K[r][c];
    // zero out Tx, Ty
    P[0][3] = P[1][3] = P[2][3] = 0.0;

    printMatrix("projection_matrix", P);
}

void printDistortionCoefficients(const std::vector<double> &distCoeffs)
{
    //plumb bob is k1, k2, p1, p2, k3
    const uint LAST_PLUMB_BOB_INDEX = 4;
    //last index is k3, 4th

    for(uint i=0;i<distCoeffs.size();++i)
      if(i > LAST_PLUMB_BOB_INDEX && distCoeffs[i] != 0.0)
      {
        std::cerr << "only plumb_bob (k1, k2, p1, p2, k3 " << " is supported now!" << std::endl
                  << " different model detected!" << std::endl;
        return;
      }

    std::cout << "distortion_model: plumb_bob" << std::endl;
    std::cout << "distortion_coefficients: " << std::endl
              << "  rows: 1" << std::endl
              << "  cols: "  << LAST_PLUMB_BOB_INDEX+1 << std::endl
              << "  data: [";

    for(uint i=0;i<=LAST_PLUMB_BOB_INDEX;++i)
    {
      std::cout << distCoeffs[i];
      if(i<LAST_PLUMB_BOB_INDEX)
        std::cout << ", ";
    }

    std::cout << "]" << std::endl;
}

void printMatrix(const std::string &name, const pho::api::CameraMatrix64f &matrix)
{
    if (matrix.Empty()) {
        std::cout << "  " << name << ": empty" << std::endl;
        return;
    }

    std::cout << name << ": " << std::endl
              << "  rows: " << matrix.Size.Height << std::endl
              << "  cols: " << matrix.Size.Width << std::endl
              << "  data: [";

    for(int r=0;r<matrix.Size.Height;++r)
      for(int c=0;c<matrix.Size.Width;++c)
      {
        std::cout << matrix[r][c];
        if(! (r == matrix.Size.Height-1 && c == matrix.Size.Width -1) )
          std::cout << ", ";
      }

    std::cout << "]" << std::endl;
}

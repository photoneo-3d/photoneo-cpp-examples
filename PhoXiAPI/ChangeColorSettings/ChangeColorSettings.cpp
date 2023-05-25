/*
* Photoneo's API Example - ChangeColorSettingsExample.cpp
* Defines the entry point for the console application.
* Demonstrates the extended functionality of PhoXi devices. This Example shows how to change the settings of the device.
* Points out the correct way to disconnect the device from PhoXiControl.
*/

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "PhoXi.h"

//The whole api is in namespace pho (Photoneo) :: api
class ColorAPIExample
{
private:
	pho::api::PhoXiFactory Factory;
	pho::api::PPhoXi PhoXiDevice;
	pho::api::PFrame SampleFrame;
	std::vector<pho::api::PhoXiDeviceInformation> DeviceList;

	void CorrectDisconnectExample();
	void ConnectPhoXiDeviceBySerialExample();
	void PrintColorSettings(const pho::api::PhoXiColorSettings& ColorSettings);
	void ChangeColorSettingsExample();
	void ComputeCustomWhiteBalanceExample();
	template<class T>
	bool ReadLine(T &Output) const
	{
		std::string Input;
		std::getline(std::cin, Input);
		std::stringstream InputSteam(Input);
		return (InputSteam >> Output) ? true : false;
	}
	bool ReadLine(std::string &Output) const
	{
		std::getline(std::cin, Output);
		return true;
	}

public:
	ColorAPIExample() {};
	~ColorAPIExample() {};
	void Run();
};

void ColorAPIExample::ConnectPhoXiDeviceBySerialExample()
{
	std::cout << std::endl << "Please enter the Hardware Identification Number (for example 'YYYY-MM-###-LC#'): ";
	std::string HardwareIdentification;
	if (!ReadLine(HardwareIdentification))
	{
		std::cout << "Incorrect input!" << std::endl;
		return;
	}

	pho::api::PhoXiTimeout Timeout = pho::api::PhoXiTimeout::ZeroTimeout;
	PhoXiDevice = Factory.CreateAndConnect(HardwareIdentification, Timeout);
	if (PhoXiDevice)
	{
		std::cout << "Connection to the device " << HardwareIdentification << " was Successful!" << std::endl;
	}
	else
	{
		std::cout << "Connection to the device " << HardwareIdentification << " was Unsuccessful!" << std::endl;
	}
}

void ColorAPIExample::ChangeColorSettingsExample()
{
	std::cout << "Change Color Settings Example" << std::endl;
	std::cout << std::endl;

	//check if device support Color settings
	if( !PhoXiDevice->Info().CheckFeature("Color")){
		std::cout << "Device does not support Color features" << std::endl;
		return;
	}

	//Retrieving the current ColorSettings
	const pho::api::PhoXiColorSettings CurrentColorSettings = PhoXiDevice->ColorSettings;
	//Check if the Current Color Settings have been retrieved succesfully
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage().c_str());
	}

	std::cout << "Settings before set up:" << std::endl;
	PrintColorSettings(CurrentColorSettings);

	//get SupportedColorIso
	std::vector<int> supportedIso = PhoXiDevice->SupportedColorIso;
	if (!PhoXiDevice->SupportedColorIso.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->SupportedColorIso.GetLastErrorMessage().c_str());
	}
	//Pick an iso value from vector
	PhoXiDevice->ColorSettings->Iso = supportedIso.front();
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage().c_str());
	}

	//Get SupportedColorExposure
	std::vector<double> supportedExposure = PhoXiDevice->SupportedColorExposure;
	if (!PhoXiDevice->SupportedColorExposure.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->SupportedColorExposure.GetLastErrorMessage().c_str());
	}
	//Pick an exposure value from vector
	PhoXiDevice->ColorSettings->Exposure = supportedExposure.back();
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage().c_str());
	}

	//Get all SupportedColorCapturingModes
	std::vector<pho::api::PhoXiCapturingMode> CapturingModes = PhoXiDevice->SupportedColorCapturingModes;
	//Check if the SupportedColorCapturingModes have been retrieved succesfully
	if (!PhoXiDevice->SupportedColorCapturingModes.isLastOperationSuccessful())
	{
		throw std::runtime_error(PhoXiDevice->SupportedColorCapturingModes.GetLastErrorMessage().c_str());
	}
	//Pick a capturing mode
	PhoXiDevice->ColorSettings->CapturingMode = CapturingModes[0];
	//Check if the Resolution (SupportedColorCapturingModes) has been changed succesfully
	if (!PhoXiDevice->CapturingMode.isLastOperationSuccessful())
	{
		throw std::runtime_error(PhoXiDevice->CapturingMode.GetLastErrorMessage().c_str());
	}

	//Get all SupportedColorWhiteBalancePresets
	std::vector<std::string> WhiteBalancePresets = PhoXiDevice->SupportedColorWhiteBalancePresets;
	//Check if the SupportedColorWhiteBalancePresets have been retrieved succesfully
	if (!PhoXiDevice->SupportedColorWhiteBalancePresets.isLastOperationSuccessful())
	{
		throw std::runtime_error(PhoXiDevice->SupportedColorWhiteBalancePresets.GetLastErrorMessage().c_str());
	}
	//pick a white balance preset
	PhoXiDevice->ColorSettings->WhiteBalance.Preset = WhiteBalancePresets[0];
	//Check if the white balance has been changed succesfully
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful())
	{
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage().c_str());
	}

	//set RGB values for white balance (R,G,B)
	pho::api::Point3_64f rgb(0.5, 0.5, 0.7);
	PhoXiDevice->ColorSettings->WhiteBalance.BalanceRGB = rgb;
	//Check if the white balance has been changed succesfully
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful())
	{
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage().c_str());
	}

	std::cout << "Settings after set up:" << std::endl;
	PrintColorSettings(PhoXiDevice->ColorSettings);

	//Restore previous values, if you want to return to previous state
	PhoXiDevice->ColorSettings = CurrentColorSettings;
}

void ColorAPIExample::ComputeCustomWhiteBalanceExample() 
{
	//The white balance factors can be also computed by the camera from a frame
	std::cout << "Compute Custom White Balance Example" << std::endl;
	std::cout << std::endl;
	
	//Check if device support Color settings
	if( !PhoXiDevice->Info().CheckFeature("Color")){
		std::cout << "Device does not support Color features" << std::endl;
		return;
	}

	//Start camera acquisition if not acquiring
	if (!PhoXiDevice->isAcquiring() && !PhoXiDevice->StartAcquisition()) {
		throw std::runtime_error("Failed to start acquisition");
	}

	//Set a software trigger
	PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
	if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->TriggerMode.GetLastErrorMessage());
	}

	//Enable ComputeCustomWhiteBalance setting
	PhoXiDevice->ColorSettings->WhiteBalance.ComputeCustomWhiteBalance = true;
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage());
	}

	//Trigger a frame
	int frameId = PhoXiDevice->TriggerFrame(true, true);
	if (frameId < 0) {
		throw std::runtime_error("Failed to trigger a frame");
	}

	pho::api::PFrame frame = PhoXiDevice->GetSpecificFrame(frameId);
	if (frame == nullptr) {
		throw std::runtime_error("Failed to acquire a frame");
	}

	//The computed white balance factors are NOT updated in ColorSettings white balance structure
	//but are present in acquired frame info structure
	std::cout << "Computed white balance factors:" << std::endl;
	std::cout << "R: " << frame->Info.BalanceRGB.x << std::endl;
	std::cout << "G: " << frame->Info.BalanceRGB.y << std::endl;
	std::cout << "B: " << frame->Info.BalanceRGB.z << std::endl;

	//If white balance settings are acceptable, they can be made persistent by setting them 
	//into ColorSetting structure and disabling ComputeCustomWhiteBalance setting
	PhoXiDevice->ColorSettings->WhiteBalance.BalanceRGB = frame->Info.BalanceRGB;
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage());
	}

	PhoXiDevice->ColorSettings->WhiteBalance.ComputeCustomWhiteBalance = false;
	if (!PhoXiDevice->ColorSettings.isLastOperationSuccessful()) {
		throw std::runtime_error(PhoXiDevice->ColorSettings.GetLastErrorMessage());
	}

	std::cout << "Settings after ComputeCustomWhiteBalance:" << std::endl;
	PrintColorSettings(PhoXiDevice->ColorSettings);
}

void ColorAPIExample::CorrectDisconnectExample()
{
	//The whole API is designed on C++ standards, using smart pointers and constructor/destructor logic
	//All resources will be closed automatically, but the device state will not be affected -> it will remain connected in PhoXi Control and if in freerun, it will remain Scanning
	//To Stop the device, just
	PhoXiDevice->StopAcquisition();
	//If you want to disconnect and logout the device from PhoXi Control, so it will then be available for other devices, call
	std::cout << "Do you want to logout the device? (0 if NO / 1 if YES) ";
	bool Entry;
	if (!ReadLine(Entry))
	{
		return;
	}
	PhoXiDevice->Disconnect(Entry);
	//The call PhoXiDevice without Logout will be called automatically by destructor
}

void ColorAPIExample::PrintColorSettings(const pho::api::PhoXiColorSettings &ColorSettings)
{
	std::cout << "  ColorSettings: " << std::endl;
	std::cout << "    Iso:                " << ColorSettings.Iso << std::endl;
	std::cout << "    Exposure:           " << ColorSettings.Exposure << std::endl;
	std::cout << "    Resolution: Width:  " << ColorSettings.CapturingMode.Resolution.Width << std::endl;
	std::cout << "                Height: " << ColorSettings.CapturingMode.Resolution.Height << std::endl;
	std::cout << "    WhiteBalancePreset: " << ColorSettings.WhiteBalance.Preset << std::endl;
	std::cout << "    WhiteBalance:    R: " << ColorSettings.WhiteBalance.BalanceRGB.x << std::endl;
	std::cout << "    WhiteBalance:    G: " << ColorSettings.WhiteBalance.BalanceRGB.y << std::endl;
	std::cout << "    WhiteBalance:    B: " << ColorSettings.WhiteBalance.BalanceRGB.z << std::endl;
}

void ColorAPIExample::Run()
{
	try
	{	//Connecting to scanner
		ConnectPhoXiDeviceBySerialExample();
		std::cout << std::endl;

		//Checks
		//Check if the device is connected
		if (!PhoXiDevice || !PhoXiDevice->isConnected())
		{
			std::cout << "Device is not created, or not connected!" << std::endl;
			return;
		}
		//Check if the CapturingSettings are Enabled and Can be Set
		if (!PhoXiDevice->ColorSettings.isEnabled() ||
			!PhoXiDevice->ColorSettings.CanSet() ||
			!PhoXiDevice->ColorSettings.CanGet()) {
			std::cout << "Color Settings are not supported by the Device Hardware, or are Read only on the specific device" << std::endl;

			//Disconnect with logout and stop acquisition
			PhoXiDevice->Disconnect(true, true);
			return;
		}

		ChangeColorSettingsExample();
		ComputeCustomWhiteBalanceExample();

		CorrectDisconnectExample();
	}
	catch (std::runtime_error &InternalException)
	{
		std::cout << std::endl << "Exception was thrown: " << InternalException.what() << std::endl;
		if (PhoXiDevice->isConnected())
		{
			PhoXiDevice->Disconnect(true);
		}
	}
}

int main(int argc, char *argv[])
{
	ColorAPIExample Example;
	Example.Run();
	return 0;
}

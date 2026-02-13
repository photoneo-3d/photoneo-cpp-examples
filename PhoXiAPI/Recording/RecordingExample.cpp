/*
* Photoneo's API Example - RecordingExample.cpp
* Sets up and starts recording. The complete list of recording options
* in json schema format can be found in the PhoXi API documentation:
*  [phoxi_install_dir]/API/RecordingOptionsSchema.json]
* 
*/

#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <regex>

#include "PhoXi.h"

// Setup PLY recording with enabled Point Cloud, Depth Map and Texture
/* Note:
 * When `folder` path specified is a relative path, then it is relative to PhoXiControl working directory:
 * Windows: 'c:\Users\{user name}\AppData\Roaming\PhotoneoPhoXiControl'
 * Linux: '~/.PhotoneoPhoXiControl'
 */
bool enablePlyFile(pho::api::PPhoXi PhoXiDevice) {
    auto plyRecordingOptions = R"json({
        "folder": "RecordingExampleOutput",
        "every": 1,
        "capacity": -1,
        "pattern": "my_scan_####",
        "overwrite_existing": true,
        "containers": {
            "ply": {
                "enabled": true,
                "point_cloud": true,
                "depth_map": true,
                "texture": true
            }
        }
    })json";

    // Start recording with setup json options for PLY container, do not store options persistently
    pho::api::PhoXi::StartRecordingResult ret = PhoXiDevice->StartRecording(plyRecordingOptions, false);
    if (ret != pho::api::PhoXi::StartRecordingResult::Success) {
        std::cout << "Failed to start recording for PLY container! Error: " << static_cast<int>(ret) << std::endl;
        return false;
    }

    return true;
}

// Example of diagnostic PRAW recording function
/* Note:
 * When `folder` path specified is a relative path, then it is relative to PhoXiControl working directory:
 * Windows: 'c:\Users\{user name}\AppData\Roaming\PhotoneoPhoXiControl'
 * Linux: '~/.PhotoneoPhoXiControl'
 */
bool enableDiagnosticsPraw(pho::api::PPhoXi PhoXiDevice, int profile) {
    // Setup PRAW recording with enabled diagnostics profile
    // profiles:
    // 0: 3D Scan (PRAW, ~90MB)
    // 1: 3D Scan With Diagnostic Data(PRAW, ~140MB)
    // 2: 3D Scan With Full Diagnostic Data(PRAW, ~200MB)
    // 3: 2D Outputs(~1OMB)
    // 4: 2D Outputs With Diagnostic Data(~20MB)
    std::string prawRecordingOptions = R"json({
        "folder": "RecordingExampleDiagnosticsOutput",
        "every": 1,
        "capacity": -1,
        "pattern": "my_diag_scan_####",
        "overwrite_existing": true,
        "containers": {
            "praw": {
                "enabled": true,
                "compression": true,
                "diagnostic_profile": %profile%,
                "enable_diagnostic_data": true
            }
        }
    })json";

    prawRecordingOptions =
        std::regex_replace(prawRecordingOptions, std::regex(R"(%profile%)"), std::to_string(profile));

    // Start recording with setup json options for PRAW container, do not store options persistently
    pho::api::PhoXi::StartRecordingResult ret = PhoXiDevice->StartRecording(prawRecordingOptions, false);
    if (ret != pho::api::PhoXi::StartRecordingResult::Success) {
        std::cout << "Failed to start recording for PRAW container! Error: " << static_cast<int>(ret) << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cout << "Starting PhoXi API example for demonstrating recording functionality" << std::endl;
        std::cout << "Usage: RecordingExample.exe --diagnostic <PROFILE_NUM> " << std::endl;
        std::cout << "--diagnostic is optional" << std::endl;
    }

    int diagnosticesProfile = -1;
    if (argc == 3) {
        std::string diagArg = argv[1];
        if (diagArg == "--diagnostic") {
            diagnosticesProfile = std::stoi(argv[2]);
            if (diagnosticesProfile < 0 || diagnosticesProfile > 4) {
                std::cout << "Invalid diagnostic profile! Valid values are 0, 1, 2, 3, 4" << std::endl;
                return 1;
            }
            std::cout << "Diagnostic PRAW recording enabled with profile: " << diagnosticesProfile << std::endl;
        }
    }

    pho::api::PhoXiFactory Factory;

    //Check if the PhoXi Control Software is running
    if (!Factory.isPhoXiControlRunning())
    {
        std::cout << "PhoXi Control Software is not running" << std::endl;
        return 1;
    }

    //Get List of available devices on the network
    std::vector<pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
    if (DeviceList.empty())
    {
        std::cout << "PhoXi Factory has found 0 devices" << std::endl;
        return 1;
    }

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
        return 1;
    }

    //Check if device is connected
    if (!PhoXiDevice->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    if (PhoXiDevice->IsRecording()) {
        //Recording may be started by default when connected to device, let's stop it
        PhoXiDevice->StopRecording();
    }

    //Read last stored recording options for the device
    auto recordingOptions = PhoXiDevice->RecordingOptions();
    std::cout << "Current recording options:\n" << recordingOptions << std::endl;

    if (diagnosticesProfile >= 0) {
        //Enable diagnostic PRAW recording
        if (!enableDiagnosticsPraw(PhoXiDevice, diagnosticesProfile)) {
            return 1;
        }
    } else {
        //Enable PLY recording
        if (!enablePlyFile(PhoXiDevice)) {
            return 1;
        }
    }

    //Get changed recording options
    auto changedRecordingOptions = PhoXiDevice->RecordingOptions();
    std::cout << "Changed recording options:\n" << changedRecordingOptions << std::endl;

    PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
    if(!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
        std::cout << "Failed to set trigger mode to Software!";
        return 1;
    }

    if(!PhoXiDevice->isAcquiring()) {
        if(!PhoXiDevice->StartAcquisition()) {
            std::cout << "Failed to start acquisition!";
            return 1;
        }
    }

    auto frameId = PhoXiDevice->TriggerFrame(true, true);
    if(frameId < 0) {
        std::cout << "Failed to trigger frame!" << std::endl;
        return 1;
    }

    //Wait for recorder to finish recording all wanted frames before stopping recording.
    //This function will return frame index even for frames which were not actually recorded
    //due to `every` (every n-th) skipping.
    while(frameId != PhoXiDevice->LastRecordedFrameIndex()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!PhoXiDevice->StopRecording()) {
        std::cout << "Failed to stop recording!" << std::endl;
        return 1;
    }

    PhoXiDevice->StopAcquisition();
    //Disconnect and keep PhoXiControl connected
    PhoXiDevice->Disconnect();
    return 0;
}

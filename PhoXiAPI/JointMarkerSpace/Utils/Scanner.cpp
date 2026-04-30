#include "Scanner.h"
#include "Util.h"

#include <iostream>
#include <thread>

namespace utils {

std::string selectAvailableDevice(pho::api::PhoXiFactory& factory, const std::string &typeOfDevice) {
    auto deviceList = factory.GetDeviceList();

    std::cout << "PhoXi Factory found " << deviceList.size()
            << " devices by GetDeviceList call." << std::endl
            << std::endl;
    int i = 0;
    for (const auto& device : deviceList) {
        std::cout << "Device: " << i << std::endl;
        std::cout << "  Name:                    " << device.Name << std::endl;
        std::cout << "  Hardware Identification: " << device.HWIdentification << std::endl;
        std::cout << "  Type:                    " << std::string(device.Type) << std::endl;
        std::cout << "  Firmware version:        " << device.FirmwareVersion << std::endl;
        std::cout << "  Status:                  "
                << (device.Status.Attached ?
                        "Attached to PhoXi Control. " :
                        "Not Attached to PhoXi Control. ")
                << (device.Status.Ready ? "Ready to connect" : "Occupied")
                << std::endl
                << std::endl;

        ++i;
    }

    const std::string devicePrefix = typeOfDevice.empty() ? std::string() : (typeOfDevice + " ");

    while (true) {
        std::cout << std::endl
                << "Please enter the index of the " << devicePrefix << "device to connect: ";
        std::size_t index;
        if (std::cin >> index) {
            if (index < deviceList.size()) {
                return std::string(deviceList[index]);
            }
        }
        std::cout << "Bad Index, or not number!" << std::endl;
    }

}

pho::api::PPhoXi connectDevice(pho::api::PhoXiFactory& factory, const std::string& type) {
    auto device = factory.Create(type);

    if (device) {
        if (device->Connect()) {
            std::cout << "Connection to the device " << type
                    << " was Successful!" << std::endl;
        } else {
            std::cout << "Connection to the device " << type
                    << " was Unsuccessful!" << std::endl;
            throw std::runtime_error("Failed to connect");
        }
    } 
    else 
    {
        throw std::runtime_error("Failed to create device");
    }

    return device;
}

pho::api::PPhoXi selectAndConnectDevice(
        pho::api::PhoXiFactory& factory, const std::string &typeOfDevice) {
    auto type = selectAvailableDevice(factory, typeOfDevice);
    return connectDevice(factory, type);
}

void disconnectOrLogOut(pho::api::PPhoXi device) {
    if (utils::ask<int>("Do you want to also log out the device " + device->Info().Name +
            " out of PhoXiControl when disconnecting?", {
        {0, "No"},
        {1, "Yes"}})
    ) {
        device->Disconnect(true);
    } else {
        device->Disconnect();
    }
}

void ensureSoftwareTriggerMode(pho::api::PPhoXi& PhoXiDevice) {
    if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Software) {
        std::cout << "Device is not in Software trigger mode" << std::endl;

        if (PhoXiDevice->isAcquiring()) {
            std::cout << "Stopping acquisition" << std::endl;
            // If the device is in Acquisition mode,
            // we need to stop the acquisition
            if (!PhoXiDevice->StopAcquisition()) {
                throw std::runtime_error("Error in StopAcquistion");
            }
        }

        std::cout << "Switching to Software trigger mode " << std::endl;
        // Switching the mode is as easy as assigning of a value,
        // it will call the appropriate calls in the background
        PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;

        // Just check if everything went smoothly
        if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
            throw std::runtime_error(
                PhoXiDevice->TriggerMode.GetLastErrorMessage().c_str());
        }
    }
}

pho::api::PFrame triggerScanAndGetFrame(pho::api::PPhoXi device) {
    // Check if the device is connected
    if (!device || !device->isConnected())
        throw std::runtime_error("Device not connected");

    // If it is not in Software trigger mode, we need to switch the modes
    ensureSoftwareTriggerMode(device);

    // Start the device acquisition, if necessary
    if (!device->isAcquiring()) {
        if (!device->StartAcquisition()) {
            throw std::runtime_error("Error in StartAcquisition");
        }
    }

    // We can clear the current Acquisition buffer -- This will not clear
    // Frames that arrive to the PC after the Clear command is performed
    auto clearedFrames = device->ClearBuffer();
    std::cout << "Dropped " << clearedFrames
            << " frames in acquisition buffer" << std::endl;

    // While we checked the state of the StartAcquisition call,
    // this check is not necessary, but it is a good practice
    if (!device->isAcquiring())
        throw std::runtime_error("Scanner is not acquiring");

    std::cout << "Triggering a scan..." << std::endl;
    const auto frameID = device->TriggerFrame(
            /*WaitForAccept*/true,
            /*WaitForGrabbingEnd*/true);

    if (frameID < 0) {
        // If negative number is returned trigger was unsuccessful
        throw std::runtime_error("Trigger was unsuccessful!");
    }

    std::cout << "Scan was triggered, Frame Id: " << frameID << std::endl;

    // Wait for a frame with specific FrameID. There is a possibility, that
    // frame triggered before the trigger will arrive after the trigger
    // call, and will be retrieved before requested frame
    // Because of this, the TriggerFrame call returns the requested frame
    // ID, so it can than be retrieved from the Frame structure. This call
    // is doing that internally in background
    // You can specify Timeout here - default is the Timeout stored in Timeout
    // Feature -> Infinity by default
    const auto frame = device->GetSpecificFrame(frameID);

    if (!frame || !frame->Successful) {
        throw std::runtime_error("Failed to retrieve frame");
    }

    std::cout << "Frame successfully retrieved." << std::endl;

    if (frame->Empty()) {
        throw std::runtime_error("Frame is empty");
    }

    if (frame->Texture.Empty()) {
        throw std::runtime_error("Frame Texture is empty");
    }

    return frame;
}

}   // namespace utils

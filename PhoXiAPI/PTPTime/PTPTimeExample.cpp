/*
 * Photoneo's API Example - PTPTime.cpp
 * pho::api::Frame::FrameStartTime processing
 */

#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <set>

#include "PhoXi.h"

const int FRAMES_TO_TRIGGER = 10;

struct Result {
    pho::api::PPhoXi PhoXiDevice;
    std::vector<pho::api::PFrame> frames;
};

void triggerDevice(const pho::api::PPhoXi& PhoXiDevice,
                   std::vector<Result> &results,
                   std::mutex& mtx) {
    Result result;

    result.PhoXiDevice = PhoXiDevice;

    for (int i = 0; i < FRAMES_TO_TRIGGER; ++i) {
        int frameIndex = PhoXiDevice->TriggerFrame();
        if (frameIndex < 0) {
            std::cout << PhoXiDevice->HardwareIdentification.GetValue() << ": Failed to trigger frame!" << std::endl;
            return;
        }

        pho::api::PFrame frame = PhoXiDevice->GetSpecificFrame(frameIndex);
        if (!frame) {
            std::cout << PhoXiDevice->HardwareIdentification.GetValue() << ": Failed to getting frame!" << std::endl;
            return;
        }

        result.frames.push_back(frame);
    }

    PhoXiDevice->StopAcquisition();
    // Disconnect and keep PhoXiControl connected
    PhoXiDevice->Disconnect();

    // Protect access to the results vector
    {
        std::lock_guard<std::mutex> lock(mtx);
        results.push_back(result);
    }
}

void triggerParallel(const std::vector<pho::api::PPhoXi> &PhoXiDevices, std::vector<Result> &Results) {
    std::vector<std::thread> Threads;
    std::mutex Mutex;

    std::cout << "Collecting " << FRAMES_TO_TRIGGER << " frames from connected devices" << std::endl << std::endl;

    for (auto& PhoXiDevice : PhoXiDevices) {
        Threads.push_back(std::thread(triggerDevice,
                                      PhoXiDevice,
                                      std::ref(Results),
                                      std::ref(Mutex)));
    }

    for (auto& thread : Threads) {
        thread.join();
    }
}

void collectAttachedDevices(std::vector<pho::api::PPhoXi> &PhoXiDevices) {
    pho::api::PhoXiFactory Factory;

    std::cout << "Trying to connect to all opened devices in PhoXiControl" << std::endl;

    // Check if the PhoXi Control Software is running
    if (!Factory.isPhoXiControlRunning()) {
        std::cout << "PhoXi Control Software is not running" << std::endl;
        return;
    }

    // Get List of available PhoXiDevices on the network
    std::vector<pho::api::PhoXiDeviceInformation> DeviceList = Factory.GetDeviceList();
    if (DeviceList.empty()) {
        std::cout << "PhoXi Factory has found 0 PhoXiDevices" << std::endl;
        return;
    }

    for (const auto &PhoXiDeviceInList : DeviceList) {
        if (! PhoXiDeviceInList.Status.Attached) {
            continue;
        }

        auto PhoXiDevice = Factory.CreateAndConnect(PhoXiDeviceInList.HWIdentification);


        // Check if device was created
        if (!PhoXiDevice) {
            std::cout << PhoXiDeviceInList.Name << " was not created!" << std::endl;
            continue;
        }

        if (PhoXiDevice->isAcquiring()) {
            if (!PhoXiDevice->StopAcquisition()) {
                std::cout << "Failed to stop acquisition!" << std::endl;
                continue;
            }
        }

        PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
        if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
            std::cout << "Failed to set trigger mode to Software!" << std::endl;
            continue;
        }

        if (!PhoXiDevice->isAcquiring()) {
            if (!PhoXiDevice->StartAcquisition()) {
                std::cout << "Failed to start acquisition!" << std::endl;
                continue;
            }
        }

        PhoXiDevices.push_back(PhoXiDevice);
    }

    std::cout << std::endl;
}

bool checkPtp(const std::vector<Result> &Results) {
    bool portStateIsOK = true;
    bool ptpTimeIsValid = true;
    bool grandMasterIdentityIsSame = true;

    std::set<std::string> grandMasters;

    for (const auto &result : Results) {
        for (const auto &frame : result.frames ) {
            const pho::api::PhoXiPTPTime &ptpTime = frame->Info.FrameStartTime;

            if (!ptpTime.IsValid()) {
                ptpTimeIsValid = false;
            }

            if (ptpTime.PortState != "MASTER" &&
                ptpTime.PortState != "SLAVE") {
                portStateIsOK = false;
            }

            grandMasters.insert(ptpTime.GrandMasterIdentity);
        }

    }

    if (grandMasters.size() != 1) {
        grandMasterIdentityIsSame = false;
    }

    if (ptpTimeIsValid &&
        portStateIsOK &&
        grandMasterIdentityIsSame) {
        std::cout << "All PTP timestamps are valid, in correct state, and have the same grandmaster identity (" << *grandMasters.begin() << ")." << std::endl
                  << "Timestamps can be compared with each other" << std::endl;
    } else {
        std::cout << "Not all timestamps can be compared with each other." << std::endl;
        if (!ptpTimeIsValid) {
            std::cout << "Not all timestamps are valid." << std::endl;
        }
        if (!portStateIsOK) {
            std::cout << "Not all port states are in the correct state." << std::endl;
        }
        if (!grandMasterIdentityIsSame) {
            std::cout << "Not all grandmaster identify values are the same." << std::endl;
        }
    }

    std::cout << std::endl;

    return grandMasterIdentityIsSame;
}

void dumpFramePtpInfo(const std::vector<Result> &Results) {
    std::cout << "Dump frame start acquisition PTP time:" << std::endl << std::endl;

    for (const auto &result : Results) {

        std::cout << "Device: " << result.PhoXiDevice->HardwareIdentification.GetValue() << std::endl;

        for (const auto &frame : result.frames ) {
            pho::api::PhoXiPTPTime ptpTime = frame->Info.FrameStartTime;

            if (!ptpTime.IsValid()) {
                std::cout << "Frame doesn't contains valid start acquisition time!" << std::endl;
                continue;
            }

            auto subSeconds = ptpTime.Time.time_since_epoch().count() % pho::api::PhoXiPTPTime::Clock::duration::period::den;

            std::cout << "Frame start acquisition time is ";
            std::cout << ptpTime.TimeAsString("%Y-%m-%d %H:%M:%S") << "." << std::setw(9) << std::setfill('0') << subSeconds << ", ";
            std::cout << "PTP port state is " << ptpTime.PortState << ", ";
            std::cout << "PTP grandmaster identity is " << ptpTime.GrandMasterIdentity;
            std::cout << std::endl;
        }

        std::cout << std::endl;

    }
}

// calculate: delta = lhs(time) - rhs(time)
template <typename T = std::chrono::milliseconds>
T calcDelta(const pho::api::PhoXiPTPTime &lhs, const pho::api::PhoXiPTPTime &rhs) {
    if (lhs.IsValid() && rhs.IsValid() && lhs.GrandMasterIdentity == rhs.GrandMasterIdentity) {
        T delta = std::chrono::duration_cast<T>(lhs.Time - rhs.Time);
        return delta;
    } else {
        return T(-1);
    }
}

void dumpDelays(const std::vector<Result> &Results) {
    std::cout << "Delay table of the same frames but from different devices:" << std::endl << std::endl;

    struct FramePTPGroups {
        std::vector<pho::api::PhoXiPTPTime> times;
    };
    std::vector<FramePTPGroups> groups;

    // reorder PTP times
    for (int r = 0; r < Results.size(); r++) {
        if (groups.empty()) {
            groups.resize(Results[r].frames.size());
        }
        if (groups.size() != Results[r].frames.size()) {
            std::cout << "The devices did not trigger the same number of frames" << std::endl;
            return;
        }
        for (int f = 0; f < Results[r].frames.size(); f++) {
            pho::api::PhoXiPTPTime ptpTime = Results[r].frames[f]->Info.FrameStartTime;
            groups[f].times.push_back(ptpTime);
        }
    }

    std::cout << "Frame x | ";
    for (const auto &result : Results) {
        std::cout << result.PhoXiDevice->HardwareIdentification.GetValue() << " | ";
    }
    std::cout << std::endl;

    int frameIndex = 0;
    for (const FramePTPGroups& group : groups) {
        // find the lowest timestamp
        pho::api::PhoXiPTPTime zeroTime = group.times[0];
        for (int d = 1; d < group.times.size(); d++) {

            if (calcDelta(zeroTime, group.times[d]).count() > 0) {
                zeroTime = group.times[d];
            }

        }

        // calc and dump final deltas
        std::cout << "Frame " << frameIndex++ << " | ";
        for (const auto &t : group.times) {
            std::cout << calcDelta(t, zeroTime).count() << "ms | ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
    std::vector<pho::api::PPhoXi> PhoXiDevices;
    std::vector<Result> Results;

    collectAttachedDevices(PhoXiDevices);

    if (PhoXiDevices.size() < 2) {
        std::cout << "The example program needs at least two devices connected to the PhoXi Control."
                  << std::endl;
        return 0;
    }

    triggerParallel(PhoXiDevices, Results);

    bool grandMasterIdentityIsSame = checkPtp(Results);

    dumpFramePtpInfo(Results);

    if (grandMasterIdentityIsSame) {
        dumpDelays(Results);
    }

    return 0;
}

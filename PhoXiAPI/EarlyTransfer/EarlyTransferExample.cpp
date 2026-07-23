/*
* Photoneo's API Example - EarlyTransferExample.cpp
*
* Demonstrates the use of the Early Transfer feature that delivers the
* ColorCameraImage as a separate frame as soon as possible.
*
* Usage: EarlyTransferExample [-X] [-a] [-i <iterations>] [-e <colorExposure>] [-r <randomDelayBeforeTriggerMillis>]
 * -a                 : Run the example with asynchronous frame grabbing (default is synchronous)
 * -X                 : Don't use Early Transfer. Helpful to compare how Early Transfer affects the timing of the
 *                      final full frame
 * -i <iterations>    : Number of frames to grab (default is 1)
 * -e <colorExposure> : Set the exposure of color camera (in ms) if present (default is to keep what is currently set)
 * -r <randomDelayBeforeTriggerMillis> : Wait for a random time before each trigger for up to this number of
 *                                       milliseconds (default is none)
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "PhoXi.h"

// Helper class for version numbers manipulation and comparison
struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
    bool parse(const std::string& s) {
        return std::sscanf(s.c_str(), "%d.%d.%d", &major, &minor, &patch) == 3;
    }
    bool operator>=(const Version& other) const {
        if (major > other.major) return true;
        if (major < other.major) return false;
        if (minor > other.minor) return true;
        if (minor < other.minor) return false;
        return patch >= other.patch;
    }
    bool operator<(const Version& other) const {
        return !(*this >= other); }
};

// Commandline options that affect behaviour of this example
struct Options {
    /// Rung the asynchronous grabbing example.
    bool async = false;
    /// Number of frames to grab in one run.
    int iterations = 1;
    /// Set the exposure of color camera (in ms) if present
    /// (otherwise keep what is currently set).
    std::optional<float> colorExposure = {};
    /// Wait for a random time before each trigger for up to
    /// this number of milliseconds.
    std::optional<int> randomDelayBeforeTriggerMillis = {};

    /// Whether to actually use early transfer. Setting this
    /// to false is helpful when comparing timing to "normal"
    /// operation.
    bool earlyTransfer = true;

    /// Parse these options from the given commandline.
    void parse(int argc, char *argv[]);

};

std::ostream& operator<<(std::ostream&& os, Options o);

// Setup device for enable Early Transfer feature
void setupDeviceForEarlyTransfer(pho::api::PPhoXi &PhoXiDevice, const Options& options);
// Run software trigger example
void startSoftwareTriggerExample(pho::api::PPhoXi &PhoXiDevice, const Options& options);
// Run software trigger example with asynchronous frame grabbing
void startSoftwareTriggerAsyncGrabExample(pho::api::PPhoXi& PhoXiDevice, const Options& options);
// Print out frame info to standard output
void printFrameInfo(const pho::api::PFrame &Frame);
// Print out frame data to standard output
void printFrameData(const pho::api::PFrame &Frame);

struct Error : public std::runtime_error {
    using runtime_error::runtime_error;
};

int main(int argc, char *argv[])
{
    try {
        // Parse command line options
        Options options;
        options.parse(argc, argv);

        pho::api::PhoXiFactory Factory;

        // Check if the PhoXi Control Software is running
        if (!Factory.isPhoXiControlRunning())
        {
            std::cout << "PhoXi Control Software is not running" << std::endl;
            return 0;
        }

        // Check if the device supports Early transfer feature
        Version phoXiControlVersion, recomendedVersion{1, 17, 0};
        phoXiControlVersion.parse(Factory.GetPhoXiControlVersion());

        if (phoXiControlVersion < recomendedVersion) {
            std::cout << "Your PhoXi Control version " << Factory.GetPhoXiControlVersion()
                      << "\ndoes not support Early Transfer feature,"
                      << "\nplease update your PhoXi Control application to 1.17.0 or newer" << std::endl;
            return 0;
        }

        // Try to connect device opened in PhoXi Control, if any
        pho::api::PPhoXi PhoXiDevice = Factory.CreateAndConnectFirstAttached();
        if (PhoXiDevice)
        {
            std::cout << "You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: "
                << (std::string) PhoXiDevice->HardwareIdentification << std::endl;
        }

        // Check if device was created
        if (!PhoXiDevice)
        {
            std::cout << "Your device was not created! Connect the device to the PhoXi Control." << std::endl;
            return 0;
        }

        // Check if device is connected
        if (!PhoXiDevice->isConnected())
        {
            std::cout << "Your device is not connected" << std::endl;
            return 0;
        }

        // Check if the device supports Early transfer feature
        Version fwVersion;
        fwVersion.parse(PhoXiDevice->Info().FirmwareVersion);

        if (fwVersion < recomendedVersion)
        {
            std::cout << "Your device with firmware version " << PhoXiDevice->Info().FirmwareVersion
                      << "\ndoes not support Early Transfer feature,"
                      << "\nplease update your device firmware to 1.17.0 or newer" << std::endl;
            return 0;
        }

        if(false && !PhoXiDevice->Info().CheckFeature("Color"))
        {
            std::cout << "Your device does not support Color feature" << std::endl;
            return 0;
        }

        // Setup device for receiving early transfer frames
        setupDeviceForEarlyTransfer(PhoXiDevice, options);

        if (options.async) {
            // Launch software trigger example with asynchronous frame grabber
            startSoftwareTriggerAsyncGrabExample(PhoXiDevice, options);
        } else {
            // Launch software trigger example
            startSoftwareTriggerExample(PhoXiDevice, options);
        }

        PhoXiDevice->Disconnect();
    }
    catch (Error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

void Options::parse(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        using namespace std::literals;
        auto opt = std::string_view(argv[i]);

        auto getIntArgument = [&]() {
            if (i + 1 < argc) {
                return atoi(argv[++i]);
            } else {
                throw Error("Option '" + std::string{opt} + "' requires an argument");
            }
        };
        auto getFloatArgument = [&]() {
            if (i + 1 < argc) {
                return static_cast<float>(atof(argv[++i]));
            } else {
                throw Error("Option '" + std::string{opt} + "' requires an argument");
            }
        };

        if (opt == "-a") {
            async = true;
        }
        else if (opt == "-X") {
            earlyTransfer = false;
        }
        else if (opt == "-i") {
            iterations = getIntArgument();
        }
        else if (opt == "-e") {
            colorExposure = getFloatArgument();
        }
        else if (opt == "-r") {
            randomDelayBeforeTriggerMillis = getIntArgument();
        } else {
            throw Error("Uknown option + std::string{opt}");
        }
    }
}

std::ostream& operator<<(std::ostream&& os, Options o) {
    os << "early transfer: " << (o.earlyTransfer ? "enabled" : "disabled") << "\n";
    os << "async: " << o.async << "\n";
    os << "iterations: " << o.iterations << "\n";
    os << "colorExposure: ";
    if (o.colorExposure) {
        os << *o.colorExposure << "\n";
    } else {
        os << "not given\n";
    }
    return os;
}

// Simple timer class for measuring elapsed time
struct Timer {
    using millis_double = std::chrono::duration<double, std::milli>;
    using clock = std::chrono::steady_clock;
    clock::time_point startedAt;

    Timer()
    {
        reset();
    }
    void reset()
    {
        startedAt = clock::now();
    }

    clock::duration elapsed() const
    {
        return clock::now() - startedAt;
    }

    millis_double elapsedMillis() const
    {
        return clock::now() - startedAt;
    }
};

std::ostream& operator<<(std::ostream& os, const Timer::millis_double& m)
{
    std::ios::fmtflags f(std::cout.flags());
    os << std::fixed << std::setprecision(3) << m.count() << " ms";
    std::cout.flags(f);
    return os;
}

void setupDeviceForEarlyTransfer(pho::api::PPhoXi& PhoXiDevice, const Options& options)
{
    auto earlyTransfer = options.earlyTransfer
        ? pho::api::PhoXiEarlyTransfer::ColorCameraImage
        : pho::api::PhoXiEarlyTransfer::Off;
    if (PhoXiDevice->GetType() == pho::api::PhoXiDeviceType::PhoXiScanner)
        PhoXiDevice->CapturingSettings->EarlyTransfer = earlyTransfer;
    else
        PhoXiDevice->MotionCam->EarlyTransfer = earlyTransfer;

    PhoXiDevice->OutputSettings->SendColorCameraImage = true;
    PhoXiDevice->CoordinatesSettings->CameraSpace = pho::api::PhoXiCameraSpace::ColorCamera;

    if (options.colorExposure) {
        PhoXiDevice->ColorSettings->Exposure = *options.colorExposure;
    }

    std::cout << "Early Transfer feature was "
        << (options.earlyTransfer ? "enabled" : "disabled")
        << std::endl;

    std::cout << "Color Exposure: " << PhoXiDevice->ColorSettings->Exposure << "\n";
    std::cout << "Iterations: " << options.iterations << "\n";
    std::cout << "Random delay before trigger: ";
    if (options.randomDelayBeforeTriggerMillis) {
        std::cout << "0 - " << *options.randomDelayBeforeTriggerMillis << " ms \n";
    } else {
        std::cout << "none\n";
    }
}

void startSoftwareTriggerExample(pho::api::PPhoXi &PhoXiDevice, const Options& options)
{
    if (PhoXiDevice->isAcquiring())
    {
        // Stop acquisition to change trigger mode
        PhoXiDevice->StopAcquisition();
    }

    PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
    std::cout << "Software trigger mode was set" << std::endl;

    PhoXiDevice->StartAcquisition();
    if (!PhoXiDevice->isAcquiring())
    {
        std::cout << "Your device could not start acquisition!" << std::endl;
        return;
    }

    Timer timer;

    for (int i = 0; i < options.iterations; ++i)
    {
        std::cout << "\n\n";
        if (options.randomDelayBeforeTriggerMillis) {
            auto delay = std::chrono::milliseconds(rand() % *options.randomDelayBeforeTriggerMillis);
            std::cout << "Waiting for " << delay.count() << " ms before triggering\n" << std::endl;
            std::this_thread::sleep_for(delay);
        }

        timer.reset();
        std::cout << timer.elapsedMillis() << " Triggering frame " << i << std::endl;
        int FrameID = PhoXiDevice->TriggerFrame(true);
        if (FrameID < 0)
        {
            // If negative number is returned trigger was unsuccessful
            std::cout << timer.elapsedMillis() << " Trigger was unsuccessful! code=" << FrameID << std::endl;
            continue;
        }
        else
        {
            std::cout << timer.elapsedMillis() << " Frame was triggered, Frame Id: " << FrameID << std::endl;
        }

        auto expectedFrames = options.earlyTransfer ? std::vector{"color", "full"} : std::vector{"full"};
        for (const auto& pass : expectedFrames) {
            std::cout <<"\n";
            std::cout << timer.elapsedMillis() << " Getting " << pass << " frame\n";
            pho::api::PFrame Frame = PhoXiDevice->GetSpecificFrame(FrameID++, pho::api::PhoXiTimeout::Infinity);

            if (Frame)
            {
                std::cout << timer.elapsedMillis() << " Got " << pass << " frame " << Frame->Info.FrameIndex << std::endl;
                printFrameInfo(Frame);
                printFrameData(Frame);
            }
            else
            {
                std::cout << timer.elapsedMillis() << " Failed to retrieve the frame!" << std::endl;
            }
        }
    }
    PhoXiDevice->StopAcquisition();
}

void startSoftwareTriggerAsyncGrabExample(pho::api::PPhoXi& PhoXiDevice, const Options& options)
{
    std::atomic<uint64_t> AsyncFrameID;
    Timer timer;

    // This callback will be called when new frame is arrived
    auto AsyncGetFrameCallback = [&AsyncFrameID, &timer](pho::api::PFrame Frame) {
        if (Frame) {
            std::cout << "\n";
            std::cout << timer.elapsedMillis() << " New frame arrived\n";
            printFrameInfo(Frame);
            printFrameData(Frame);
            AsyncFrameID = Frame->Info.FrameIndex;
        }
        else
        {
            std::cout << timer.elapsedMillis() << " Failed to retrieve the frame!" << std::endl;
        }
    };

    if (PhoXiDevice->isAcquiring())
    {
        // Stop acquisition to change trigger mode
        PhoXiDevice->StopAcquisition();
    }

    PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;
    std::cout << "Software trigger mode was set" << std::endl;
    PhoXiDevice->ClearBuffer();
    // Enable exclusive asynchronous frame grabbing mode with user defined notification callback
    PhoXiDevice->EnableAsyncGetFrame(std::move(AsyncGetFrameCallback));
    PhoXiDevice->StartAcquisition();
    if (!PhoXiDevice->isAcquiring())
    {
        std::cout << "Your device could not start acquisition!" << std::endl;
        return;
    }

    timer.reset();
    int finalFrameID = -1;

    for (int i = 0; i < options.iterations; ++i)
    {
        std::cout << timer.elapsedMillis() << " Triggering the " << i << "-th frame" << std::endl;
        int FrameID = PhoXiDevice->TriggerFrame();
        if (FrameID < 0)
        {
            // If negative number is returned trigger was unsuccessful
            std::cout << timer.elapsedMillis() << " Trigger was unsuccessful! code=" << FrameID << std::endl;
            continue;
        }
        else
        {
            std::cout << timer.elapsedMillis() << " Frame was triggered, Frame Id: " << FrameID << std::endl;
            finalFrameID = FrameID;
        }
    }

    if (finalFrameID < 0) {
        std::cout << "No frames were successfully triggered.\n";
        return;
    }

    // With Early Transfer feature, SW trigger currently returns the frame ID of the first
    // (rgb only) frame, so the final frame is FrameID + 1
    if (options.earlyTransfer)
        finalFrameID += 1;

    // Wait for last frame catched in callback
    while (static_cast<uint64_t>(finalFrameID) != AsyncFrameID) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    PhoXiDevice->StopAcquisition();
    // Disable asynchronous frame grabbing and switch back to synchronous mode
    PhoXiDevice->DisableAsyncGetFrame();
}

void printFrameInfo(const pho::api::PFrame &Frame)
{
    const pho::api::FrameInfo &FrameInfo = Frame->Info;
    std::cout << "  Frame params: " << std::endl;
    std::cout << "    Frame Index: "                << FrameInfo.FrameIndex << std::endl;
    std::cout << "    Frame EarlyTransfer flag: "   << (FrameInfo.IsEarlyTransferFrame ? "YES" : "NO") << std::endl;
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
    std::cout << "    FilenamePath: " << FrameInfo.FilenamePath << std::endl;
    std::cout << "    HWIdentification: " << FrameInfo.HWIdentification << std::endl;
    std::cout << "    Marker dot status: " << FrameInfo.MarkerDots.Status << std::endl;
}

//
// Helpers to make a quick hash/checksum of the data to see when it changes...
//

template<typename It>
size_t hash_range(It begin, It end);

template<>
struct std::hash<pho::api::ColorRGB_16> {
    std::size_t operator()(const pho::api::ColorRGB_16& t) {
        const std::vector<size_t> hashes = {
            std::hash<uint16_t>{}(t.r),
            std::hash<uint16_t>{}(t.g),
            std::hash<uint16_t>{}(t.b),
        };
        return hash_range(hashes.cbegin(), hashes.cend());
    }
};

template<>
struct std::hash<pho::api::Depth_32f> {
    std::size_t operator()(const pho::api::Depth_32f& d) { return std::hash<float>{}(float{d}); }
};

template<typename T>
size_t hash(const T& t) {
    return std::hash<T>{}(t);
}

template<typename It>
size_t hash_range(It begin, It end) {
    size_t h = 0;
    // only "sample" the image to make it fast
    // TODO not really end iterator ;)
    size_t step = std::min<size_t>(1L, (end - begin) / 100);
    for (auto i = begin; i < end; i += 2000) {
        h ^= hash(*i) + 0x9e3779b9 + (h<<6) + (h>>2);
    }
    return h;
}

template<typename T>
struct std::hash<pho::api::Mat2D<T>> {
    std::size_t operator()(const pho::api::Mat2D<T>& t) {
        const auto* begin = reinterpret_cast<const typename T::ElementChannelType*>(t.GetDataPtr());
        const auto* end = begin + T::ElementChannelCount * t.GetElementsCount();
        return hash_range(begin, end);
    }
};

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
        std::cout << "    PointCloud:       ("
            << Frame->PointCloud.Size.Width << " x "
            << Frame->PointCloud.Size.Height << ") Type: "
            << Frame->PointCloud.GetElementName()
            << " hash " << std::hex << hash(Frame->PointCloud) << std::dec
            << std::endl;
    }
    if (!Frame->NormalMap.Empty())
    {
        std::cout << "    NormalMap:        ("
            << Frame->NormalMap.Size.Width << " x "
            << Frame->NormalMap.Size.Height << ") Type: "
            << Frame->NormalMap.GetElementName()
            << " hash " << std::hex << hash(Frame->NormalMap) << std::dec
            << std::endl;
    }
    if (!Frame->DepthMap.Empty())
    {
        std::cout << "    DepthMap:         ("
            << Frame->DepthMap.Size.Width << " x "
            << Frame->DepthMap.Size.Height << ") Type: "
            << Frame->DepthMap.GetElementName()
            << " hash " << std::hex << hash(Frame->DepthMap) << std::dec
            << std::endl;
    }
    if (!Frame->ConfidenceMap.Empty())
    {
        std::cout << "    ConfidenceMap:    ("
            << Frame->ConfidenceMap.Size.Width << " x "
            << Frame->ConfidenceMap.Size.Height << ") Type: "
            << Frame->ConfidenceMap.GetElementName()
            << " hash " << std::hex << hash(Frame->ConfidenceMap) << std::dec
            << std::endl;
    }
    if (!Frame->Texture.Empty())
    {
        std::cout << "    Texture:          ("
            << Frame->Texture.Size.Width << " x "
            << Frame->Texture.Size.Height << ") Type: "
            << Frame->Texture.GetElementName()
            << " hash " << std::hex << hash(Frame->Texture)<< std::dec
            << std::endl;
    }
    if (!Frame->TextureRGB.Empty())
    {
        std::cout << "    TextureRGB:       ("
            << Frame->TextureRGB.Size.Width << " x "
            << Frame->TextureRGB.Size.Height << ") Type: "
            << Frame->TextureRGB.GetElementName()
            << " hash " << std::hex << hash(Frame->TextureRGB) << std::dec
            << std::endl;
    }
    if (!Frame->ColorCameraImage.Empty())
    {
        std::cout << "    ColorCameraImage: ("
            << Frame->ColorCameraImage.Size.Width << " x "
            << Frame->ColorCameraImage.Size.Height << ") Type: "
            << Frame->ColorCameraImage.GetElementName()
            << " hash " << std::hex << hash(Frame->ColorCameraImage) << std::dec
            << std::endl;
    }
}

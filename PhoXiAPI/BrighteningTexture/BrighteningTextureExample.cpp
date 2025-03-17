/*
* Photoneo's API Example - BrighteningTextureExample.cpp
* Shows way how to brighten / normalize textures obtainable from the frame using min and max values.
 */

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <functional>

#include "PhoXi.h"

namespace {

//Helper functions
template<typename T>
inline std::array<const T*, 1> makeIterable(const pho::api::MatType<T>& value) {
    return std::array<const T*, 1>{&static_cast<const T&>(value)};
}
template<typename T>
inline std::array<T*, 1> makeIterable(pho::api::MatType<T>& value) {
    return std::array<T*, 1>{&static_cast<T&>(value)};
}

template<typename T>
inline std::array<const T*, 3> makeIterable(const pho::api::ColorRGB<T>& value) {
    return std::array<const T*, 3>{
        &static_cast<const T&>(value.r),
        &static_cast<const T&>(value.g),
        &static_cast<const T&>(value.b)
    };
}
template<typename T>
inline std::array<T*, 3> makeIterable(pho::api::ColorRGB<T>& value) {
    return std::array<T*, 3>{
        &static_cast<T&>(value.r),
        &static_cast<T&>(value.g),
        &static_cast<T&>(value.b)
    };
}

using FilterFunc = std::function<bool(size_t)>;
bool defaultFilter(size_t) {
    return true;
}

}

//Classic full range method to find min and max in the matrix for normalization (doesn't remove outliers)
template<typename UnderlyingT, typename ElementT>
std::pair<UnderlyingT, UnderlyingT> findMinMaxFullRange(const pho::api::Mat2D<ElementT>& mat, const FilterFunc& filter = defaultFilter) {
    if (mat.GetElementsCount() == 0) {
        return {UnderlyingT{}, UnderlyingT{}};
    }

    auto min = std::numeric_limits<UnderlyingT>::max();
    auto max = std::numeric_limits<UnderlyingT>::lowest();
    for (size_t i = 0; i < mat.GetElementsCount(); ++i) {
        if (!filter(i)) {
            continue;
        }
        for (auto* element : makeIterable(mat.GetDataPtr()[i])) {
            const auto& value = static_cast<const UnderlyingT&>(*element);
            if (value < min) {
                min = value;
            }
            if (value > max) {
                max = value;
            }
        }
    }
    return {min, max};
}

//Smart version of algorithm to find min and max values in the matrix for normalization
//Universal, removes outliers, tunable by number of iterations
template<typename UnderlyingT, typename ElementT>
std::pair<UnderlyingT, UnderlyingT> findMinMaxSmartIterative(const pho::api::Mat2D<ElementT>& mat, int iterations, const FilterFunc& filter = defaultFilter) {
    if (mat.GetElementsCount() == 0) {
        return {UnderlyingT{}, UnderlyingT{}};
    }

    double centralSum = 0;
    size_t centralCount = 0;
    for (size_t i = 0; i < mat.GetElementsCount(); ++i) {
        if (!filter(i)) {
            continue;
        }
        for (auto* element : makeIterable(mat.GetDataPtr()[i])) {
            centralSum += static_cast<const UnderlyingT&>(*element);
            ++centralCount;
        }
    }

    const double centralMean = centralSum / static_cast<double>(centralCount);
    double lowerMean = centralMean;
    double upperMean = centralMean;

    while (iterations-- > 0) {
        double lowerSum = 0;
        double upperSum = 0;
        size_t lowerCount = 0;
        size_t upperCount = 0;

        for (size_t i = 0; i < mat.GetElementsCount(); ++i) {
            if (!filter(i)) {
                continue;
            }
            for (auto* element : makeIterable(mat.GetDataPtr()[i])) {
                auto& value = static_cast<const UnderlyingT&>(*element);
                if (value < lowerMean) {
                    lowerSum += value;
                    ++lowerCount;
                } else if (value > upperMean) {
                    upperSum += value;
                    ++upperCount;
                }
            }
        }

        if (lowerCount > 0) {
            lowerMean = lowerSum / static_cast<double>(lowerCount);
        }
        if (upperCount > 0) {
            upperMean = upperSum / static_cast<double>(upperCount);
        }
    }

    return {static_cast<UnderlyingT>(lowerMean), static_cast<UnderlyingT>(upperMean)};
}

//Smart version of algorithm to find min and max values for normalization using buckets
//Better performance for small unsigned integral types, removes outliers, tunable by number of iterations
template<typename UnderlyingT, typename ElementT, std::enable_if_t<std::is_unsigned<UnderlyingT>::value && sizeof(UnderlyingT) <= 2, bool> = true>
std::pair<UnderlyingT, UnderlyingT> findMinMaxSmartBucket(const pho::api::Mat2D<ElementT>& mat, int iterations, const FilterFunc& filter = defaultFilter) {
    if (mat.GetElementsCount() == 0) {
        return {UnderlyingT{}, UnderlyingT{}};
    }

    uint64_t buckets[std::numeric_limits<UnderlyingT>::max() + 1] = {};
    uint64_t centralSum = 0;
    size_t centralCount = 0;
    for (size_t i = 0; i < mat.GetElementsCount(); ++i) {
        if (!filter(i)) {
            continue;
        }
        for (auto* element : makeIterable(mat.GetDataPtr()[i])) {
            auto& value = static_cast<const UnderlyingT&>(*element);
            centralSum += value;
            ++centralCount;
            ++buckets[value];
        }
    }

    const uint64_t centralMean = centralSum / centralCount;
    uint64_t lowerMean = centralMean;
    uint64_t upperMean = centralMean;

    while (iterations-- > 0) {
        uint64_t lowerSum = 0;
        uint64_t upperSum = 0;
        size_t lowerCount = 0;
        size_t upperCount = 0;

        for (size_t i = 0; i < lowerMean; ++i) {
            const auto& bucket = buckets[i];
            lowerSum += bucket * i;
            lowerCount += bucket;
        }

        for (size_t i = std::numeric_limits<UnderlyingT>::max(); i > upperMean; --i) {
            const auto& bucket = buckets[i];
            upperSum += bucket * i;
            upperCount += bucket;
        }

        if (lowerCount > 0) {
            lowerMean = lowerSum / lowerCount;
        }
        if (upperCount > 0) {
            upperMean = upperSum / upperCount;
        }
    }

    return {static_cast<UnderlyingT>(lowerMean), static_cast<UnderlyingT>(upperMean)};
}

//Normalization function with min-max type of normalization
template<typename UnderlyingT, typename ElementT>
void normalize(UnderlyingT min, UnderlyingT max, const pho::api::Mat2D<ElementT>& input, pho::api::Mat2D<ElementT>& output, UnderlyingT beta) {
    if (min >= max || input.GetElementsCount() == 0) {
        return;
    }

    output.Resize(input.Size);
    for (size_t i = 0; i < input.GetElementsCount(); ++i) {
        auto inputElementIterable = makeIterable(input.GetDataPtr()[i]);
        auto outputElementIterable = makeIterable(output.GetDataPtr()[i]);
        for (size_t ei = 0; ei < inputElementIterable.size(); ++ei) {
            auto val = static_cast<double>(*inputElementIterable[ei]);
            val -= min;
            val /= (max - min);
            val = (val < 0.0) ? 0.0 : (val > 1.0) ? 1.0 : val;
            val *= beta;
            *outputElementIterable[ei] = static_cast<UnderlyingT>(val);
        }
    }
}

int main(int /*argc*/, char */*argv*/[])
{
    pho::api::PhoXiFactory factory;

    //Check if the PhoXi Control Software is running
    if (!factory.isPhoXiControlRunning()) {
        std::cout << "PhoXi Control Software is not running" << std::endl;
        return 1;
    }

    //Get List of available devices on the network
    const auto deviceList = factory.GetDeviceList();
    if (deviceList.empty()) {
        std::cout << "PhoXi Factory has found 0 devices" << std::endl;
        return 1;
    }

    //Try to connect device opened in PhoXi Control, if any
    auto device = factory.CreateAndConnectFirstAttached();
    if (device) {
        std::cout << "You have already PhoXi device opened in PhoXi Control, the API Example is connected to device: "
                  << device->HardwareIdentification.GetValue() << std::endl;
    }
    else {
        std::cout << "You have no PhoXi device opened in PhoXi Control, the API Example will try to connect to first device in device list" << std::endl;
        device = factory.CreateAndConnect(deviceList.front().HWIdentification);
    }

    //Check if device was created
    if (!device) {
        std::cout << "Your device was not created!" << std::endl;
        return 1;
    }

    //Check if device is connected
    if (!device->isConnected()) {
        std::cout << "Your device is not connected" << std::endl;
        return 1;
    }

    std::cout << std::endl << std::endl;

    //Setup texture source
    bool isColor = false;
    switch (device->GetType()) {
    case pho::api::PhoXiDeviceType::PhoXiScanner:
        device->CapturingSettings->TextureSource = pho::api::PhoXiTextureSource::LED;
        break;
    case pho::api::PhoXiDeviceType::MotionCam3D:
        device->MotionCam->OperationMode = pho::api::PhoXiOperationMode::Camera;
        isColor = device->Info().CheckFeature("Color");
        device->MotionCamCameraMode->TextureSource = isColor ? pho::api::PhoXiTextureSource::Color : pho::api::PhoXiTextureSource::LED;
        break;
    default:
        std::cout << "Unhandled PhoXiDeviceType!" << std::endl;
        return 1;
    }

    //Set software trigger
    device->TriggerMode = pho::api::PhoXiTriggerMode::Software;
    if(!device->TriggerMode.isLastOperationSuccessful()) {
        std::cout << "Failed to set trigger mode to Software!";
        return 1;
    }

    //Start acquisition if not acquiring already
    if(!device->isAcquiring()) {
        if(!device->StartAcquisition()) {
            std::cout << "Failed to start acquisition!";
            return 1;
        }
    }

    //Enable texture and point cloud in the output settings
    pho::api::FrameOutputSettings outputSettings;
    outputSettings.SendTexture = true;
    outputSettings.SendPointCloud = true;
    device->OutputSettings = outputSettings;

    //Trigger and obtain frame and wait for acquisition to finish
    const auto frameId = device->TriggerFrame(true, true);
    if(frameId < 0) {
        std::cout << "Failed to trigger frame!" << std::endl;
        return 1;
    }

    const auto frame = device->GetSpecificFrame(frameId);

    //For results to be the same as texture shown in PhoXi Control, filter method which filters out
    //invalid points when searching for min and max values has to be used.
    //This function is not mandatory, any or none filtering can be used too.
    auto& pointCloud = frame->PointCloud;
    auto filterInvalidPoints = [&](size_t i) {
        if(i < pointCloud.GetElementsCount()) {
            auto& point = pointCloud.GetDataPtr()[i];
            return (std::fabs(point.z) > std::numeric_limits<float>::epsilon());
        }
        return true;
    };

    //PhoXi Control uses 3 iterations by default for Smart algorithms
    const int DEFAULT_ITERATIONS = 3;

    if (isColor) {
        //Find min and max values from texture
        const auto minMaxFull = findMinMaxFullRange<uint16_t>(frame->TextureRGB, filterInvalidPoints);
        const auto minMaxSmart = findMinMaxSmartBucket<uint16_t>(frame->TextureRGB, DEFAULT_ITERATIONS, filterInvalidPoints);

        std::cout << "Color texture:" << std::endl;
        std::cout << "FullRange min: " << minMaxFull.first << ", max: " << minMaxFull.second << std::endl;
        std::cout << "SmartBucket min: " << minMaxSmart.first << ", max: " << minMaxSmart.second << std::endl;

        //Normalize texture with calculated min and max values. Using beta=255 to get output range 0-255.
        pho::api::TextureRGB16 textureFullRange, textureSmart;
        normalize<uint16_t>(minMaxFull.first, minMaxFull.second, frame->TextureRGB, textureFullRange, 255);
        normalize<uint16_t>(minMaxSmart.first, minMaxSmart.second, frame->TextureRGB, textureSmart, 255);

        //Store / show normalized textures using your preferred way...
    } else {
        //Find min and max values from texture
        const auto minMaxFull = findMinMaxFullRange<pho::api::float32_t>(frame->Texture, filterInvalidPoints);
        const auto minMaxSmart = findMinMaxSmartIterative<pho::api::float32_t>(frame->Texture, DEFAULT_ITERATIONS, filterInvalidPoints);

        std::cout << "Grayscale texture:" << std::endl;
        std::cout << "FullRange min: " << minMaxFull.first << ", max: " << minMaxFull.second << std::endl;
        std::cout << "SmartIterative min: " << minMaxSmart.first << ", max: " << minMaxSmart.second << std::endl;

        //Normalize texture with calculated min and max values. Using beta=255 to get output range 0-255.
        pho::api::Texture32f textureFullRange, textureSmart;
        normalize<pho::api::float32_t>(minMaxFull.first, minMaxFull.second, frame->Texture, textureFullRange, 255);
        normalize<pho::api::float32_t>(minMaxSmart.first, minMaxSmart.second, frame->Texture, textureSmart, 255);

        //Store / show normalized textures using your preferred way...
    }

    device->StopAcquisition();
    //Disconnect and keep PhoXiControl connected
    device->Disconnect();
    return 0;
}

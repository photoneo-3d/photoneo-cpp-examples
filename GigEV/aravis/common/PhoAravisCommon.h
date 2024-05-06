#ifndef PHOTONEOMAIN_PHOARAVISCOMMON_H
#define PHOTONEOMAIN_PHOARAVISCOMMON_H

#include <arv.h>

#include <memory>
#include <iostream>
#include <vector>

namespace pho {

#pragma pack(push,1)
struct Vec2D {
    float x;
    float y;
};

struct Vec3D {
    float x;
    float y;
    float z;
};

struct NormalsAngles {
    uint8_t x;
    uint8_t y;
};
#pragma pack(pop)

enum TriggerMode { Freerun = 0, SWTrigger = 1 };

enum OutputMat {
    Intensity = 1,
    Range = 4,
    Confidence = 6,
    CoordinateMapA = 10,
    CoordinateMapB = 11,
    Normal = 0xFF00,
    Event = 0xFF01,
    ColorCameraImage = 0xFF02
};

enum StreamOutputFormat {
    ImageData = 1,
    MultipartData = 2,
};

template <typename T> void gobject_destroyer(T* object) {
    g_clear_object(&object);
}

template <typename T> auto create_gobject_unique(T* ptr) {
    return std::unique_ptr<T, void (*)(T*)>(ptr, gobject_destroyer);
}

bool setTriggerMode(ArvCamera* camera, TriggerMode triggerMode) {
    if (!camera) {
        return false;
    }

    GError* error = nullptr;
    arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS, &error);
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    switch(triggerMode) {
        case Freerun: {
            arv_camera_clear_triggers(camera, &error);
            if (error) {
                std::cerr << "Error: " << error->message << std::endl;
                return false;
            }
            break;
        }
        case SWTrigger: {
            arv_camera_set_trigger(camera, "Software", &error);
            if (error) {
                std::cerr << "Error: " << error->message << std::endl;
                return false;
            }
            break;
        }
    }

    return true;
}

bool triggerFrame(ArvCamera* camera) {
    if (!camera) {
        return false;
    }

    GError* error = nullptr;
    arv_camera_software_trigger(camera, &error);
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    return true;
}

bool setOutputMat(ArvCamera* camera, OutputMat outputMat, bool state) {
    if (!camera) {
        return false;
    }

    GError* error = nullptr;
    std::string selectorOption;
    switch (outputMat) {
        case OutputMat::Intensity:
            selectorOption = "Intensity";
            break;
        case OutputMat::Range:
            selectorOption = "Range";
            break;
        case OutputMat::Normal:
            selectorOption = "Normal";
            break;
        case OutputMat::CoordinateMapA:
            selectorOption = "CoordinateMapA";
            break;
        case OutputMat::CoordinateMapB:
            selectorOption = "CoordinateMapB";
            break;
        case OutputMat::Confidence:
            selectorOption = "Confidence";
            break;
        case OutputMat::Event:
            selectorOption = "Event";
            break;
        case OutputMat::ColorCameraImage:
            selectorOption = "ColorCamera";
            break;
        default:
            return false;
    }

    if(!arv_camera_is_enumeration_entry_available(camera, "ComponentSelector", selectorOption.c_str(), &error)) {
        std::cerr << "Warning: Camera does not support enumeration '" << selectorOption
                  << "' for the ComponentSelector feature. Ignoring..." << std::endl;
        return true;
    }

    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    arv_camera_set_string(camera, "ComponentSelector", selectorOption.c_str(), &error);
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    arv_camera_set_boolean(camera, "ComponentEnable", state, &error);
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    return true;
}

bool setStreamOutputFormat(ArvCamera* camera, StreamOutputFormat format) {
    if (!camera) {
        return false;
    }

    GError* error = nullptr;
    switch (format) {
        case StreamOutputFormat::ImageData: {
            arv_camera_gv_set_multipart(camera, false, &error);
            break;
        }
        case StreamOutputFormat::MultipartData: {
            arv_camera_gv_set_multipart(camera, true, &error);
            break;
        }
        default:
            return false;
    }

    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return false;
    }

    return true;
}

} //namespace pho

#endif  // PHOTONEOMAIN_PHOARAVISCOMMON_H

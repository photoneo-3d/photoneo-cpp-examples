#ifndef PHOTONEOMAIN_PHOARAVISCOMMON_H
#define PHOTONEOMAIN_PHOARAVISCOMMON_H

extern "C" {
#include <arv.h>
}

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
    Texture = 1,
    DepthMap = 4,
    ConfidenceMap = 6,
    NormalMap = 0xFF00,
    EventMap = 0xFF01,
    ColorCameraImage = 0xFF02,
    ReprojectionMap = 0xFF03
};

enum StreamOutputFormat {
    ChunkData = 1,
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
        case OutputMat::Texture:
            selectorOption = "Intensity";
            break;
        case OutputMat::DepthMap:
            selectorOption = "Range";
            break;
        case OutputMat::NormalMap:
            selectorOption = "Normal";
            break;
        case OutputMat::ConfidenceMap:
            selectorOption = "Confidence";
            break;
        case OutputMat::EventMap:
            selectorOption = "Event";
            break;
        case OutputMat::ColorCameraImage:
            selectorOption = "ColorCamera";
            break;
        case OutputMat::ReprojectionMap:
            selectorOption = "Reprojection";
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
        case StreamOutputFormat::ChunkData: {
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

bool getReprojectionMatrix(ArvCamera* camera, Vec2D*& reprojectionMap, size_t& reprojectionMapDataSize) {
    GError* error = nullptr;
    setTriggerMode(camera, TriggerMode::Freerun);
    setStreamOutputFormat(camera, StreamOutputFormat::ChunkData);
    setOutputMat(camera, OutputMat::ReprojectionMap, true);

    auto buffer = arv_camera_acquisition(camera, 0, &error);
    if (!ARV_IS_BUFFER(buffer)) {
        std::cerr << "Error: Not a buffer instance!" << std::endl;
        return false;
    }

    if (!arv_buffer_has_chunks(buffer)) {
        std::cerr << "Error: Buffer has no chunk data!" << std::endl;
        return false;
    }

    reprojectionMap = (Vec2D*)arv_buffer_get_chunk_data(buffer, OutputMat::ReprojectionMap, &reprojectionMapDataSize);
    return true;
}

} //namespace pho

#endif  // PHOTONEOMAIN_PHOARAVISCOMMON_H

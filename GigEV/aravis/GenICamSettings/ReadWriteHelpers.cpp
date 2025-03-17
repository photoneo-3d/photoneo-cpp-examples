#include "ReadWriteHelpers.h"

namespace pho {

bool ReadWriteHelper::testCommandFeature(const std::string& nodeName) {
    GError* error = nullptr;

    arv_camera_execute_command(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }
    return true;
}

bool ReadWriteHelper::testBooleanFeature(const std::string& nodeName, const AccessType access, bool val) {
    GError* error = nullptr;

    if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
        std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
        return true;
    }
    if (error) {
        return handleError(error);
    }

    const auto beforeVal = arv_camera_get_boolean(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    if (access == RO) {
        std::cout << "[" << nodeName << "]: " << beforeVal << std::endl;
        return true;
    }

    std::cout << "[" << nodeName << "] BEFORE: " << beforeVal << " (setting to: " << val << ")" << std::endl;
    arv_camera_set_boolean(_camera, nodeName.c_str(), val, &error);
    if (error) {
        return handleError(error);
    }

    const auto afterVal = arv_camera_get_boolean(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "[" << nodeName << "] AFTER: " << afterVal << std::endl;
    return true;
}

bool ReadWriteHelper::testStringFeature(const std::string& nodeName, const AccessType access, const std::string& val) {
    GError* error = nullptr;

    if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
        std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
        return true;
    }
    if (error) {
        return handleError(error);
    }

    const auto beforeVal = arv_camera_get_string(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    if (access == RO) {
        std::cout << "[" << nodeName << "]: " << beforeVal << std::endl;
        return true;
    }

    std::cout << "[" << nodeName << "] BEFORE: " << beforeVal << " (setting to: " << val << ")" << std::endl;
    arv_camera_set_string(_camera, nodeName.c_str(), val.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    const auto afterVal = arv_camera_get_string(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "[" << nodeName << "] AFTER: " << afterVal << std::endl;
    return true;
}

bool ReadWriteHelper::testIntegerFeature(const std::string& nodeName, const AccessType access, gint64 val) {
    GError* error = nullptr;

    if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
        std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
        return true;
    }
    if (error) {
        return handleError(error);
    }

    const auto beforeVal = arv_camera_get_integer(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    if (access == RO) {
        std::cout << "[" << nodeName << "]: " << beforeVal << std::endl;
        return true;
    }

    std::cout << "[" << nodeName << "] BEFORE: " << beforeVal << " (setting to: " << val << ")" << std::endl;
    arv_camera_set_integer(_camera, nodeName.c_str(), val, &error);
    if (error) {
        return handleError(error);
    }

    gint64 min = G_MININT64;
    gint64 max = G_MAXINT64;
    arv_camera_get_integer_bounds(_camera, nodeName.c_str(), &min, &max, &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "    -> bounds: <" << min << "; " << max << ">" << std::endl;

    const auto afterVal = arv_camera_get_integer(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "[" << nodeName << "] AFTER: " << afterVal << std::endl;
    return true;
}

bool ReadWriteHelper::testFloatFeature(const std::string& nodeName, const AccessType access, double val) {
    GError* error = nullptr;

    if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
        std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
        return true;
    }
    if (error) {
        return handleError(error);
    }

    const auto beforeVal = arv_camera_get_float(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    if (access == RO) {
        std::cout << "[" << nodeName << "]: " << beforeVal << std::endl;
        return true;
    }

    std::cout << "[" << nodeName << "] BEFORE: " << beforeVal << " (setting to: " << val << ")" << std::endl;
    arv_camera_set_float(_camera, nodeName.c_str(), val, &error);
    if (error) {
        return handleError(error);
    }

    double min = -G_MAXDOUBLE;
    double max = G_MAXDOUBLE;
    arv_camera_get_float_bounds(_camera, nodeName.c_str(), &min, &max, &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "    -> bounds: <" << min << "; " << max << ">" << std::endl;

    const auto afterVal = arv_camera_get_float(_camera, nodeName.c_str(), &error);
    if (error) {
        return handleError(error);
    }
    std::cout << "[" << nodeName << "] AFTER: " << afterVal << std::endl;
    return true;
}

bool ReadWriteHelper::testEnumFeature(const std::string& nodeName, const AccessType access, const std::string& val) {
    GError* error = nullptr;

    if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
        std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
        return true;
    }
    if (error) {
        return handleError(error);
    }

    std::cout << "[" << nodeName << "] Enumerations:" << std::endl;
    std::vector<std::pair<gint64, std::string>> featurePairs;
    guint enumValues = -1;
    gint64* compSelEnumVals = arv_camera_dup_available_enumerations(_camera, nodeName.c_str(), &enumValues, &error);
    if (error) {
        return handleError(error);
    }
    guint strValues = -1;
    const char** compSelStrVals = arv_camera_dup_available_enumerations_as_strings(_camera, nodeName.c_str(), &strValues, &error);
    if (error) {
        return handleError(error);
    }

    if (enumValues != strValues) {
        std::cerr << "!!! [" << nodeName << "] enumeration readout error !!!\n";
        return false;
    }

    for (guint i = 0; i < enumValues; i++) {
        std::cout << "    -> " << compSelStrVals[i] << "(" << compSelEnumVals[i] << ")";
        if (!arv_camera_is_enumeration_entry_available(_camera, nodeName.c_str(), compSelStrVals[i], &error)) {
            std::cout << " - ENTRY NOT AVAILABLE!";
        }
        std::cout << std::endl;
        if (error) {
            return handleError(error);
        }
    }

    if (access == RO || access == RW) {
        const auto beforeEnumVal = arv_camera_get_integer(_camera, nodeName.c_str(), &error);
        if (error) {
            return handleError(error);
        }
        const auto beforeStrVal = arv_camera_get_string(_camera, nodeName.c_str(), &error);
        if (error) {
            return handleError(error);
        }

        std::cout << "[" << nodeName << "] BEFORE: " << beforeStrVal << "(" << beforeEnumVal << ")" << std::endl;
    }

    if (access == RO) {
        return true;
    }

    std::cout << "  -> changing to: " << val << std::endl;

    // setter for enum values:
    //arv_camera_set_integer(_camera, nodeName.c_str(), val, &error);
    arv_camera_set_string(_camera, nodeName.c_str(), val.c_str(), &error);
    if (error) {
        return handleError(error);
    }

    if(access == RW) {
        const auto afterEnumVal = arv_camera_get_integer(_camera, nodeName.c_str(), &error);
        if(error) {
            return handleError(error);
        }
        const auto afterStrVal = arv_camera_get_string(_camera, nodeName.c_str(), &error);
        if(error) {
            return handleError(error);
        }

        std::cout << "[" << nodeName << "] AFTER: " << afterStrVal << "(" << afterEnumVal << ")" << std::endl;
    }

    return true;
}

ArvGc* ReadWriteHelper::getGenicam() {
    ArvDevice* device = arv_camera_get_device(_camera);
    return arv_device_get_genicam(device);
}

bool ReadWriteHelper::handleError(GError* error) {
    if (error) {
        std::cerr << " ReadWriteHelper error: " << error->message;
    }
    std::cerr << std::endl;
    return false;
}

std::vector<uint8_t> ReadWriteHelper::getBuffer(const std::string& nodeName) {
    ArvGc* genicam = getGenicam();
    ArvGcNode* node = arv_gc_get_node(genicam, nodeName.c_str());

    GError* error = nullptr;
    guint64 length = arv_gc_register_get_length(ARV_GC_REGISTER(node), &error);
    if (error) {
        handleError(error);
        return {};
    }

    std::vector<uint8_t> buffer(length);
    arv_gc_register_get(ARV_GC_REGISTER(node), &buffer[0], length, &error);
    if (error) {
        handleError(error);
        return {};
    }
    return buffer;
}

}  // namespace pho
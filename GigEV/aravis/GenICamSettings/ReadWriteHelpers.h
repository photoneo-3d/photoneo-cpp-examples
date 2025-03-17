#pragma once
#ifndef __READ_WRITE_HELPER_H__
#define __READ_WRITE_HELPER_H__

#include <arv.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum AccessType {
    RO, WO, RW
};

namespace {
    static gint64 ZERO_INT{0};
    static double ZERO_DOUBLE{0.0};
    static std::string EMPTY_STRING;
    static std::vector<gint64> EMPTY_INT_VECTOR;
    static std::vector<double> EMPTY_DOUBLE_VECTOR;
    static std::vector<std::string> EMPTY_STRING_VECTOR;
}

namespace pho {

class ReadWriteHelper {
public:
    ReadWriteHelper(ArvCamera* camera) : _camera(camera) {}

    bool testCommandFeature(const std::string& nodeName);
    bool testBooleanFeature(const std::string& nodeName, const AccessType access = RO, bool val = false);
    bool testStringFeature(const std::string& nodeName, const AccessType access = RO, const std::string& val = "");
    bool testIntegerFeature(const std::string& nodeName, const AccessType access = RO, gint64 val = 0);
    bool testFloatFeature(const std::string& nodeName, const AccessType access = RO, double val = 0.0);
    bool testEnumFeature(const std::string& nodeName, const AccessType access = RO, const std::string& val = "");

    //readOnlyForNow
    template<typename T>
    bool testArrayFeature(const std::string& nodeName, const std::vector<T>& val, const AccessType access = RO) {
        GError* error = nullptr;

        if (!arv_camera_is_feature_available(_camera, nodeName.c_str(), &error)) {
            std::cout << "[" << nodeName << "]  >> FEATURE IS NOT AVAILABLE FOR THIS DEVICE! <<" << std::endl;
            return true;
        }
        if (error) {
            return handleError(error);
        }

        auto buffer = getBuffer(nodeName);
        if (buffer.empty()) {
            return false;
        }

        if(access == RO) {
            std::cout << "[" << nodeName << "]: " << std::endl;
            printBuffer<T>(buffer);
            return true;
        }

        std::cout << "[" << nodeName << "] BEFORE: " << std::endl;
        printBuffer<T>(buffer);
        std::cout << "[" << nodeName << "] SETTING TO:" << std::endl;
        printBuffer<T>(val);

        if(!setBuffer<T>(val, nodeName)) {
            return false;
        }

        auto afterBuffer = getBuffer(nodeName);
        std::cout << "[" << nodeName << "] AFTER:" << std::endl;
        printBuffer<T>(afterBuffer);

        return true;
    }

    ArvGc* getGenicam();

private:
    bool handleError(GError* error = nullptr);
    std::vector<uint8_t> getBuffer(const std::string& nodeName);

    template<typename T>
    bool setBuffer(const std::vector<T>& buffer, const std::string& nodeName) {
        ArvGc* genicam = getGenicam();
        ArvGcNode* node = arv_gc_get_node(genicam, nodeName.c_str());

        GError* error = nullptr;
        guint64 address = arv_gc_register_get_address(ARV_GC_REGISTER(node), &error);
        if (error) {
            handleError(error);
            return false;
        }

        guint64 length = arv_gc_register_get_length(ARV_GC_REGISTER(node), &error);
        if (error) {
            handleError(error);
            return false;
        }

        if(length > buffer.size() * sizeof(T)) {
            std::cerr << "[" << nodeName << "] Provided array is smaller than expected length " << length << std::endl;
        }

        arv_gc_register_set(ARV_GC_REGISTER(node), &buffer[0], length, &error);
        if (error) {
            handleError(error);
            return false;
        }

        return true;
    }

    template<typename T, template<typename, typename...> typename Container, typename CT, typename... Rest>
    void printBuffer(const Container<CT, Rest...>& buffer) {
        if(buffer.empty()) {
            return;
        }

        T* val = (T*)&buffer[0];
        size_t count = 0;

        if constexpr(std::is_same_v<T, CT>) {
            count = buffer.size();
        } else if constexpr(std::is_same_v<CT, uint8_t>) {
            if constexpr(std::is_same_v<T, std::string>) {
                std::string outStr(buffer.begin(), buffer.end());
                std::cout << "    " << outStr << std::endl;
                return;
            }

            count = buffer.size() / sizeof(T);
        } else {
            std::cerr << "Unsupported buffer for print!" << std::endl;
            return;
        }

        if(count == 3) {
            std::cout
                << "    [" << val[0] << "]\n"
                << "    [" << val[1] << "]\n"
                << "    [" << val[2] << "]" << std::endl;
        } else if(count == 9) {
            std::cout
                << "    [" << val[0] << ", " << val[1] << ", " << val[2] << "]\n"
                << "    [" << val[3] << ", " << val[4] << ", " << val[5] << "]\n"
                << "    [" << val[6] << ", " << val[7] << ", " << val[8] << "]" << std::endl;
        } else {
            std::cout << "    [";
            for (size_t i = 0; i < count - 1; ++i) {
                std::cout << val[i] << ", ";
            }
            std::cout << val[count - 1] << "]" << std::endl;
        }
    }

    ArvCamera* _camera;
};

}  // namespace pho

#endif  // __READ_WRITE_HELPER_H__

#pragma once
#ifndef __READ_WRITE_HELPER_H__
#define __READ_WRITE_HELPER_H__

extern "C" {
#include <arv.h>
}

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
    bool testArrayFeature(const std::string& nodeName, std::vector<T>& val, const AccessType access = RO) {
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
        std::cerr << "[" << nodeName << "]: " << std::endl;

        if constexpr(std::is_same_v<gint64, T>) {
            uint32_t* ptr = (uint32_t*)&buffer[0];
            std::cerr << "    ";
            size_t bufferIntVals = buffer.size() / sizeof(uint32_t);
            for (size_t i = 0; i < bufferIntVals - 1; i++) {
                std::cerr << ptr[i] << ", ";
            }
            std::cerr << ptr[bufferIntVals - 1] << std::endl;
        } else if constexpr(std::is_same_v<double, T>) {
            double* ptr = (double*)&buffer[0];
            if (buffer.size() == (9 * sizeof(double))) {
                std::cerr 
                    << "    [" << ptr[0] << ", " << ptr[1] << ", " << ptr[2] << "]" << std::endl
                    << "    [" << ptr[3] << ", " << ptr[4] << ", " << ptr[5] << "]" << std::endl
                    << "    [" << ptr[6] << ", " << ptr[7] << ", " << ptr[8] << "]" << std::endl;
            } else {
                std::cerr << "    ";
                size_t bufferDoubleVals = buffer.size() / sizeof(double);
                for (size_t i = 0; i < bufferDoubleVals - 1; i++) {
                    std::cerr << ptr[i] << ", ";
                }
                std::cerr << ptr[bufferDoubleVals - 1] << std::endl;
            }
        } else if (std::is_same<std::string, T>::value) {
            std::string strBuffer(buffer.begin(), buffer.end());
            std::cerr << strBuffer << std::endl;
        } else {
            std::cerr << "Wrong value type: " << typeid(T).name() << std::endl;
            return false;
        }

        return true;
    }

    ArvGc* getGenicam();

private:
    bool handleError(GError* error = nullptr);
    std::vector<uint8_t> getBuffer(const std::string& nodeName);

    ArvCamera* _camera;
};

}  // namespace pho

#endif  // __READ_WRITE_HELPER_H__

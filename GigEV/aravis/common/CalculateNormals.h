#ifndef PHOTONEOMAIN_CALCULATENORMALS_H
#define PHOTONEOMAIN_CALCULATENORMALS_H

#include "PhoAravisCommon.h"
#include <cmath>

namespace pho {

inline std::vector<Vec3D> calculateNormals(const NormalsAngles* normalsAngles, uint32_t width, uint32_t height) {
    //Initialize table of angles once, it does not change
    static std::vector<Vec3D> normalsAnglesTable = []() {
        std::vector<Vec3D> table(256 * 256);
        const float pi = 3.14159265359f;
        for(int y = 0; y < 256; ++y) {
            for(int x = 0; x < 256; ++x) {
                int i = y * 256 + x;
                float azimuthalAngle = (float) x * pi / 128.0f;
                float polarAngle = (float) y * pi / 256.0f;
                table[i].z = cosf(polarAngle);
                float radius = sinf(polarAngle);
                table[i].x = radius * cosf(azimuthalAngle);
                table[i].y = radius * sinf(azimuthalAngle);
            }
        }
        return table;
    }();

    std::vector<Vec3D> normals(width * height);
    for(size_t i = 0; i < width * height; ++i) {
        normals[i] = normalsAnglesTable[normalsAngles[i].y * 256 + normalsAngles[i].x];
    }

    return normals;
}

}

#endif //PHOTONEOMAIN_CALCULATENORMALS_H

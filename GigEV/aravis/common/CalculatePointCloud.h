#ifndef PHOTONEOMAIN_CALCULATEPOINTCLOUD_H
#define PHOTONEOMAIN_CALCULATEPOINTCLOUD_H

#include "PhoAravisCommon.h"

namespace pho {

inline std::vector<Vec3D> calculatePointCloud(const float* depthMap, const Vec2D* reprojectionMap,
        uint32_t width, uint32_t height) {
    std::vector<Vec3D> pointCloud(width * height);
    for(uint32_t i = 0; i < width * height; ++i) {
        const float& depth = depthMap[i];
        const Vec2D r = reprojectionMap[i];
        Vec3D& p = pointCloud[i];
        p.x = r.x * depth;
        p.y = r.y * depth;
        p.z = depth;
    }
    return pointCloud;
}

}

#endif //PHOTONEOMAIN_CALCULATEPOINTCLOUD_H

#pragma once
#include <PhoXi.h>
#include <opencv2/core.hpp>

namespace externalCamera {
/**
 * A class representing the interaction with the external camera.
 *
 * This implementation contains only stubs that should be
 * re-implemented with the actual way to retrieve images from
 * the camera.
 */
class ExternalCamera {
public:
    /**
     * Get a grayscale image from external camera for calibration.
     *
     *   REIMPLEMENT THIS with the actual method of getting images
     *   from the external camera.
     */
    pho::api::Texture32f getCalibrationImage();


    /**
     * Get a color image from external camera for color point cloud calculation.
     *
     *   REIMPLEMENT THIS with the actual method of getting images
     *   from the external camera.
     */
    cv::Mat getColorImage();
};

} // namespace externalCamera

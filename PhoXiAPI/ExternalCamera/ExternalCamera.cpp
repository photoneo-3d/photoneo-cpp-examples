#include "ExternalCamera.h"
#include <PhoXiOpenCVSupport.h>

namespace externalCamera {

pho::api::Texture32f ExternalCamera::getCalibrationImage()
{
    // TODO implement me
    cv::Mat cvImage;  /* = cv::imread(...)  or similar */

    // Convert to Texture32f
    pho::api::Texture32f image;
    pho::api::ConvertOpenCVMatToMat2D(cvImage, image);
    return image;
}

cv::Mat ExternalCamera::getColorImage()
{
    cv::Mat image;
    // TODO implement me
    return image;
};

} // namespace externalCamera

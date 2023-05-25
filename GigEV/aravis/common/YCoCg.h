#ifndef PHOTONEOMAIN_YCOCG_H
#define PHOTONEOMAIN_YCOCG_H

#include <cstdint>
#include <opencv2/core/core.hpp>

namespace pho {
/**
 * Utility class providing conversion function for images.
 * Provides conversion
 *     BGR  <--> YCoCg 4:2:0,
 * where the chromatic channels Co, Cg are subsampled in half resolution.
 * The 2x2 plaquette of the YCoCg format consists of:
 *   (0, 0): Y (10 bits), 1st half of Co (6 bits)
 *   (1, 0): Y (10 bits), 2nd half of Co (6 bits)
 *   (0, 1): Y (10 bits), 1st half of Cg (6 bits)
 *   (1, 1): Y (10 bits), 2nd half of Cg (6 bits)
 *
 * Black pixels are treated specially in order to prevent artifacts in images containing valid pixels only in a subregion.
 *
 * NOTE: The YCoCg format used here differs from the reference by:
 *        - a factor 2 in the both chromatic channels Co, Cg,
 *        - an offset equal to the saturation value, which is added to Co and Cg to prevent negative values.
 *
 * @see https://en.wikipedia.org/wiki/YCoCg
 */
class YCoCg {
public:
    // Subsampling of the chromatic channels Co, Cg.
    // The subsampling is meant both in horizontal and vertical direction.
    static const int chromaSubsampling = 2;

    // Pixel depth of B, G, R, and Y channels. Chromatic channels Co and Cg have an additional bit.
    static const int pixelDepth = 10;

    using ChannelType = std::uint16_t;
    using BGRType = cv::Vec<ChannelType, 3>;
    using RGBType = cv::Vec<ChannelType, 3>;
    using YCoCgType = ChannelType;

    // Converts the image in the YCoCg format to BGR.
    // Preconditions: even-sized matrix.
    static cv::Mat_<BGRType> convertToRGB(const cv::Mat_<YCoCgType>& ycocgImg);

private:
    // Pixel conversions.
    static RGBType pixelRGB(const ChannelType y, const ChannelType co, const ChannelType cg);
};

YCoCg::RGBType YCoCg::pixelRGB(const ChannelType y, const ChannelType co, const ChannelType cg) {
    if (y == ChannelType(0)) {
        return {0, 0, 0};
    }
    const ChannelType delta = (1 << (pixelDepth - 1));
    const ChannelType maxValue = 2 * delta - 1;
    ChannelType r1 = 2 * y + co;
    ChannelType r = r1 > cg ? (r1 - cg) / 2 : ChannelType(0);
    ChannelType g1 = y + cg / 2;
    ChannelType g = g1 > delta ? (g1 - delta) : ChannelType(0);
    ChannelType b1 = y + 2 * delta;
    ChannelType b2 = (co + cg) / 2;
    ChannelType b = b1 > b2 ? (b1 - b2) : ChannelType(0);
    return {std::min(r, maxValue), std::min(g, maxValue), std::min(b, maxValue)};
}

cv::Mat_<YCoCg::RGBType> YCoCg::convertToRGB(const cv::Mat_<YCoCgType>& ycocgImg) {
    const int yShift = std::numeric_limits<ChannelType>::digits - pixelDepth;
    const ChannelType mask = static_cast<ChannelType>((1 << yShift) - 1);

    cv::Mat_<RGBType> rgbImg(ycocgImg.size());
    for (int row = 0; row < ycocgImg.rows; row += 2) {
        for (int col = 0; col < ycocgImg.cols; col += 2) {
            const ChannelType y00 = ycocgImg(row, col) >> yShift;
            const ChannelType y01 = ycocgImg(row, col + 1) >> yShift;
            const ChannelType y10 = ycocgImg(row + 1, col) >> yShift;
            const ChannelType y11 = ycocgImg(row + 1, col + 1) >> yShift;
            const ChannelType co = ((ycocgImg(row, col) & mask) << yShift) + (ycocgImg(row, col + 1) & mask);
            const ChannelType cg = ((ycocgImg(row + 1, col) & mask) << yShift) + (ycocgImg(row + 1, col + 1) & mask);
            // Note: We employ neareast neighbor interpolation for the subsampled Co, Cg channels.
            //       It's possible to implement bilinear interpolation in those channels.
            rgbImg(row, col)         = pixelRGB(y00, co, cg);
            rgbImg(row, col + 1)     = pixelRGB(y01, co, cg);
            rgbImg(row + 1, col)     = pixelRGB(y10, co, cg);
            rgbImg(row + 1, col + 1) = pixelRGB(y11, co, cg);
        }
    }
    return rgbImg;
}
}

#endif  // PHOTONEOMAIN_YCOCG_H

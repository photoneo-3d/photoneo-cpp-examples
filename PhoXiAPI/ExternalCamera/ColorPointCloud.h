#pragma once
#define PHOXI_OPENCV_SUPPORT
#include <PhoXi.h>
#include <string>

namespace externalCamera {

/**
 * Calculate and save the color point coud for a scanner frame (a praw file)
 * and an external camera image file specified on the commandline.
 *
 * Uses the example data in Data folder if no files are specified.
 *
 * @param factory the PhoXi Factory used to create devices
 * @param argc,argv the rest of the commandline containing the input files
 */
void colorPointCloudFromFile(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]);

/**
 * Interactively get frames / images from a connected scanner and external
 * camera and calculate and save a color point cloud for them.
 *
 * @param factory the PhoXi Factory used to create devices
 */
void colorPointCloudInteractive(
        pho::api::PhoXiFactory& factory);

} // namespace externalCamera

#pragma once
#define PHOXI_OPENCV_SUPPORT
#include <PhoXi.h>
#include <string>

namespace externalCamera {

/**
 * Calculate and save the depth map from the viewpoint of the external camera
 * from a frame (a praw file) specified on the commandline.
 *
 * Uses the example data in Data folder if no files are specified.
 *
 * @param factory the PhoXi Factory used to create devices
 * @param argc,argv the rest of the commandline containing the praw file and
 *        output file
 */
void depthMapFromFile(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]);

/**
 * Interactively get frames from a connected scanner
 * and the calculated depth map for them.
 *
 * @param factory the PhoXi Factory used to create devices
 */
void depthMapInteractive(
        pho::api::PhoXiFactory& factory);

} // namespace externalCamera

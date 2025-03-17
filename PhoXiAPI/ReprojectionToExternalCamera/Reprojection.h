#pragma once

#include <PhoXi.h>
#include <string>

namespace reprojectionToExternalCamera {

/**
 * Interactively get the reprojected depth map from a connected scanner.
 *
 * @param factory the PhoXi Factory used to create devices
 */
void reprojectionInteractive(
        pho::api::PhoXiFactory& factory);

} // namespace reprojectionToExternalCamera

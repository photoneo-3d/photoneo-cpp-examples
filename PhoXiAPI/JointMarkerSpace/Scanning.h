#pragma once

#include <PhoXi.h>


namespace jointMarkerSpace {

/**
 * Get point clouds from both connected scanners and store them to file.
 *
 * @param primaryDevice device used as a primary in marker recognition
 * @param secondaryDevice device used as a secondary in marker recognition
 * @param counter sequence number of point cloud pair
 */
void trigAndSavePointClouds(
    pho::api::PPhoXi& primaryDevice, pho::api::PPhoXi& secondaryDevice, int counter);

} // namespace jointMarkerSpace

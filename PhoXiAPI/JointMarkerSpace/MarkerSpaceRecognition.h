#pragma once

#include <PhoXi.h>


namespace jointMarkerSpace {

/**
 * Setup devices using:
 *   1. marker recognition,
 *   2. readout of marker transformations,
 *   3. computation of a relative transformation between the devices,
 *   4. and setting it to the primary device.
 *
 * @param primaryDevice the coordinate space of this camera will be changed to match
 *          the secondary device's coordinate space
 * @param secondaryDevice the coordinate space of this camera will be its primary camera space
 *          which is the destination space of both cameras
 */
void recognizeMarkerAndSetupDevices(pho::api::PPhoXi& primaryDevice, pho::api::PPhoXi& secondaryDevice);

}  // namespace jointMarkerSpace

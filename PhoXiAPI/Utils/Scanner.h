#pragma once

#include <PhoXi.h>

#include <string>

namespace utils {

std::string selectAvailableDevice(pho::api::PhoXiFactory& factory);
pho::api::PPhoXi selectAndConnectDevice(pho::api::PhoXiFactory& factory);
pho::api::PPhoXi connectDevice(pho::api::PhoXiFactory& factory, const std::string& type);
pho::api::PFrame triggerScanAndGetFrame(pho::api::PPhoXi device);
void disconnectOrLogOut(pho::api::PPhoXi device);

}   // namespace utils

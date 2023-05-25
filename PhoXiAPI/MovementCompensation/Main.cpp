#include <thread>
#include <sstream>
#include <fstream>

#include <PhoXi.h>

#include "Utils/FileCamera.h"
#include "Utils/Checks.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"
#include "Utils/Checks.h"
#include "Utils/Info.h"

#define LOCAL_CROSS_SLEEP(Millis) std::this_thread::sleep_for(std::chrono::milliseconds(Millis));

pho::api::Point3_32f loadVelocityVector() {
    pho::api::Point3_32f velocity;

    std::ifstream velocityFile(utils::Path::join(utils::Path::dataFolder(), "velocity.txt"));
    if (velocityFile.is_open()) {
        std::string line;
        std::getline(velocityFile, line);

        std::istringstream stream(line);
        stream >> velocity.x >> velocity.y >> velocity.z;
    }

    return velocity;
}

std::string dumpVelocityVector(const pho::api::Point3_32f& vec)
{
    std::stringstream ss;
    ss    << "x = " << vec.x << "mm/ms"
        << ", y = " << vec.y << "mm/ms"
        << ", z = " << vec.z << "mm/ms";
    return ss.str();
}

void applyVelocityToPointCloud(pho::api::PhoXiFactory &factory) {

    // Velocity vector in units of millimeters per millisecond (= meter per second)
    pho::api::Point3_32f velocity = loadVelocityVector();
    std::cout << "Loaded velocity vector " << dumpVelocityVector(velocity) << std::endl;

    std::vector<std::string> prawNames;

    prawNames.push_back(utils::Path::join(utils::Path::dataFolder(), "original.praw"));

    // Attach praw file to a Camera
    utils::AttachedFileCamera fileCamera{factory, prawNames};
    auto device = fileCamera.connect();

    // We just need to trigger a scan, but don't need the result, because the
    // aligner will use the data present in PhoXiControl
    pho::api::PFrame frame = utils::triggerScanAndGetFrame(device);

    utils::printMat2D(frame->EventMap, "EventMap");

    // Log out the device from PhoXi Control
    device->Disconnect(true);

    // Store the original frame to ply file -> for easy compere with "motion-compensated.ply"
    utils::saveFrameToPly(frame, utils::Path::join(utils::Path::dataFolder(), "original.ply"));

    const pho::api::Depth_32f zero_float = 0.0f;
    auto size = frame->EventMap.Size;

    for (int row = 0; row < size.Height; ++row) {
        for (int col = 0; col < size.Width; ++col) {
            const auto& depth = frame->DepthMap[row][col];
            
            if (depth != zero_float) {
                auto& point = frame->PointCloud[row][col];
                const auto& time = frame->EventMap[row][col];

                // Apply opposite shifts depending on the time and speed of the scene to each point
                point.x -= velocity.x * time;
                point.y -= velocity.y * time;
                point.z -= velocity.z * time;
            }            
        }
    }

    // Store the frame after motion compensation as a ply structure
    // the PLY file will be saved in the Data folder where the Example executable is located
    const auto sampleFramePly = utils::Path::join(utils::Path::dataFolder(), "motion-compensated.ply");
    
    utils::saveFrameToPly(frame, sampleFramePly);
}

int main(int argc, char* argv[]) {
    std::cout << "Movement Compensation Example" << std::endl << std::endl;

    pho::api::PhoXiFactory factory;

    utils::Path::setProjectFolder("");

    utils::PhoXiControlRunning(factory);

    applyVelocityToPointCloud(factory);

    return 0;
}
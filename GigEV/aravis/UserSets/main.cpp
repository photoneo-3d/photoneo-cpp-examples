/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"

using namespace pho;

void printValues(ArvCamera* camera) {
    std::cout << "UserSetDescription: " << arv_camera_get_string(camera, "UserSetDescription", nullptr) << std::endl;
    std::cout << "OperationMode: " << arv_camera_get_string(camera, "OperationMode", nullptr) << std::endl;
    std::cout << "CameraExposure: " << arv_camera_get_float(camera, "CameraExposure", nullptr) << std::endl;
    std::cout << "CameraTextureSource: " << arv_camera_get_string(camera, "CameraTextureSource", nullptr) << std::endl;
    std::cout << "---" << std::endl;
}

int main (int argc, char **argv)
{
    if(argc < 2) {
        std::cerr << "Provide device IP as a parameter!" << std::endl;
        return 1;
    }

    const char* deviceIp = argv[1];

    GError *error = nullptr;

    /* Connect to the camera */
    auto camera = create_gobject_unique(arv_camera_new (deviceIp, &error));
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if(!ARV_IS_CAMERA (camera.get())) {
        std::cerr << "Error: Not a camera instance!";
        return 1;
    }

    std::cout << "Connected to camera: " << arv_camera_get_model_name (camera.get(), nullptr) << std::endl;


    /* Select, reset, modify and save UserSet0 */
    arv_camera_set_string(camera.get(), "UserSetSelector", "UserSet0", &error);
    if(error) {
        std::cerr << "Failed to select UserSetSelector='UserSet0'!" << std::endl;
    }

    arv_camera_execute_command(camera.get(), "UserSetReset", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetReset!" << std::endl;
    }

    arv_camera_set_string(camera.get(), "UserSetDescription", "My custom UserSet", nullptr);
    arv_camera_set_string(camera.get(), "OperationMode", "Camera", nullptr);
    arv_camera_set_float(camera.get(), "CameraExposure", 30.72, nullptr);
    arv_camera_set_string(camera.get(), "CameraTextureSource", "Laser", nullptr);

    arv_camera_execute_command(camera.get(), "UserSetSave", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetSave!" << std::endl;
    }


    /* Select, reset, modify and save UserSet1 */
    arv_camera_set_string(camera.get(), "UserSetSelector", "UserSet1", &error);
    if(error) {
        std::cerr << "Failed to select UserSetSelector='UserSet1'!" << std::endl;
    }

    arv_camera_execute_command(camera.get(), "UserSetReset", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetReset!" << std::endl;
    }

    arv_camera_set_string(camera.get(), "UserSetDescription", "My other custom UserSet", nullptr);
    arv_camera_set_string(camera.get(), "OperationMode", "Scanner", nullptr);
    arv_camera_set_float(camera.get(), "CameraExposure", 40.96, nullptr);
    arv_camera_set_string(camera.get(), "CameraTextureSource", "LED", nullptr);

    arv_camera_execute_command(camera.get(), "UserSetSave", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetSave!" << std::endl;
    }


    /* Load saved user sets */
    /* Default factory user set (read only) */
    arv_camera_set_string(camera.get(), "UserSetSelector", "Default", &error);
    if(error) {
        std::cerr << "Failed to select UserSetSelector='Default'!" << std::endl;
    }
    arv_camera_execute_command(camera.get(), "UserSetLoad", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetLoad!" << std::endl;
    }
    printValues(camera.get());

    /* Custom UserSet0 */
    arv_camera_set_string(camera.get(), "UserSetSelector", "UserSet0", &error);
    if(error) {
        std::cerr << "Failed to select UserSetSelector='UserSet0'!" << std::endl;
    }
    arv_camera_execute_command(camera.get(), "UserSetLoad", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetLoad!" << std::endl;
    }
    printValues(camera.get());

    /* Custom UserSet1 */
    arv_camera_set_string(camera.get(), "UserSetSelector", "UserSet1", &error);
    if(error) {
        std::cerr << "Failed to select UserSetSelector='UserSet1'!" << std::endl;
    }
    arv_camera_execute_command(camera.get(), "UserSetLoad", &error);
    if(error) {
        std::cerr << "Failed to execute UserSetLoad!" << std::endl;
    }
    printValues(camera.get());

    return 0;
}

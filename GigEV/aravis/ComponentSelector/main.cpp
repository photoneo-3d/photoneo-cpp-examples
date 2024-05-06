/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"

#include <string>
#include <vector>

using namespace pho;

int main (int argc, char **argv)
{
    if(argc < 2) {
        std::cerr << "Provide device IP as a parameter!" << std::endl;
        return 1;
    }

    const char* deviceIp = argv[1];

    GError *error = nullptr;

    /* Connect to the first available camera */
    auto camera = create_gobject_unique(arv_camera_new (deviceIp, &error));
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if(!ARV_IS_CAMERA (camera.get())) {
        std::cerr << "Error: Not a camera instance!";
        return 1;
    }

    std::cerr << "Connected to camera: " << arv_camera_get_model_name (camera.get(), nullptr) << std::endl;

    /* Set trigger mode */
    if(!setTriggerMode(camera.get(), TriggerMode::SWTrigger)) {
        return 1;
    }

    /* Create the stream object */
    auto stream = create_gobject_unique(arv_camera_create_stream (camera.get(), nullptr, nullptr, &error));
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if(!ARV_IS_STREAM(stream.get())) {
        std::cerr << "Error: Not a stream instance!";
        return 1;
    }

    arv_camera_gv_set_packet_size_adjustment (camera.get(), ARV_GV_PACKET_SIZE_ADJUSTMENT_ALWAYS);

    /* Get ComponentSelector enumerations */
    std::vector<std::pair<uint32_t, std::string>> componentStrIdPairs;
    guint enumValues = -1;
    gint64* compSelEnumVals = arv_camera_dup_available_enumerations (
            camera.get(),
            "ComponentSelector",
            &enumValues,
            &error);
    if(error) {
        std::cerr << "Error: Cannot retrieve ComponentSelector enumerations!";
        return 1;
    }
    guint strValues = -1;
    const char** compSelStrVals = arv_camera_dup_available_enumerations_as_strings (
            camera.get(),
            "ComponentSelector",
            &strValues,
            &error);
    if(error) {
        std::cerr << "Error: Cannot retrieve ComponentSelector string enumerations!";
        return 1;
    }

    if(enumValues == strValues) {
        std::cout << "Camera supports these components:\n";
        for(guint i = 0; i < enumValues; i++) {
            arv_camera_set_string (camera.get(), "ComponentSelector", compSelStrVals[i], &error);
            //or arv_camera_set_integer (camera.get(), "ComponentSelector", compSelEnumVals[i], &error);
            if(error) {
                std::cerr << "Error: Cannot retrieve ComponentSelector value!";
                return 1;
            }
            const auto idVal = arv_camera_get_integer (camera.get(), "ComponentIDValue", &error);
            if(error) {
                std::cerr << "Error: Cannot retrieve ComponentIDValue value!";
                return 1;
            }
            std::cout << "    [EnumID: " << compSelEnumVals[i] << "] ValID: = " << idVal << ": " << compSelStrVals[i] << std::endl;
            componentStrIdPairs.push_back({idVal, compSelStrVals[i]});

            /* Enable every supported component */
            arv_camera_set_boolean (camera.get(), "ComponentEnable", true, &error);
            if(error) {
                std::cerr << "Error: Cannot modify ComponentEnable value!";
                return 1;
            }
        }
    } else {
        std::cerr << "ComponentSelector feature enumeration readout error!";
        return 1;
    }

    /* Set output format BEFORE getting payload size for buffers */
    /* ImageData: Only Texture is sent.
     * MultipartData: Multipart buffer with selected components is sent
     */
    if(!setStreamOutputFormat(camera.get(), StreamOutputFormat::MultipartData)) {
        return 1;
    }

    /* Retrieve the payload size for buffer creation */
    size_t payload = arv_camera_get_payload (camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to obtain payload size!" << std::endl;
        return 1;
    }
    std::cout << "Payload size: " << payload << " bytes" << std::endl;

    /* Insert buffer in the stream buffer pool */
    arv_stream_push_buffer (stream.get(), arv_buffer_new(payload, nullptr));

    /* Start the acquisition */
    arv_camera_start_acquisition (camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to start acquisition!" << std::endl;
        return 1;
    }
    std::cout << "Acquisition started..." << std::endl;

    triggerFrame(camera.get());

    auto* buffer = arv_stream_timeout_pop_buffer (stream.get(), 2000000);
    if (!ARV_IS_BUFFER (buffer)) {
        std::cerr << "Error: Buffer is not a buffer instance!" << std::endl;
        return 1;
    }

    auto nparts = arv_buffer_get_n_parts (buffer);
    std::cout << "MULTIPART Buffer contains: " << nparts << " parts\n";

    gint partId = -1;
    size_t dataSize = 0;
    for(const auto& component: componentStrIdPairs) {
        partId = arv_buffer_find_component(buffer, component.first);
        if(partId == -1) {
            continue;
        }
        std::cout << "    Component[" << component.first << "]: " << component.second << " has part_id = " << partId << std::endl;

        auto data = arv_buffer_get_part_data (buffer, partId, &dataSize);
        auto dataType = arv_buffer_get_part_data_type (buffer, partId);
        std::cout << "    -> Size: " << dataSize << ", Type: " << dataType << std::endl;
    }

    arv_stream_push_buffer (stream.get(), buffer);

    /* Stop the acquisition */
    arv_camera_stop_acquisition (camera.get(), &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Acquisition stopped..." << std::endl;

    return 0;
}

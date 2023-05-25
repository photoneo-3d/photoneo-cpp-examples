/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"

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

    if(!ARV_IS_CAMERA(camera.get())) {
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

    /* Enable required output matrices BEFORE getting payload size for buffers */
    const std::pair<OutputMat, bool> outputMats[] = {
        {Texture, true},
        {DepthMap, true},
        {NormalMap, false},
        {ConfidenceMap, false},
        {EventMap, false},
        {ColorCameraImage, false},
        {ReprojectionMap, false}
    };

    for(const auto& output : outputMats) {
        if(!setOutputMat(camera.get(), output.first, output.second)) {
            return 1;
        }

        /* If NormalMap is enabled, check custom setting NormalsEstimationRadius and if value is 0 set to 1 (range: 1-4) */
        if(output.first == NormalMap && output.second == true) {
            auto radius = arv_camera_get_integer(camera.get(), "NormalsEstimationRadius", &error);
            if(error) {
                std::cerr << "Error: Failed to get NormalsEstimationRadius!" << std::endl;
                return 1;
            }

            if(radius == 0) {
                arv_camera_set_integer(camera.get(), "NormalsEstimationRadius", 1, &error);
                if(error) {
                    std::cerr << "Error: Failed to set NormalsEstimationRadius!" << std::endl;
                    return 1;
                }
            }
        }
    }

    /* Set output format BEFORE getting payload size for buffers */
    /* ChunkData:
     * If Texture is enabled, then Image buffer (optionally with chunks) is sent.
     * If Texture is not enabled, then Chunk data buffer is sent
     *
     * MultipartData:
     * Multipart buffer is always sent
     */
    if(!setStreamOutputFormat(camera.get(), StreamOutputFormat::ChunkData)) {
        return 1;
    }

    /* Retrieve the payload size for buffer creation */
    size_t payload = arv_camera_get_payload (camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to obtain payload size!" << std::endl;
        return 1;
    }
    std::cout << "Payload size: " << payload << " bytes" << std::endl;

    /* Insert some buffers in the stream buffer pool */
    arv_stream_push_buffer(stream.get(), arv_buffer_new(payload, nullptr));

    /* Start the acquisition */
    arv_camera_start_acquisition(camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to start acquisition!" << std::endl;
        return 1;
    }
    std::cout << "Acquisition started..." << std::endl;

    triggerFrame(camera.get());

    auto* buffer = arv_stream_timeout_pop_buffer(stream.get(), 2000000);
    if (!ARV_IS_BUFFER (buffer)) {
        std::cerr << "Error: Buffer is not a buffer instance!" << std::endl;
        return 1;
    }

    std::cout << "Got buffer..." << std::endl;

    arv_stream_push_buffer(stream.get(), buffer);

    /* Stop the acquisition */
    arv_camera_stop_acquisition(camera.get(), &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Acquisition stopped..." << std::endl;

    return 0;
}
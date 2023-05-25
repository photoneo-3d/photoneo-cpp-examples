/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"
#include <iomanip>

using namespace pho;

void handleChunkDataBuffer(ArvBuffer *buffer, uint32_t width, uint32_t height, Vec2D* reprojectionMap) {
    static const OutputMat chunkIds[] = {
        //OutputMat::Texture - not present in the chunks
        OutputMat::DepthMap,
        OutputMat::NormalMap,
        OutputMat::ConfidenceMap,
        OutputMat::EventMap,
        OutputMat::ColorCameraImage,
        OutputMat::ReprojectionMap,
    };

    std::cout << "CHUNK DATA buffer:" << std::endl;

    const float* depthMap = nullptr;

    for(auto chunkId : chunkIds) {
        size_t dataSize = 0;
        const void* data = arv_buffer_get_chunk_data(buffer, static_cast<uint32_t>(chunkId), &dataSize);
        if(!data || dataSize == 0) {
            continue;
        }

        switch(chunkId) {
        case OutputMat::Texture:
            //If texture is enabled then Image buffer with chunks is sent (see: handleImageBuffer)
            break;
        case OutputMat::DepthMap:
            std::cout << "DepthMap received\n";
            depthMap = (float*)data;
            break;
        case OutputMat::NormalMap:
            std::cout << "NormalMap received\n";
            break;
        case OutputMat::ConfidenceMap:
            std::cout << "ConfidenceMap received\n";
            break;
        case OutputMat::EventMap:
            std::cout << "EventMap received\n";
            break;
        case OutputMat::ColorCameraImage:
            std::cout << "ColorCameraImage received\n";
            break;
        case OutputMat::ReprojectionMap:
            std::cout << "ReprojectionMap received\n";
            break;
        }
    }

    /* Calculate point cloud */
    if(depthMap && reprojectionMap) {
        auto pointCloud = std::make_unique<Vec3D[]>(width * height);
        for(uint32_t i = 0; i < width * height; ++i) {
            const float& depth = depthMap[i];
            const Vec2D r = reprojectionMap[i];
            Vec3D& p = pointCloud[i];
            p.x = r.x * depth;
            p.y = r.y * depth;
            p.z = depth;
        }
    }

    std::cout << "-------------------------------" << std::endl;
}

void handleImageBuffer(ArvBuffer *buffer, uint32_t width, uint32_t height, Vec2D* reprojectionMap) {
    std::cout << "IMAGE buffer:" << std::endl;
    std::cout << "Width: " << arv_buffer_get_image_width(buffer) << std::endl;
    std::cout << "Height: " << arv_buffer_get_image_height(buffer) << std::endl;

    if(arv_buffer_has_chunks(buffer)) {
        //If buffer has chunks handle them too
        handleChunkDataBuffer(buffer, width, height, reprojectionMap);
    }
}

void handleMultipartBuffer(ArvBuffer *buffer) {
    auto nparts = arv_buffer_get_n_parts(buffer);
    std::cout << "MULTIPART buffer:" << std::endl;

    for(uint32_t i = 0; i < nparts; ++i) {
        size_t dataSize = 0;
        auto data = arv_buffer_get_part_data(buffer, i, &dataSize);
        auto dataType = arv_buffer_get_part_data_type(buffer, i);
        std::cout << "Part " << i << " size: " << dataSize << " type: " << dataType << std::endl;
    }

    std::cout << "-------------------------------" << std::endl;

}

/*
 * Connect to the first available camera, then acquire 10 buffers.
 */
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

    ///-----------------------------------------------------------------------------------------------------------------

    /* Read frame width */
    uint32_t width = arv_camera_get_integer(camera.get(), "Width", &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Width: " << width << "px" << std::endl;

    /* Read frame height */
    uint32_t height = arv_camera_get_integer(camera.get(), "Height", &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Height: " << height << "px" << std::endl;

    /* Read reprojection matrix for point cloud calculation (no need to request in every frame) */
    Vec2D* reprojectionMap = nullptr;
    size_t reprojectionMapDataSize = 0;
    if(!getReprojectionMatrix(camera.get(), reprojectionMap, reprojectionMapDataSize)) {
        std::cerr << "Error: Failed to read reprojection matrix from camera!" << std::endl;
    }
    std::cout << "Reprojection matrix size: " << reprojectionMapDataSize << std::endl;

    ///-----------------------------------------------------------------------------------------------------------------

    /* Set trigger mode */
    if(!setTriggerMode(camera.get(), TriggerMode::Freerun)) {
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
     * If Texture is not enabled then, Chunk data buffer is sent
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
    for (int i = 0; i < 10; i++) {
        arv_stream_push_buffer(stream.get(), arv_buffer_new(payload, nullptr));
    }

    /* Start the acquisition */
    arv_camera_start_acquisition(camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to start acquisition!" << std::endl;
        return 1;
    }
    std::cout << "Acquisition started..." << std::endl;

    /* Retrieve 10 buffers */
    for (int i = 0; i < 10; i++) {
        auto* buffer = arv_stream_pop_buffer(stream.get());
        if (!ARV_IS_BUFFER (buffer)) {
            std::cerr << "Error: Buffer " << i << " is not a buffer instance!" << std::endl;
            continue;
        }

        auto payloadType = arv_buffer_get_payload_type(buffer);
        switch(payloadType) {
        case ARV_BUFFER_PAYLOAD_TYPE_IMAGE:
            handleImageBuffer(buffer, width, height, reprojectionMap);
            break;
        case ARV_BUFFER_PAYLOAD_TYPE_CHUNK_DATA:
            handleChunkDataBuffer(buffer, width, height, reprojectionMap);
            break;
        case ARV_BUFFER_PAYLOAD_TYPE_MULTIPART:
            handleMultipartBuffer(buffer);
            break;
        default:
            std::cerr << "Unsupported buffer type: 0x" << std::hex << std::setfill('0') << std::setw(4) << payloadType
                      << ". Ignoring..." << std::endl;
            break;
        }

        arv_stream_push_buffer (stream.get(), buffer);
    }

    /* Stop the acquisition */
    arv_camera_stop_acquisition(camera.get(), &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Acquisition stopped..." << std::endl;

    return 0;
}

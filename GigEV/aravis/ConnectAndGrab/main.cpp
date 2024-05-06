/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"
#include "common/CalculateNormals.h"
#include <iomanip>

using namespace pho;

void handleImageBuffer(ArvBuffer *buffer) {
    std::cout << "IMAGE buffer:" << std::endl;
    std::cout << "Width: " << arv_buffer_get_image_width(buffer) << std::endl;
    std::cout << "Height: " << arv_buffer_get_image_height(buffer) << std::endl;
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
        {Intensity, true},
        {Range, true},
        {Normal, true},
        {Confidence, false},
        {Event, false},
        {ColorCameraImage, false},
        {CoordinateMapA, false},
        {CoordinateMapB, false},
    };

    for(const auto& output : outputMats) {
        if(!setOutputMat(camera.get(), output.first, output.second)) {
            return 1;
        }

        if(output.first == Normal && output.second) {
            /* If Normal is enabled, check custom setting NormalsEstimationRadius and if value is 0 set to 1 (range: 1-4) */
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

            /* Set normals pixel format to:
             * Coord3D_ABC32f - vectors, more data, no post-processing needed
             * Coord3D_AC8 - angles of normal vector, less data, required post-processing, see header file
             *               `CalculateNormals.h`
             */
            arv_camera_set_string(camera.get(), "ComponentSelector", "Normal", &error);
            if(error) {
                std::cerr << "Error: Failed to select ComponentSelector='Normal'!";
                return 1;
            }
            arv_camera_set_string(camera.get(), "PixelFormat", "Coord3D_ABC32f", &error);
            if(error) {
                std::cerr << "Error: Failed to set PixelFormat='Coord3D_ABC32f'!";
                return 1;
            }
        }
        /* If Range is enabled set required format */
        else if(output.first == Range && output.second) {
            /*
             * CalibratedABC_Grid -> direct XYZ point cloud, more data, no post-processing
             * ProjectedC -> only projected Z value, less data, requires post-processing with CoordinateMapA and B
             */
            arv_camera_set_string(camera.get(), "Scan3dOutputMode", "CalibratedABC_Grid", &error);
            if(error) {
                std::cerr << "Error: Failed to set Scan3dOutputMode!" << std::endl;
                return 1;
            }
        }
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
            handleImageBuffer(buffer);
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

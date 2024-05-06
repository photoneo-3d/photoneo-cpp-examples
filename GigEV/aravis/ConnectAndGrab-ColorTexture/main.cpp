/* SPDX-License-Identifier:Unlicense */

#include "common/PhoAravisCommon.h"
#include "common/YCoCg.h"

/* For visualization only */
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace pho;

void handleImageDataBufferMono16(ArvBuffer *buffer) {
    size_t dataSize = 0;
    const void* data = arv_buffer_get_image_data(buffer, &dataSize);
    if (!data || dataSize == 0) {
        return;
    }

    auto width = arv_buffer_get_image_width(buffer);
    auto height = arv_buffer_get_image_height(buffer);
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    auto mat = cv::Mat(height, width, CV_16U, (void*)data);
    cv::Mat rgbMat = YCoCg::convertToRGB(mat);

    /* Normalize just for visualization */
    cv::normalize(rgbMat, rgbMat, 0, 255, cv::NORM_MINMAX);
    rgbMat.convertTo(rgbMat, CV_8UC3);

    /* cv::imshow uses BRG format */
    cv::cvtColor(rgbMat, rgbMat, cv::COLOR_RGB2BGR);

    cv::imshow("YCoCg converted to RGB", rgbMat);
    cv::imshow("YCoCg texture", mat);
    cv::waitKey(0);
}

void handleImageDataBufferRGB(ArvBuffer *buffer) {
    size_t dataSize = 0;
    const void* data = arv_buffer_get_image_data(buffer, &dataSize);
    if (!data || dataSize == 0) {
        return;
    }

    auto width = arv_buffer_get_image_width(buffer);
    auto height = arv_buffer_get_image_height(buffer);
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    auto mat = cv::Mat(height, width, CV_8UC3, (void*)data);

    /* Normalize just for visualization */
    cv::normalize(mat, mat, 0, 255, cv::NORM_MINMAX);

    /* cv::imshow uses BRG format */
    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
    cv::imshow("RGB Texture from device", mat);
    cv::waitKey(0);
}

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

    /* Set OperationMode to Camera */
    arv_camera_set_string(camera.get(), "OperationMode", "Camera", &error);
    if(error) {
        std::cerr << "Error: Failed to set OperationMode='Camera'!";
        return 1;
    }

    /* Set TextureSource to Color
     * If OperationMode == Scanner then use TextureSource instead of CameraTextureSource
     */
    arv_camera_set_string(camera.get(), "CameraTextureSource", "Color", &error);
    if(error) {
        std::cerr << "Error: Failed to set CameraTextureSource='Color'!";
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
        {Range, false},
        {Normal, false},
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
    }

    /* Set output format BEFORE getting payload size for buffers */
    /* ImageData: Only Texture is sent.
     * MultipartData: Multipart buffer with selected components is sent
     */
    if(!setStreamOutputFormat(camera.get(), StreamOutputFormat::MultipartData)) {
        return 1;
    }

    /// Example using PixelFormat RGB8 directly from the device
    /* Set texture pixel format to RGB8 */
    arv_camera_set_string(camera.get(), "ComponentSelector", "Intensity", &error);
    if(error) {
        std::cerr << "Error: Failed to select ComponentSelector='Intensity'!";
        return 1;
    }
    arv_camera_set_string(camera.get(), "PixelFormat", "RGB8", &error);
    if(error) {
        std::cerr << "Error: Failed to set PixelFormat='RGB8'!";
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
    handleImageDataBufferRGB(buffer);

    /*
     * In this example changing PixelFormat changes payload size so buffer must be recreated. Delete old one instead
     * of pushing it back to the queue.
     */
    g_object_unref(buffer);


    /// Example using PixelFormat Mono16 in YCoCg coding and conversion to RGB8
    /* Set texture pixel format to Mono16 */
    arv_camera_set_string(camera.get(), "ComponentSelector", "Intensity", &error);
    if(error) {
        std::cerr << "Error: Failed to select ComponentSelector='Intensity'!";
        return 1;
    }
    arv_camera_set_string(camera.get(), "PixelFormat", "Mono16", &error);
    if(error) {
        std::cerr << "Error: Failed to set PixelFormat='Mono16'!";
        return 1;
    }

    /* Retrieve the payload size after PixelFormat change for new buffer creation */
    payload = arv_camera_get_payload (camera.get(), &error);
    if(error) {
        std::cerr << "Error: Failed to obtain payload size!" << std::endl;
        return 1;
    }
    std::cout << "Payload size: " << payload << " bytes" << std::endl;

    /* Insert some buffers in the stream buffer pool */
    arv_stream_push_buffer(stream.get(), arv_buffer_new(payload, nullptr));

    triggerFrame(camera.get());

    buffer = arv_stream_timeout_pop_buffer(stream.get(), 2000000);
    if (!ARV_IS_BUFFER (buffer)) {
        std::cerr << "Error: Buffer is not a buffer instance!" << std::endl;
        return 1;
    }

    std::cout << "Got buffer..." << std::endl;
    handleImageDataBufferMono16(buffer);

    /*
     * In this example changing PixelFormat changes payload size so buffer must be recreated. Delete old one instead
     * of pushing it back to the queue.
     */
    g_object_unref(buffer);


    /* Stop the acquisition */
    arv_camera_stop_acquisition(camera.get(), &error);
    if(error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }
    std::cout << "Acquisition stopped..." << std::endl;

    return 0;
}
/*
* Photoneo's C-API Example - CAPIExample.cpp
* Defines the entry point for the console application.
* This Example shows how to:
* - initialize and exit PhoXi C API
* - call JSON commands to control the device
* - Read the frame from the device
* 
*/

// C API header
#include "phoxi/phoxi_c_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void callback(const char* response, size_t response_size, void* user_data)
{
    char** buffer = (char**)user_data;
    if (*buffer) free(*buffer);  // Free the previous buffer if it exists
    *buffer = (char*)malloc(response_size + 1);
    strcpy(*buffer, response);        // Copy the response to the buffer
    (*buffer)[response_size] = '\0';  // Null-terminate the string
}
 
// Connect to a PhoXi device with the given device ID: "InstalledExamples-basic-example"
const char* request_connect = "{\"command\": \"connect\", \"params\": { \"device_id\": \"InstalledExamples-basic-example\"}}";
const char* request_software_trigger_mode =
    "{ \"command\" : \"trigger_mode\", \"operation\" : \"set\", \"params\" : {\"device_id\" : "
    "\"InstalledExamples-basic-example\", \"mode\" : \"Software\"}}";
const char* request_acquisition_status =
    "{\"command\": \"acquisition\", \"operation\": \"status\", \"params\": { \"device_id\": "
    "\"InstalledExamples-basic-example\"}}";
const char* request_acquisition_start = "{\"command\": \"acquisition\", \"operation\": \"start\", \"params\": { \"device_id\": \"InstalledExamples-basic-example\"}}";
const char* request_trigger_frame =
    "{\"command\": \"trigger_frame\", \"params\": { \"device_id\": \"InstalledExamples-basic-example\", \"wait_accept\": true, \"wait_grabbing_end\": true}}";


// Calculate the number of valid points in the frame record
int calcValidPoints(const struct phoxi_frame_record_t* record) {
    if (record->data == NULL) {
        return 0;
    }
    switch (record->format) {
        case PHOXI_FRAME_FORMAT_POINT3_32F: {
            const float(*points)[3] = (const float(*)[3])record->data;
            int validPoints = 0;
            for (int i = 0; i < record->width * record->height; ++i) {
                if (points[i][0] != 0. || points[i][1] != 0. || points[i][2] != 0.) {
                    validPoints++;
                }
            }
            return validPoints;
        }
        case PHOXI_FRAME_FORMAT_FLOAT_32F: {
            const float* points = (const float*)record->data;
            int validPoints = 0;
            for (int i = 0; i < record->width * record->height; ++i) {
                if (points[i] != 0.) {
                    validPoints++;
                }
            }
            return validPoints;
        }
        case PHOXI_FRAME_FORMAT_RGB_16: {
            const uint16_t(*points)[3] = (const uint16_t(*)[3])record->data;
            int validPoints = 0;
            for (int i = 0; i < record->width * record->height; ++i) {
                if (points[i][0] != 0 || points[i][1] != 0 || points[i][2] != 0) {
                    validPoints++;
                }
            }
            return validPoints;
        }
        default: {
            return 0;  // Unsupported format, return 0 valid points
        }
    }
}

// Acceptor function for frame records
// This function is called when the frame records are received from the device.
// This function should copy data or perform very fast processing, as this call blocks
//  receiving additional frames until it returns.
void frameAcceptor(const struct phoxi_frame_record_t* records, void* user_data) {
    if (records == NULL || records->type == PHOXI_FRAME_TYPE_EMPTY) {
        return;
    }
    char** buffer = (char**)user_data;
    if (*buffer) free(*buffer);  // Free the previous buffer if it exists

    const struct phoxi_frame_record_t* record = records;
    while (record != 0 && record->type != PHOXI_FRAME_TYPE_EMPTY) {
        switch (record->type) {
            case PHOXI_FRAME_TYPE_FRAMEINFO:
                if (record->format == PHOXI_FRAME_FORMAT_STRING) {
                    const char* info = (const char*)record->data;
                    *buffer = (char*)malloc(strlen(info) + 1);
                    strcpy(*buffer, info);
                }
                break;
            case PHOXI_FRAME_TYPE_POINTCLOUD:
                printf("Received PointCloud with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
            case PHOXI_FRAME_TYPE_NORMALMAP:
                printf("Received NormalMap with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
            case PHOXI_FRAME_TYPE_DEPTHMAP:
                printf("Received DepthMap with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
            case PHOXI_FRAME_TYPE_CONFIDENCEMAP:
                printf("Received ConfidenceMap with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
            case PHOXI_FRAME_TYPE_EVENTMAP:
                printf("Received EventMap with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
            case PHOXI_FRAME_TYPE_TEXTURE:
                switch (record->format) {
                    case PHOXI_FRAME_FORMAT_FLOAT_32F: {
                        printf("Received Gray Texture with size: %dx%d, valid points:%d\n", record->width, record->height,
                               calcValidPoints(record));
                        break;
                    }
                    case PHOXI_FRAME_FORMAT_RGB_16: {
                        printf("Received RGB Texture with size: %dx%d, valid points:%d\n", record->width, record->height,
                               calcValidPoints(record));
                        break;
                    }
                };
                break;
            case PHOXI_FRAME_TYPE_COLORCAMERAIMAGE:
                printf("Received RGB ColorCameraImage with size: %dx%d, valid points:%d\n", record->width, record->height,
                       calcValidPoints(record));
                break;
        }
        record++;
    }
}

int main(int argc, char *argv[])
{
    // Initialize the PhoXi C API
    struct phoxi_api_interface_descriptor_t desc;
    int ret = phoxi_init(0, &desc, PHOXI_API_INTERFACE_VERSION);
    if (ret!= PHOXI_OK) {
        printf("Failed to initialize PhoXi API: %d\n", ret);
        return ret;
    }
    printf("PhoXi API initialized successfully. API version is %s\n", desc.api_version);


    // Connect to the PhoXi device
    // The function `phoxi_command_execute` sends a command to the PhoXi device and waits for a response.
    // The `callback` function is called when the response is received, and it stores the response in a buffer.
    // The `timeout` parameter specifies how long (in milliseconds) to wait for a response before timing out.
    //      -1 means no timeout.
    // The command request and also response is a JSON string.
    int timeout = 5000;  // Timeout in milliseconds
    char* response = 0;
    ret = phoxi_command_execute(request_connect, strlen(request_connect), timeout, callback, &response);
    printf("Response connect: %s\n", response);


    // Switch the device to software trigger mode
    ret = phoxi_command_execute(request_software_trigger_mode, strlen(request_connect), timeout, callback, &response);
    printf("Response trigger_mode: %s\n", response);


    // Get the acquisition status
    // Response JSON string will contain "running" if the acquisition is running
    ret = phoxi_command_execute(request_acquisition_status, strlen(request_acquisition_start), timeout, callback,
                                &response);
    printf("Response acquisition: %s\n", response);
    char* isRunning = strstr(response, "running");
    if (isRunning == NULL) {
        // Start the acquisition if not running
        ret = phoxi_command_execute(request_acquisition_start, strlen(request_acquisition_start), timeout, callback,
                                    &response);
        printf("Response acquisition: %s\n", response);
    }


    // Trigger a frame
    // Read the frame data from the device and process it using the `frameAcceptor` function.
    ret = phoxi_command_execute(request_trigger_frame, strlen(request_trigger_frame), timeout, callback, &response);
    printf("Response trigger_frame: %s\n", response);

    char* frameInfo = 0;
    int frameId = -1;  // Use -1 to get the latest frame
    timeout = 10000;
    ret = phoxi_frame_read("InstalledExamples-basic-example", frameId, timeout, frameAcceptor, &frameInfo);
    if (ret != PHOXI_FRAME_OK) {
        printf("Failed to read frame\n");
        return ret;
    }

    // Clean up the PhoXi C API
    ret = phoxi_exit();
    if (ret != PHOXI_OK) {
        printf("Failed to exit PhoXi API: %d\n", ret);
        return ret;
    }

    // Clean up allocated memory
    if (frameInfo) free(frameInfo);
    if (response) free(response);

    return 0;
}

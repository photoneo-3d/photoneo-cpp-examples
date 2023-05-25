/**
 * Simple example that retrieves frames from a Photoneo device using aravis.
 *
 * Demonstrates
 *   - streaming using either multipart or chunked mode,
 *   - software trigger or freerun,
 *   - enabling / disabling different components.
 *
 * Compile with
 *
 *   gcc -o ConnectAndGrabC main.c -Wall $(pkg-config --libs --cflags  aravis-0.8)
 *
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <arv.h>

void fail(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void checkError(GError* error) {
    if (error) {
        fail(error->message);
    }
}


guint componentCount;
const char** componentNames;
guint32* componentIds;

/**
 * Get all the available components and their IDs.
 *
 * Call this before calling findComponentName and findComponentId.
 */
void getComponentNamesAndIds(ArvCamera* camera, GError **err) {
    assert(camera);
    GError *error;
    componentNames = arv_camera_dup_available_enumerations_as_strings (
            camera,
            "ComponentSelector",
            &componentCount,
            &error);
    if (!componentNames|| componentCount == 0) {
        g_propagate_error(err, error);
        return;
    }

    componentIds = g_new(guint32, componentCount);
    for (guint i = 0; i < componentCount; ++i) {
        arv_camera_set_string(camera, "ComponentSelector", componentNames[i], &error);
        if (error) {
            g_propagate_error(err, error);
            return;
        }

        guint32 idValue = arv_camera_get_integer(camera, "ComponentIDValue", &error);
        if (error) {
            g_propagate_error(err, error);
            return;
        }

        componentIds[i] = idValue;
    }
}

/**
 * Get the name of the component with the given ID.
 */
const char* findComponentName(guint32 id) {
    for (guint i = 0; i < componentCount; ++i) {
        if (componentIds[i] == id) {
            return componentNames[i];
        }
    }
    return "UNKNOWN";
}

/**
 * Get the ID of the given component.
 *
 * Returns the component ID or zero if componenet is not found / available.
 */
guint32 findComponentId(const char* name) {
    for (guint i = 0; i < componentCount; ++i) {
        if (!strcmp(componentNames[i], name)) {
            return componentIds[i];
        }
    }
    return 0x0000;
}

/**
 * Enable or disable the given component.
 *
 * This sets the ComponentEnable feature for the appropriate ComponentSelector:
 *
 *   ComponentSelector = component
 *   ComponentEnable = enabled
 */
void enableComponent(ArvCamera* camera, const char* component, gboolean enabled, GError **err) {
    assert(camera);
    GError* error = NULL;

    arv_camera_set_string(camera, "ComponentSelector", component, &error);
    if (error) {
        g_propagate_error(err, error);
        return;
    }

    arv_camera_set_boolean(camera, "ComponentEnable", enabled, &error);
    if (error) {
        g_propagate_error(err, error);
        return;
    }
}

/**
 * Disable all available components.
 *
 * At least one component should be enabled afterwards.
 */
void disableAllComponents(ArvCamera* camera, GError **err) {
    assert(camera);
    GError* error = NULL;
    for (guint i = 0; i < componentCount; ++i) {
        enableComponent(camera, componentNames[i], FALSE, &error);
        if (error) {
            g_propagate_error(err, error);
            return;
        }
    }
}


void handleChunkDataBuffer(ArvBuffer *buffer, uint32_t width, uint32_t height) {
    assert(buffer);

    ArvBufferPayloadType payloadType = arv_buffer_get_payload_type(buffer);
    if (payloadType == ARV_BUFFER_PAYLOAD_TYPE_IMAGE) {
        printf("IMAGE buffer %s\n", arv_buffer_has_chunks(buffer) ? "(with chunks)" : "");
        // Note: aravis reports the whole frame size here, including chunks.
        // Width, height and pixel format need to be used to select only the appropriate
        // bytes. (arv_buffer_get_image_data introduced in 0.8.25 reports the same, which
        // might be a bug)
        size_t dataSize = 0;
        const void* data = arv_buffer_get_data(buffer, &dataSize);
        if (data && dataSize) {
            fprintf(stdout, "  Image (Intensity) size %zu bytes, %u x %u\n", dataSize, width, height);
        }
    } else {
        printf("CHUNK DATA buffer\n");
    }

    if (arv_buffer_has_chunks(buffer)) {
        for (guint i = 0; i < componentCount; ++i) {
            guint32 chunkId = componentIds[i];
            size_t dataSize = 0;
            const void* data = arv_buffer_get_chunk_data(buffer, chunkId, &dataSize);

            if (!data || dataSize == 0) {
                continue;
            }

            // No metada (width, height, pixel format) for chunks
            fprintf(stdout, "  Chunk %4x %s size %zu bytes\n", chunkId, componentNames[i], dataSize);
        }
    }
}

void handleMultipartBuffer(ArvBuffer *buffer) {
    assert(buffer);
    printf("MULTIPART buffer\n");

    guint nparts = arv_buffer_get_n_parts(buffer);
    for (guint i = 0; i < nparts; ++i) {
        size_t dataSize = 0;
        const void* data = arv_buffer_get_part_data(buffer, i, &dataSize);
        ArvBufferPartDataType dataType = arv_buffer_get_part_data_type(buffer, i);
        guint componentId = arv_buffer_get_part_component_id(buffer, i);
        gint width = arv_buffer_get_part_width(buffer, i);
        gint height = arv_buffer_get_part_height(buffer, i);
        const char* componentName = findComponentName(componentId);

        if (!data || dataSize == 0) {
            continue;
        }

        printf("  Part %d %4x %s type %04x size %zu bytes, %d x %d\n",
                i, componentId, componentName, dataType, dataSize, width, height);

    }
    // Alternatively there's arv_buffer_find_component to find which
    // part contains specific component
    //     guint32 intensityId = findComponentId("Intensity");
    //     guint partNUm = arv_buffer_find_component_id(buffer, intensityId);
}

int main (int argc, char **argv) {
    if (argc < 2) {
        fail("Provide device IP as a parameter!");
    }

    gboolean useMultipart = TRUE;
    gboolean useSoftwareTrigger = FALSE;

    const char* deviceIp = argv[1];
    GError *error = NULL;

    /* Connect to the first available camera */
    ArvCamera* camera = arv_camera_new (deviceIp, &error);
    checkError(error);

    if (!ARV_IS_CAMERA(camera)) {
        fail("Not a camera instance!");
    }

    fprintf(stderr, "Connected to camera: %s\n", arv_camera_get_model_name (camera, NULL));

    ///---------------------------------------------------------------------------------------------

    /* Read frame width */
    uint32_t width = arv_camera_get_integer(camera, "Width", &error);
    checkError(error);
    fprintf(stdout, "Width: %upx\n", width);

    /* Read frame height */
    uint32_t height = arv_camera_get_integer(camera, "Height", &error);
    checkError(error);
    fprintf(stdout, "Height: %upx\n", height);

    getComponentNamesAndIds(camera, &error);
    checkError(error);

    fprintf(stdout, "Components:\n");
    for (guint i = 0; i < componentCount; ++i) {
        fprintf(stdout, "  %04x %s\n", componentIds[i], componentNames[i]);
    }

    ///---------------------------------------------------------------------------------------------

    /* Set trigger mode */
    arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS, &error);
    checkError(error);

    if (useSoftwareTrigger) {
        fprintf(stdout, "Setting up software trigger\n");
        arv_camera_set_trigger(camera, "Software", &error);
        checkError(error);
    } else {
        fprintf(stdout, "Setting up freerun...\n");
        arv_camera_clear_triggers(camera, &error);
        checkError(error);
    }

    /* Create the stream object */
    ArvStream* stream = arv_camera_create_stream (camera, NULL, NULL, &error);
    checkError(error);

    if (!ARV_IS_STREAM(stream)) {
        fail("Not a stream instance!");
    }

    arv_camera_gv_set_packet_size_adjustment (camera, ARV_GV_PACKET_SIZE_ADJUSTMENT_ALWAYS);

    /* Enable required components  BEFORE getting payload size for buffers */
    //disableAllComponents(camera, &error); checkError(error);
    enableComponent(camera, "Intensity", TRUE, &error); checkError(error);
    enableComponent(camera, "Range", TRUE, &error); checkError(error);
    enableComponent(camera, "Confidence", TRUE, &error); checkError(error);
    enableComponent(camera, "Normal", TRUE, &error); checkError(error);
    enableComponent(camera, "Reprojection", TRUE, &error); checkError(error);
    /* Event map is available only on MotionCam */
    if (findComponentId("Event") != 0) {
        enableComponent(camera, "Event", TRUE, &error); checkError(error);
    }
    /*and ColorCamera image is available only on MotionCam Color */
    if (findComponentId("ColorCamera") != 0) {
        enableComponent(camera, "ColorCamera", TRUE, &error); checkError(error);
    }

    /* Choose multipart or chunked data */
    arv_camera_gv_set_multipart(camera, useMultipart, &error);
    checkError(error);

    /* Retrieve the payload size for buffer creation */
    size_t payload = arv_camera_get_payload (camera, &error);
    checkError(error);
    fprintf(stdout,"Payload size: %zu bytes\n", payload);

    /* Insert some buffers in the stream buffer pool */
    for (int i = 0; i < 3; i++) {
        arv_stream_push_buffer(stream, arv_buffer_new(payload, NULL));
    }

    /* Start the acquisition */
    arv_camera_start_acquisition(camera, &error);
    checkError(error);
    fprintf(stdout, "Acquisition started...\n");

    /* Retrieve 10 buffers */
    for (int i = 0; i < 10; i++) {
        if (useSoftwareTrigger) {
            arv_camera_software_trigger(camera, &error);
            checkError(error);
        }

        ArvBuffer* buffer = arv_stream_pop_buffer(stream);
        if (!ARV_IS_BUFFER (buffer)) {
            fail("Buffer is not a buffer instance!");
        }

        ArvBufferStatus status = arv_buffer_get_status(buffer);
        if (status != ARV_BUFFER_STATUS_SUCCESS) {
            fprintf(stderr, "Buffer status: %d\n", status);
            fail("Buffer status is not success");
        }

        ArvBufferPayloadType payloadType = arv_buffer_get_payload_type(buffer);
        switch(payloadType) {
        case ARV_BUFFER_PAYLOAD_TYPE_IMAGE:
        case ARV_BUFFER_PAYLOAD_TYPE_CHUNK_DATA:
            handleChunkDataBuffer(buffer, width, height);
            break;
        case ARV_BUFFER_PAYLOAD_TYPE_MULTIPART:
            handleMultipartBuffer(buffer);
            break;
        default:
            fprintf(stderr, "Error: unsupported buffer type %04x, ignoring\n", payloadType);
            break;
        }

        arv_stream_push_buffer (stream, buffer);
    }

    /* Stop the acquisition */
    arv_camera_stop_acquisition(camera, &error);
    checkError(error);

    fprintf(stdout, "Acquisition stopped...\n");

    g_clear_object(&stream);
    g_clear_object(&camera);

    return 0;
}

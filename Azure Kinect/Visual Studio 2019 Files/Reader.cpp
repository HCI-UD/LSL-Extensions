#include <iostream>
#include "lsl_cpp.h"
#include <k4a\k4a.hpp>
#include <k4abt.h>
#include <time.h>
#include <Windows.h>
#include <cstdio>
#include <mmdeviceapi.h>
#include <Audioclient.h>

#include <Functiondiscoverykeys_devpkey.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

constexpr auto REFTIMES_PER_SEC = 10000000;
constexpr auto REFTIMES_PER_MILLISEC = 10000;

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

int quitter = 0;
int audioDone = 0;

DWORD WINAPI audioThread(LPVOID lpParameter)
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE* pData;
    DWORD flags;
    lsl::stream_info audioInfo;
    lsl::stream_outlet audioOutlet(audioInfo);

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    EXIT_ON_ERROR(hr);

    hr = pCollection->Item(1, &pDevice); //Had to hard-code index. Find the actual index on your own device.
    EXIT_ON_ERROR(hr);

    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr);

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr);

    printf("PWFX->wFormatTag. Expected: WAVE_FORMAT_EXTENSIBLE (fffe) -- Actual: %x\n", pwfx->wFormatTag);
    printf("PWFX->nChannels. Expected: 7 -- Actual: %d\n", pwfx->nChannels);
    printf("PWFX->nSamplesPerSec. Expected: 48000 -- Actual: %d\n", pwfx->nSamplesPerSec);
    printf("PWFX->nAvgBytesPerSec. Expected: 1344000 -- Actual: %d\n", pwfx->nAvgBytesPerSec);
    printf("PWFX->nBlockAlign. Expected: 28 -- Actual: %d\n", pwfx->nBlockAlign);
    printf("PWFX->wBitsPerSample. Expected: 32 -- Actual: %d\n", pwfx->wBitsPerSample);
    printf("PWFX->cbSize. Expected: 22 -- Actual: %d\n", pwfx->cbSize);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
    EXIT_ON_ERROR(hr);
   
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr);

    hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr);

    hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

    audioInfo = lsl::stream_info("Audio Stream", "Audio", 480*28, 100, 
        lsl::channel_format_t::cf_int8, "Audio ID - 5");
    audioOutlet = lsl::stream_outlet(audioInfo);

    hr = pAudioClient->Start();
    EXIT_ON_ERROR(hr);

    while (bDone == FALSE) {
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr);

        while (packetLength != 0 && bDone == FALSE) {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            EXIT_ON_ERROR(hr);
            printf("%d, ", numFramesAvailable);
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                pData = NULL;
                for (int iterator = 0; 0 < numFramesAvailable; iterator += 480) { //480 = packet size in my computer
                    char blanks[480*28];
                    audioOutlet.push_sample(blanks);
                }
            }
            else {
                for (int iterator = 0; iterator < numFramesAvailable; iterator += 480*28) {
                    audioOutlet.push_sample((char*)&pData[iterator]);
                }
            }
            if (quitter != 0) {
                bDone = TRUE;
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr);
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr);
        }
    }
    hr = pAudioClient->Stop();
    EXIT_ON_ERROR(hr);

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pCaptureClient);
    SAFE_RELEASE(pDevice);

    audioDone = 1;
    return hr;
}
void ListEndpoints();

void ListEndpoints()
{
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice* pEndpoint = NULL;
    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    EXIT_ON_ERROR(hr);

    UINT  count;
    hr = pCollection->GetCount(&count);
    EXIT_ON_ERROR(hr);

    if (count == 0)
    {
        printf("No endpoints found.\n");
    }

    // Each iteration prints the name of an endpoint device.
    PROPVARIANT varName;
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        EXIT_ON_ERROR(hr);

        // Get the endpoint ID string.
        hr = pEndpoint->GetId(&pwszID);
        EXIT_ON_ERROR(hr);

        hr = pEndpoint->OpenPropertyStore(
            STGM_READ, &pProps);
        EXIT_ON_ERROR(hr);

        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr);

        // Print endpoint friendly name and endpoint ID.
        printf("Endpoint %d: \"%S\" (%S)\n", i, varName.pwszVal, pwszID);
    }

Exit:
    CoTaskMemFree(pwszID);
    pwszID = NULL;
    PropVariantClear(&varName);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pProps);
}
//Code assumes little endian machine.

void sendData(k4a_device_t device, char* serial_number, k4a_device_configuration_t config, k4abt_tracker_t tracker, k4a_calibration_t* sensor_calibration, bool trackerInTwoD) {
    //Constants.
    float FRAME_RATE;

    int COLOR_HEIGHT = 0;
    int COLOR_WIDTH = 0;
    int COLOR_SIZE = 0;
    lsl::channel_format_t colorType = lsl::channel_format_t::cf_int8;

    int DEPTH_HEIGHT = 0;
    int DEPTH_WIDTH = 0;
    int DEPTH_SIZE = 0;
    lsl::channel_format_t depthType = lsl::channel_format_t::cf_int16;

    int IR_HEIGHT = 0;
    int IR_WIDTH = 0;
    int IR_SIZE = 0;
    lsl::channel_format_t irType = lsl::channel_format_t::cf_int16;;


    //LSL Objects.
    lsl::stream_info colorInfo;
    lsl::stream_outlet colorOutlet(colorInfo);

    lsl::stream_info depthInfo;
    lsl::stream_outlet depthOutlet(depthInfo);

    lsl::stream_info irInfo;
    lsl::stream_outlet irOutlet(irInfo);

    lsl::stream_info skeletonInfo;
    lsl::stream_outlet skeletonOutlet(skeletonInfo);


    //Color image resolution checks.
    switch (config.color_resolution) {
    case K4A_COLOR_RESOLUTION_OFF:
        COLOR_HEIGHT = 0;
        COLOR_WIDTH = 0;
        break;
    case K4A_COLOR_RESOLUTION_720P:
        COLOR_HEIGHT = 720;
        COLOR_WIDTH = 1280;
        break;
    case K4A_COLOR_RESOLUTION_1080P:
        COLOR_HEIGHT = 1080;
        COLOR_WIDTH = 1920;
        break;
    case K4A_COLOR_RESOLUTION_1440P:
        COLOR_HEIGHT = 1440;
        COLOR_WIDTH = 2560;
        break;
    case K4A_COLOR_RESOLUTION_1536P:
        COLOR_HEIGHT = 1536;
        COLOR_WIDTH = 2048;
        break;
    case K4A_COLOR_RESOLUTION_2160P:
        COLOR_HEIGHT = 2160;
        COLOR_WIDTH = 3840;
        break;
    case K4A_COLOR_RESOLUTION_3072P:
        COLOR_HEIGHT = 3072;
        COLOR_WIDTH = 4096;
        break;
    }


    //Color image format checks.
    switch (config.color_format) {
    case K4A_IMAGE_FORMAT_COLOR_NV12:
        COLOR_SIZE = (COLOR_HEIGHT + (COLOR_HEIGHT / 2)) * COLOR_WIDTH;
        break;
    case K4A_IMAGE_FORMAT_COLOR_YUY2:
        COLOR_SIZE = COLOR_HEIGHT * COLOR_WIDTH * 2;
        break;
    case K4A_IMAGE_FORMAT_COLOR_BGRA32:
        COLOR_SIZE = COLOR_WIDTH * COLOR_HEIGHT * 4;
        break;
    default:
        std::cout << "Can't process these kinds of color images" << std::endl;
        return;
        break;
    }


    //Depth image mode checks.
    switch (config.depth_mode) {
    case (K4A_DEPTH_MODE_OFF):
        DEPTH_HEIGHT = 0;
        DEPTH_WIDTH = 0;
        DEPTH_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;

        IR_HEIGHT = 0;
        IR_WIDTH = 0;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;
        break;
    case (K4A_DEPTH_MODE_NFOV_2X2BINNED):
        DEPTH_HEIGHT = 288;
        DEPTH_WIDTH = 320;
        DEPTH_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;

        IR_HEIGHT = 288;
        IR_WIDTH = 320;
        IR_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;
        break;
    case (K4A_DEPTH_MODE_NFOV_UNBINNED):
        DEPTH_HEIGHT = 576;
        DEPTH_WIDTH = 640;
        DEPTH_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;

        IR_HEIGHT = 576;
        IR_WIDTH = 640;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;
        break;
    case (K4A_DEPTH_MODE_WFOV_2X2BINNED):
        DEPTH_HEIGHT = 512;
        DEPTH_WIDTH = 512;
        DEPTH_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;

        IR_HEIGHT = 512;
        IR_WIDTH = 512;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;
        break;
    case (K4A_DEPTH_MODE_WFOV_UNBINNED):
        DEPTH_HEIGHT = 1024;
        DEPTH_WIDTH = 1024;
        DEPTH_SIZE = DEPTH_HEIGHT * DEPTH_WIDTH;

        IR_HEIGHT = 1024;
        IR_WIDTH = 1024;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;
        break;
    case (K4A_DEPTH_MODE_PASSIVE_IR):
        IR_HEIGHT = 1024;
        IR_WIDTH = 1024;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;
        break;
    }


    //Frame per second checks.
    switch (config.camera_fps) {
    case K4A_FRAMES_PER_SECOND_5:
        FRAME_RATE = 5;
        break;
    case K4A_FRAMES_PER_SECOND_15:
        FRAME_RATE = 15;
        break;
    case K4A_FRAMES_PER_SECOND_30:
        FRAME_RATE = 30;
        break;
    }


    //Instantiate LSL Objects.
    if (COLOR_SIZE != 0) {
        colorInfo = lsl::stream_info("Color Stream " + std::string(serial_number),
            "Color Image", COLOR_SIZE, FRAME_RATE, colorType, "Color ID - 01");
        colorOutlet = lsl::stream_outlet(colorInfo);
    }

    if (DEPTH_SIZE != 0) {
        depthInfo = lsl::stream_info("Depth Stream " + std::string(serial_number),
            "Depth Image", DEPTH_SIZE, FRAME_RATE, depthType, "Depth ID - 02");
        depthOutlet = lsl::stream_outlet(depthInfo);
    }

    if (IR_SIZE != 0) {
        irInfo = lsl::stream_info("IR Stream " + std::string(serial_number),
            "IR Image", IR_SIZE, FRAME_RATE, irType, "IR ID - 03");
        irOutlet = lsl::stream_outlet(irInfo);
    }

    if (tracker != NULL) {
        skeletonInfo = lsl::stream_info("Skeleton Stream " + std::string(serial_number),
            "Skeleton Data", 8 * K4ABT_JOINT_COUNT, FRAME_RATE, lsl::channel_format_t::cf_float32, "Skeleton ID - 04");
        skeletonOutlet = lsl::stream_outlet(skeletonInfo);
    }


    //while (true) 
    for (int i = 0; i < 5; i++)
    {
        if (i == 20) {
            float skeletonBlanks[256];
            skeletonBlanks[7] = 6;
            char* colorBlanks = new char[COLOR_SIZE];
            colorBlanks[4] = 2;
            skeletonOutlet.push_sample(skeletonBlanks, 0);
            colorOutlet.push_sample(colorBlanks, 0);
            delete[] colorBlanks;
        }
        //Get capture.
        k4a_capture_t capture;
        switch (k4a_device_get_capture(device, &capture, 2000))
        {
        case K4A_WAIT_RESULT_SUCCEEDED:
            break;
        case K4A_WAIT_RESULT_TIMEOUT:
            printf("Timed out waiting for a capture\n");
            break;
        case K4A_WAIT_RESULT_FAILED:
            printf("Failed to read a capture\n");
            return;
        }


        //Send images.
        if (COLOR_SIZE != 0) {
            k4a_image_t color = k4a_capture_get_color_image(capture);
            printf(" | Color res:%4dx%4d stride:%5d, size:%5d\n",
                k4a_image_get_height_pixels(color),
                k4a_image_get_width_pixels(color),
                k4a_image_get_stride_bytes(color),
                k4a_image_get_size(color));
            if (color != NULL) {
                uint8_t* colorBuffer = k4a_image_get_buffer(color);
                char* colorArray = (char*)colorBuffer;
                colorOutlet.push_sample(colorArray);
            }
            k4a_image_release(color);
        }
        if (DEPTH_SIZE != 0) {
            k4a_image_t depth = k4a_capture_get_depth_image(capture);
            printf(" | Depth res:%4dx%4d stride:%5d, size:%5d\n",
                k4a_image_get_height_pixels(depth),
                k4a_image_get_width_pixels(depth),
                k4a_image_get_stride_bytes(depth),
                k4a_image_get_size(depth));
            if (depth != NULL) {
                uint8_t* depthBuffer = k4a_image_get_buffer(depth);
                int16_t* depthArray = (int16_t*)depthBuffer;
                depthOutlet.push_sample(depthArray);
            }
            k4a_image_release(depth);
        }
        if (IR_SIZE != NULL) {
            k4a_image_t ir = k4a_capture_get_ir_image(capture);
            printf(" | IR res:%4dx%4d stride:%5d, size:%5d\n",
                k4a_image_get_height_pixels(ir),
                k4a_image_get_width_pixels(ir),
                k4a_image_get_stride_bytes(ir),
                k4a_image_get_size(ir));
            if (ir != NULL) {
                uint8_t* irBuffer = k4a_image_get_buffer(ir);
                int16_t* irArray = (int16_t*)irBuffer;
                irOutlet.push_sample(irArray);
            }
            k4a_image_release(ir);
        }


        //Get body tracking data.
        if (tracker != NULL) {
            k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, capture, 0);
            if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
            {
                printf("Error! Adding capture to tracker process queue failed!\n");
                return;
            }
            k4abt_frame_t body_frame = NULL;
            k4a_wait_result_t pop_frame_result = K4A_WAIT_RESULT_TIMEOUT;
            int infinityPreventer = 0;
            while (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT && infinityPreventer < 2) {
                pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, 50);
                infinityPreventer++;
            }
            if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
            {
                size_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
                if (num_bodies != 1) {
                    printf("Error: Wrong number of bodies\n");
                    float jointInfo2[256];
                    skeletonOutlet.push_sample(jointInfo2);
                }
                else
                {


                    //Generate and push LSL sample.
                    for (size_t bodyID = 0; bodyID < num_bodies; bodyID++)
                    {
                        k4abt_skeleton_t skeleton;
                        k4abt_frame_get_body_skeleton(body_frame, bodyID, &skeleton);
                        uint32_t id = k4abt_frame_get_body_id(body_frame, bodyID);

                        float jointInfo[256];
                        for (int joint = 0; joint < K4ABT_JOINT_COUNT; joint++) {
                            if (!trackerInTwoD) {
                                jointInfo[joint * 8] = skeleton.joints[joint].position.xyz.x;
                                jointInfo[joint * 8 + 1] = skeleton.joints[joint].position.xyz.y;
                                jointInfo[joint * 8 + 2] = skeleton.joints[joint].position.xyz.z;
                                jointInfo[joint * 8 + 3] = skeleton.joints[joint].orientation.wxyz.w;
                                jointInfo[joint * 8 + 4] = skeleton.joints[joint].orientation.wxyz.x;
                                jointInfo[joint * 8 + 5] = skeleton.joints[joint].orientation.wxyz.y;
                                jointInfo[joint * 8 + 6] = skeleton.joints[joint].orientation.wxyz.z;
                                jointInfo[joint * 8 + 7] = (float)skeleton.joints[joint].confidence_level;
                            }
                            else {
                                k4a_float2_t* newPos = (k4a_float2_t*)malloc(sizeof(k4a_float2_t));
                                int temp = 0;
                                k4a_result_t result = k4a_calibration_3d_to_2d(sensor_calibration, &skeleton.joints[joint].position, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_COLOR, newPos, &temp);
                                if (result == K4A_RESULT_SUCCEEDED) {
                                    jointInfo[joint * 8] = newPos->xy.x;
                                    jointInfo[joint * 8 + 1] = newPos->xy.y;
                                    jointInfo[joint * 8 + 2] = 0;
                                    jointInfo[joint * 8 + 3] = skeleton.joints[joint].orientation.wxyz.w;
                                    jointInfo[joint * 8 + 4] = skeleton.joints[joint].orientation.wxyz.x;
                                    jointInfo[joint * 8 + 5] = skeleton.joints[joint].orientation.wxyz.y;
                                    jointInfo[joint * 8 + 6] = skeleton.joints[joint].orientation.wxyz.z;
                                    jointInfo[joint * 8 + 7] = (float)skeleton.joints[joint].confidence_level;
                                }
                                else {
                                    printf("Failed\n");
                                }
                                free(newPos);
                            }
                        }
                        skeletonOutlet.push_sample(jointInfo);
                    }
                }


                //Clean-up.
                k4abt_frame_release(body_frame);
            }
            else {
                printf("Dropped body frame\n");
            }
        }
        k4a_capture_release(capture);
    }
}




int main() {
    char* serial_number = NULL;
    size_t serial_number_length = 0;
    k4a_device_t device = NULL;
    uint32_t device_count = k4a_device_get_installed_count();


    //Confirm correct number of devices (1).
    if (device_count != 1)
    {
        printf("Unexpected number of devices found (%d)\n", device_count);
        return -1;
    }


    //Open device.
    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        printf("Failed to open device\n");
        return -1;
    }


    //Get serial number.
    if (K4A_BUFFER_RESULT_TOO_SMALL != k4a_device_get_serialnum(device, NULL, &serial_number_length))
    {
        printf("Failed to get serial number length\n");
        k4a_device_close(device);
        device = NULL;
        return -1;
    }
    serial_number = (char*)malloc(serial_number_length);
    if (serial_number == NULL)
    {
        printf("Failed to allocate memory for serial number (%zu bytes)\n", serial_number_length);
        k4a_device_close(device);
        device = NULL;
        return -1;
    }
    if (K4A_BUFFER_RESULT_SUCCEEDED != k4a_device_get_serialnum(device, serial_number, &serial_number_length))
    {
        printf("Failed to get serial number\n");
        free(serial_number);
        serial_number = NULL;
        k4a_device_close(device);
        device = NULL;
        return -1;
    }
    printf("Device \"%s\"\n", serial_number);


    //Set up device configuration.
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    //config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG; //Code not set up to stream these images because they are of variable size.
    //config.color_format = K4A_IMAGE_FORMAT_COLOR_NV12;
    //config.color_format = K4A_IMAGE_FORMAT_COLOR_YUY2;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;

    //config.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    config.color_resolution = K4A_COLOR_RESOLUTION_720P;
    //config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
    //config.color_resolution = K4A_COLOR_RESOLUTION_1440P;
    //config.color_resolution = K4A_COLOR_RESOLUTION_1536P;
    //config.color_resolution = K4A_COLOR_RESOLUTION_2160P;
    //config.color_resolution = K4A_COLOR_RESOLUTION_3072P;

    //config.depth_mode = K4A_DEPTH_MODE_OFF;
    //config.depth_mode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    //config.depth_mode = K4A_DEPTH_MODE_WFOV_2X2BINNED;
    //config.depth_mode = K4A_DEPTH_MODE_WFOV_UNBINNED;
    //config.depth_mode = K4A_DEPTH_MODE_PASSIVE_IR;

    config.camera_fps = K4A_FRAMES_PER_SECOND_5;
    //config.camera_fps = K4A_FRAMES_PER_SECOND_15;
    //config.camera_fps = K4A_FRAMES_PER_SECOND_30;

    bool wantTracker = true;
    //bool wantTracker = false;

    //bool trackerInTwoD = true;
    bool trackerInTwoD = false;

    bool audio = true;
    //bool audio = false;
    if (config.color_resolution != K4A_COLOR_RESOLUTION_OFF && config.depth_mode != K4A_DEPTH_MODE_OFF)
    {
        config.synchronized_images_only = true;
        printf("true\n");
    }
    else
    {
        config.synchronized_images_only = false;
    }



    //Start device cameras.
    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        printf("Failed to start device\n");
        return -1;
    }


    //Initialize tracker.
    k4a_calibration_t sensor_calibration;
    if (K4A_RESULT_SUCCEEDED != k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &sensor_calibration))
    {
        printf("Get depth camera calibration failed!\n");
        return 0;
    }
    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    if (wantTracker && config.depth_mode != K4A_DEPTH_MODE_OFF && config.depth_mode != K4A_DEPTH_MODE_PASSIVE_IR) {
        if (K4A_RESULT_SUCCEEDED != k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker))
        {
            printf("Body tracker initialization failed!\n");
            return 0;
        }
    }


    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ListEndpoints();

    if (audio) {
        CreateThread(NULL, 0, audioThread, NULL, 0, NULL);
    }
    //Run program and send data.
    sendData(device, serial_number, config, tracker, &sensor_calibration, trackerInTwoD);

    quitter = 1;
    if (audio) {
        while (audioDone == 0);
    }

    //End of program clean-up.
    if (config.depth_mode != K4A_DEPTH_MODE_OFF && config.depth_mode != K4A_DEPTH_MODE_PASSIVE_IR) {
        k4abt_tracker_shutdown(tracker);
        k4abt_tracker_destroy(tracker);
    }
    k4a_device_stop_cameras(device);
    k4a_device_close(device);
    free(serial_number);
}
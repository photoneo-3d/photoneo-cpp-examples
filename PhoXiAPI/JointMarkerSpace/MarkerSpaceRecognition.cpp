#include "MarkerSpaceRecognition.h"

#include "Utils/Scanner.h"
#include "Utils/Util.h"


namespace jointMarkerSpace {

// Set maximum resolution and LED texture source to have the best calibration results
void prepareDevice(pho::api::PPhoXi& device) {
    const bool motionCam = device->GetType() == pho::api::PhoXiDeviceType::MotionCam3D;
    if (motionCam) {
        device->MotionCam->OperationMode = pho::api::PhoXiOperationMode::Scanner;
        device->MotionCamScannerMode->TextureSource = pho::api::PhoXiTextureSource::LED;
    } else {
        const auto& supportedCapturingModes = device->SupportedCapturingModes.GetValue();
        if (!supportedCapturingModes.empty()) {
            device->CapturingMode = supportedCapturingModes[0];
        }
        device->CapturingSettings->TextureSource = pho::api::PhoXiTextureSource::LED;
    }
    device->CoordinatesSettings->CameraSpace = pho::api::PhoXiCameraSpace::PrimaryCamera;
    device->CoordinatesSettings->CoordinateSpace = pho::api::PhoXiCoordinateSpace::MarkerSpace;
    device->CoordinatesSettings->RecognizeMarkers = true;
}

pho::api::PhoXiCoordinateTransformation getTransformation(const pho::api::FrameInfo &info) {
    pho::api::PhoXiCoordinateTransformation result;

    result.Rotation[0][0] = (double)info.CurrentCameraXAxis.x;
    result.Rotation[1][0] = (double)info.CurrentCameraXAxis.y;
    result.Rotation[2][0] = (double)info.CurrentCameraXAxis.z;

    result.Rotation[0][1] = (double)info.CurrentCameraYAxis.x;
    result.Rotation[1][1] = (double)info.CurrentCameraYAxis.y;
    result.Rotation[2][1] = (double)info.CurrentCameraYAxis.z;

    result.Rotation[0][2] = (double)info.CurrentCameraZAxis.x;
    result.Rotation[1][2] = (double)info.CurrentCameraZAxis.y;
    result.Rotation[2][2] = (double)info.CurrentCameraZAxis.z;

    result.Translation = info.CurrentCameraPosition;

    return result;
}

pho::api::PhoXiCoordinateTransformation computeRelativeTransformation(
        const pho::api::PhoXiCoordinateTransformation& primaryToMarker,
        const pho::api::PhoXiCoordinateTransformation& secondaryToMarker) {
    const auto markerToSecondary = utils::invert(secondaryToMarker);
    const auto primaryToSecondary = utils::compose(markerToSecondary, primaryToMarker);
    std::cout << std::endl;
    std::cout << "Computed transformation from primary camera space to secondary:" << std::endl;
    std::cout << "Rotation matrix" << std::endl;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            std::cout << primaryToSecondary.Rotation[row][col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Translation vector" << std::endl;
    std::cout << primaryToSecondary.Translation.x << ", " << primaryToSecondary.Translation.y <<
        ", " << primaryToSecondary.Translation.z << std::endl;

    return primaryToSecondary;
}

void recognizeMarkerAndSetupDevices(
        pho::api::PPhoXi& primaryDevice, pho::api::PPhoXi& secondaryDevice) {
    prepareDevice(primaryDevice);
    prepareDevice(secondaryDevice);

    try {
        const auto primaryFrame = utils::triggerScanAndGetFrame(primaryDevice);
        const auto secondaryFrame = utils::triggerScanAndGetFrame(secondaryDevice);

        const auto primaryToMarker = getTransformation(primaryFrame->Info);
        const auto secondaryToMarker = getTransformation(secondaryFrame->Info);

        const auto primaryToSecondaryTransformation =
            computeRelativeTransformation(primaryToMarker, secondaryToMarker);

        primaryDevice->CoordinatesSettings->CoordinateSpace =
            pho::api::PhoXiCoordinateSpace::CustomSpace;
        primaryDevice->CoordinatesSettings->CustomTransformation =
            primaryToSecondaryTransformation;

        secondaryDevice->CoordinatesSettings->CoordinateSpace =
            pho::api::PhoXiCoordinateSpace::CameraSpace;

        std::cout << "Marker is recognized, and coordinate settings are set for the devices." <<
            std::endl;
    } catch(const std::exception &e) {
        std::cout << "Marker Recognition failed. Exception:" << std::endl;
        std::cout << e.what() << std::endl;
    }

    primaryDevice->CoordinatesSettings->RecognizeMarkers = false;
    secondaryDevice->CoordinatesSettings->RecognizeMarkers = false;
}

} // namespace jointMarkerSpace

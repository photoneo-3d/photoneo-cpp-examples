#include "ReadWriteHelpers.h"

template<typename T>
void gobject_destroyer(T* object) {
    g_clear_object(&object);
}

template<typename T>
auto create_gobject_unique(T* ptr) {
    return std::unique_ptr<T, void(*)(T*)>(ptr, gobject_destroyer);
}

/*
 * Connect to the first available camera, then test available features.
 */
int main (int argc, char **argv) {
    if(argc < 2) {
        std::cerr << "Provide device IP as a parameter!" << std::endl;
        return 1;
    }

    const char* deviceIp = argv[1];

    GError *error = nullptr;

    /* Connect to the first available camera */
    auto camera = create_gobject_unique(arv_camera_new(deviceIp, &error));
    if (error) {
        std::cerr << "Error: " << error->message << std::endl;
        return 1;
    }

    if (!ARV_IS_CAMERA(camera.get())) {
        std::cerr << "Error: Not a camera instance!";
        return 1;
    }

    std::cerr << "Connected to camera: " << arv_camera_get_model_name(camera.get(), nullptr) << std::endl;

    ///-----------------------------------------------------------------------------------------------------------------

    pho::ReadWriteHelper rw(camera.get());

    /* Internal features for device type check */
    rw.testIntegerFeature("IsMotionCam3D_Val");
    rw.testIntegerFeature("IsMotionCam3DColor_Val");
    rw.testIntegerFeature("IsPhoXi3DScanner_Val");
    rw.testIntegerFeature("IsAlphaScanner_Val");
    rw.testIntegerFeature("IsNotAlphaScanner_Val");

    /* GenericSettings */
    rw.testIntegerFeature("Width");
    rw.testIntegerFeature("Height");
    rw.testIntegerFeature("PayloadSize");
    //rw.testCommandFeature("AcquisitionStart");
    //rw.testCommandFeature("AcquisitionStop");
    //rw.testCommandFeature("GevSCPSFireTestPacket");

    /* CapturingSettingsScanner */
    rw.testIntegerFeature("ShutterMultiplier", RW, 5);
    rw.testIntegerFeature("ScanMultiplier", RW, 5);
    rw.testEnumFeature("Resolution", RW, "Resolution_1032x772");
    rw.testBooleanFeature("CameraOnlyMode", RW, true);
    rw.testBooleanFeature("AmbientLightSuppression", RW, true);
    rw.testEnumFeature("CodingStrategy", RW, "Interreflections");
    rw.testEnumFeature("CodingQuality", RW, "High");
    rw.testEnumFeature("TextureSource",RW, "Computed");
    rw.testFloatFeature("SinglePatternExposure", RW, 30.72);
    rw.testFloatFeature("MaximumFPS", RW, 12.34);
    rw.testIntegerFeature("LaserPower", RW, 1024);
    rw.testIntegerFeature("ProjectionOffsetLeft", RW, 222);
    rw.testIntegerFeature("ProjectionOffsetRight", RW, 222);
    rw.testIntegerFeature("LEDPower", RW, 1024);
    rw.testBooleanFeature("HardwareTrigger", RW, true);
    rw.testEnumFeature("HardwareTriggerSignal", RW, "Rising");

    /* CapturingSettingsCamera */
    rw.testEnumFeature("OperationMode", RW, "Camera");
    rw.testEnumFeature("CameraResolution");
    rw.testFloatFeature("CameraExposure", RW, 30.72);
    rw.testEnumFeature("OutputTopology", RW, "Raw");
    rw.testEnumFeature("CameraCodingStrategy", RW, "Interreflections");
    rw.testEnumFeature("CameraTextureSource", RW, "Laser");

    /* ProcessingSettings */
    rw.testFloatFeature("MaxInaccuracy", RW, 12.34);
    rw.testFloatFeature("MaxCameraAngle", RW, 12.34);
    rw.testFloatFeature("MaxProjectorAngle", RW, 12.34);
    rw.testFloatFeature("MaxHalfwayAngle", RW, 12.34);
    rw.testFloatFeature("MaxHalfwayAngle", RW, 12.34);
    rw.testBooleanFeature("CalibrationVolumeOnly", RW, false);
    rw.testEnumFeature("SurfaceSmoothness", RW, "Normal");
    rw.testIntegerFeature("NormalsEstimationRadius", RW, 2);
    rw.testBooleanFeature("InterreflectionsFiltering", RW, false);
    rw.testFloatFeature("InterreflectionFilterStrength", RW, 0.33);
    rw.testEnumFeature("PatternDecompositionReach", RW, "Large");
    rw.testFloatFeature("SignalContrastThreshold", RW, 1111.22);

    /* CoordinatesSettings */
    rw.testEnumFeature("CoordinateSpace", RW, "RobotSpace");
    rw.testBooleanFeature("RecognizeMarkers", RW, true);
    rw.testFloatFeature("MarkerScaleWidth", RW, 0.12);
    rw.testFloatFeature("MarkerScaleHeight", RW, 3.45);

    /* CalibrationSettings */
    rw.testArrayFeature("CameraMatrix", EMPTY_DOUBLE_VECTOR);
    rw.testArrayFeature("DistortionCoefficient", EMPTY_DOUBLE_VECTOR);
    rw.testFloatFeature("FocusLength");
    rw.testFloatFeature("PixelSizeWidth");
    rw.testFloatFeature("PixelSizeHeight");
 
    /* ColorSettings */
    rw.testIntegerFeature("ColorSettings_ISO", RW, 200);
    rw.testFloatFeature("ColorSettings_Exposure", RW, 30.72);
    rw.testIntegerFeature("ColorSettings_ResolutionWidth");
    rw.testIntegerFeature("ColorSettings_ResolutionHeight");
    rw.testFloatFeature("ColorSettings_Gamma", RW, 2.5);
    rw.testBooleanFeature("ColorSettings_WhiteBalanceEnabled", RW, true);
    rw.testStringFeature("ColorSettings_WhiteBalancePreset", RW, "Internal LED (5000 - 5500K)");
    rw.testFloatFeature("ColorSettings_WhiteBalanceR", RW, 0.7);
    rw.testFloatFeature("ColorSettings_WhiteBalanceG", RW, 0.3);
    rw.testFloatFeature("ColorSettings_WhiteBalanceB", RW, 0.55);
    rw.testBooleanFeature("ColorSettings_ComputeCustomWhiteBalance", RW, true);
    rw.testEnumFeature("ColorSettings_Resolution", RO);
    rw.testArrayFeature("ColorSettings_SupportedISOs", EMPTY_INT_VECTOR);
    rw.testArrayFeature("ColorSettings_SupportedExposures", EMPTY_DOUBLE_VECTOR);
    rw.testArrayFeature("ColorSettings_SupportedCapturingModes", EMPTY_STRING_VECTOR);
    rw.testArrayFeature("ColorSettings_SupportedColorWhiteBalancePresets", EMPTY_STRING_VECTOR);

    /* ColorCalibrationSettings */
    rw.testArrayFeature("ColorCalibration_CameraMatrix", EMPTY_DOUBLE_VECTOR);
    rw.testArrayFeature("ColorCalibration_DistortionCoefficient", EMPTY_DOUBLE_VECTOR);
    rw.testFloatFeature("ColorCalibration_FocusLength");
    rw.testFloatFeature("ColorCalibration_PixelSizeWidth");
    rw.testFloatFeature("ColorCalibration_PixelSizeHeight");
    rw.testArrayFeature("ColorCalibration_RotationMatrix", EMPTY_DOUBLE_VECTOR);
    rw.testArrayFeature("ColorCalibration_TranslationVector", EMPTY_DOUBLE_VECTOR);

    return 0;
}

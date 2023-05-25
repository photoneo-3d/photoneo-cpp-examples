#include "ColorPointCloud.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <PhoXiAdditionalCamera.h>
#include <PhoXiOpenCVSupport.h>
#include "Calibration.h"
#include "ExternalCamera.h"
#include "Utils/FileCamera.h"
#include "Utils/Scanner.h"
#include "Utils/Util.h"

namespace externalCamera {

pho::api::Mat2D<pho::api::ColorRGB_16> colorPointCloudTexture(
        pho::api::PFrame frame,
        cv::Mat extCameraImage,
        const pho::api::AdditionalCameraCalibration& calibration) {
    std::cout
            << "The alignment of color texture to the point cloud in progress..."
        << std::endl;
    auto TextureSize = frame->GetResolution();

    // Set the deafult value RGB(0,0,0) of the texture
    cv::Mat cvTextureRGB =
            cv::Mat(TextureSize.Height,
                    TextureSize.Width,
                    CV_8UC3,
                    cv::Scalar(0., 0., 0.));
    // Zero-point
    pho::api::Point3_32f ZeroPoint(0.0f, 0.0f, 0.0f);

    // Parameters of computation-----------------------------------------

    // Set 'trans' matrix == rotation and translation together in 4x4 matrix
    cv::Matx<float, 4, 4> trans = cv::Matx<float, 4, 4>::eye();
    const pho::api::RotationMatrix64f &transformRotation =
                    calibration.CoordinateTransformation.Rotation;
    for (int y = 0; y < transformRotation.Size.Height; ++y) {
        for (int x = 0; x < transformRotation.Size.Width; ++x) {
            trans(y, x) = (float)transformRotation[y][x];
        }
    }
    trans(0, 3) = (float)calibration.CoordinateTransformation.Translation.x;
    trans(1, 3) = (float)calibration.CoordinateTransformation.Translation.y;
    trans(2, 3) = (float)calibration.CoordinateTransformation.Translation.z;

    // Set MCWCMatrix to the inverse of 'trans'
    const cv::Matx<float, 4, 4> MCWCMatrix = trans.inv();

    // Set projection parameters from CameraMatrix of the external camera
    float fx = 0.0, fy = 0.0, cx = 0.0, cy = 0.0;
    if (!calibration.CalibrationSettings.CameraMatrix.Empty()) {
        fx = (float)calibration.CalibrationSettings.CameraMatrix[0][0];
        fy = (float)calibration.CalibrationSettings.CameraMatrix[1][1];
        cx = (float)calibration.CalibrationSettings.CameraMatrix[0][2];
        cy = (float)calibration.CalibrationSettings.CameraMatrix[1][2];
    }

    // Set distortion coefficients of the external camera
    float k1 = 0.0, k2 = 0.0, p1 = 0.0, p2 = 0.0, k3 = 0.0;
    if (calibration.CalibrationSettings.DistortionCoefficients.size() >= 5) {
        k1 = (float)calibration.CalibrationSettings.DistortionCoefficients[0];
        k2 = (float)calibration.CalibrationSettings.DistortionCoefficients[1];
        p1 = (float)calibration.CalibrationSettings.DistortionCoefficients[2];
        p2 = (float)calibration.CalibrationSettings.DistortionCoefficients[3];
        k3 = (float)calibration.CalibrationSettings.DistortionCoefficients[4];
    }

    // Set the resolution of external camera
    int width, height;
    width = calibration.CameraResolution.Width;
    height = calibration.CameraResolution.Height;
    // End of setting the parameters--------------------------------------

    // Loop through the PointCloud
    for (int y = 0; y < frame->PointCloud.Size.Height; ++y) {
        for (int x = 0; x < frame->PointCloud.Size.Width; ++x) {
            // Do the computation for a valid point only
            if (frame->PointCloud[y][x] != ZeroPoint) {

                // Point in homogeneous coordinates
                cv::Matx<float, 4, 1> vertexMC(frame->PointCloud[y][x].x,
                                               frame->PointCloud[y][x].y,
                                               frame->PointCloud[y][x].z,
                                               1.0f);

                // Perform the transformation into the coordinates of external camera
                cv::Matx<float, 4, 1> vertexWC = MCWCMatrix * vertexMC;

                // Projection from 3D to 2D
                cv::Point_<float> camPt(vertexWC(0, 0) / vertexWC(2, 0),
                                        vertexWC(1, 0) / vertexWC(2, 0));

                // The distortion of the external camera need to be taken into account for details see e.g.
                // https://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
                float xx = camPt.x * camPt.x;
                float xy2 = 2 * camPt.x * camPt.y;
                float yy = camPt.y * camPt.y;

                float r2 = xx + yy;
                float r4 = r2 * r2;
                float r6 = r4 * r2;

                // Constant related to the radial distortion
                float c = 1 + k1 * r2 + k2 * r4 + k3 * r6;

                // Both radial and tangential distortion are applied
                cv::Point_<float> undistorted(c * camPt.x + p1 * xy2 + p2 * (r2 + 2 * xx),
                                              c * camPt.y + p1 * (r2 + 2 * yy) + p2 * xy2);

                // Final film coordinates
                const cv::Point_<float> position(undistorted.x * fx + cx,
                                                 undistorted.y * fy + cy);

                //(i,j) -> screen space coordinates
                int i = (int)std::round(position.x);
                int j = (int)std::round(position.y);

                if (i >= 0 && i < width && j >= 0 && j < height) {
                    // The loaded extCameraImage has channels ordered like BGR
                    auto yr = cvTextureRGB.ptr<uint8_t>(y);
                    auto jr = extCameraImage.ptr<uint8_t>(j);

                    // Set R - 0th channel
                    yr[3 * x + 0] = jr[3 * i + 2];
                    // Set G - 1st channel
                    yr[3 * x + 1] = jr[3 * i + 1];
                    // Set B - 2nd channel
                    yr[3 * x + 2] = jr[3 * i + 0];
                }
            }
        }
    }

    pho::api::Mat2D<pho::api::ColorRGB_16> textureRGB(TextureSize);
    ConvertOpenCVMatToMat2D(cvTextureRGB, textureRGB);
    return textureRGB;
}

void saveColorPointCloud(
        const std::string& path,
        const pho::api::PFrame frame) {
    if (!frame->SaveAsPly(path, true, true))
        throw std::runtime_error(
                "Failed saving point cloud with color texture to file " + path);

    std::cout << "Point cloud with color texture saved to " << path << std::endl;
}

cv::Mat loadExternalCameraImage(const std::string& path)
{
    auto image = cv::imread(path, cv::IMREAD_COLOR);
    if (image.empty())
        throw std::runtime_error("Error loading image " + path);

    return image;
}

void colorPointCloudFromFile(
        pho::api::PhoXiFactory& factory,
        int argc,
        char* argv[]) {
    std::vector<std::string> prawNames;
    std::string extImagePath =
        utils::Path::join(utils::Path::dataFolder(), "1.bmp");
    std::string outputPath = "fileCamera.ply";

    if (argc > 0)
        prawNames.push_back(argv[0]);

    if (argc > 1)
        extImagePath = argv[1];

    if (argc > 2)
        outputPath = argv[2];

    // Use Data/1.praw if no praw file specified
    if (prawNames.empty())
        prawNames.push_back(
            utils::Path::join(utils::Path::dataFolder(), "1.praw"));

    // Load calibration info
    auto calibration = loadCalibration();
    printCalibration(calibration);

    // Attach praw file as FileCamera
    utils::AttachedFileCamera fileCamera{factory, prawNames};
    auto device = fileCamera.connect();

    // Acquire a frame and load a corresponding image from ext. camera
    pho::api::PFrame frame = utils::triggerScanAndGetFrame(device);
    auto extImage = loadExternalCameraImage(extImagePath);

    // Add the calculated color texture
    frame->TextureRGB = colorPointCloudTexture(frame, extImage, calibration);
    saveColorPointCloud(outputPath, frame);

    // Log out the device from PhoXi Control
    device->Disconnect(true);
}

void colorPointCloudInteractive(
        pho::api::PhoXiFactory& factory) {
    std::string filePrefix = "device_";
    int count = 0;

    // Load calibration info
    auto calibration = loadCalibration();
    printCalibration(calibration);

    // Connect to a scanner
    auto device = utils::selectAndConnectDevice(factory);

    // Iinitialize external camera
    auto extCamera = ExternalCamera();

    auto shouldContinue = []() {
        return  1 == utils::ask<int>("Do you want to continue?", {
            {1, "Yes, trigger new scan and project the color texture"},
            {2, "No, disconnect the scanner"}
        });
    };

    while (shouldContinue()) {
        // Acquire a frame and load a corresponding image from ext. camera
        auto frame = utils::triggerScanAndGetFrame(device);
        auto extImage = extCamera.getColorImage();

        // Add the calculated color texture
        frame->TextureRGB = colorPointCloudTexture(frame, extImage, calibration);

        auto outputPath = filePrefix + std::to_string(++count) + ".ply";
        saveColorPointCloud(outputPath, frame);
    }

    utils::disconnectOrLogOut(device);
}

} // namespace externalCamera

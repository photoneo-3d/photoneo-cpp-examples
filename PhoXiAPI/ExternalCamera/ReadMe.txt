========================================================================
    CONSOLE APPLICATION : ExternalCameraExample Project Overview
========================================================================

ExternalCameraExample is a console application which is used to compute 
calibration for external camera and to get the aligned depth map from the 
point of view of external camera.

You will learn how to:

* calibrate the Scanner to work with an external 2D camera,
* obtain depth map aligned from the point of view of the external camera,
* align color texture from external camera with point cloud.

Prerequisites
-------------

- Download external_camera_example_1.2.zip from https://photoneo.com/files/installer/PhoXi/api/external_camera_example_1.2.zip
and extract it to ExternalCameraExample_CPP folder. This folder 
contains all needed data for testing of calibration using file camera.
You need to have the Data folder present in ExternalCameraExample_CPP folder
before running CMake
- Make sure you have opencv 3.1.0 installed on correct path:
  - Windows: C:/opencv/
  - Linux: home/OpenCV/
  - Or modify CMake if you have it installed on different path
  - You can also change opencv version on your own risk
- CMake ExternalCameraExample



Running the example application
-------------------------------

The application can be either used in a "batch" mode with requested operation
and file passed as commandline arguments or in an "interactive" mode.

Batch mode
----------

Batch mode can be used to either calibrate the external camera based
on prepared scanner frames and external camera images or to calculate
the depth map / color point cloud from saved  praw files / external camera images.

See interactive mode below for explanation of the operations.


### Calibration:

    ./ExternalCameraExample --calibrate [full path to praw file] frames... cameraImages...

There must be an equal number of (paired) frames  and external camera images.
Frames have a prefix of ‘frame’. Images have a prefix of ‘image’.
After successfully loading frames and images the application will connect to
a file camera included in the project.
When the connection to the file camera is successful, the application will
start the calibration process.

### Depth map calculation

    ./ExternalCameraExample --depthmap [full path to praw file] [output file]

### Color point cloud calculation

    ./ExternalCameraExample --colorpc [full path to praw file] [camera image] [output file]

If depth map or color point cloud calculations are invoked without any parameters,
the exmample files in Data directory will be used.


Interactive mode
----------------

Application asks whether you want to test the calibration process or compute the
aligned depth map. Press 1 for calibration, press 2 for depth map, press 3 for
point cloud with color or press 4 to exit the application.


Calibration
-----------

Before using this option you need to implement the ExternalCamera::getCalibrationImage
method in the code. This method must be implemented by you to get a greyscale
image from the external camera.

If you choose calibration: Focal length, Pixel size and Markers positions will
be loaded into Calibration Settings.
By default the 'Settings' folder is set to {PROJECT_DIRECTORY}.
If you want to change the values, you can do it in:
{Settings}/FocalLength.txt
{Settings}/PixelSize.txt
{Settings}/MarkersPositions.txt

Important! Write Focal length and pixel size in millimeters.

In MarkersPositions.txt write markers positions (all the marker patterns with
their corresponding marker positions can be accessed directly from Phoxi
Control (starting with version 1.2.13), in the menu under Tools and then folder
patterns_with_metadata or they can be downloaded from
https://www.photoneo.com/dl/markerpatternsandmetadata-REV-23A).


Connect to a scanner:

The application will show a list of available scanners and prompt the user to 
connect to one. You can connect to a scanner by pressing the corresponding key.


After successful connection the application will ask you whether you want to
trigger a scan. Press 1 to trigger a scan or 2 to stop the process.
When the scan arrives, it will be saved to calibration settings and application
will make a call to getCalibrationImage,  which will get a greyscale image
from the external camera (remember that you need to implement this method yourself).
You can repeat the process by pressing 1 and additional scan will be triggered
and image from external camera will be retrieved. The process needs at least 5
scans to compute the calibration.

When you press 2 the calibration will automatically start.

The result of calibration can be found in:
{PROJECT_FOLDER}/calibration.txt
The structure of calibration.txt is as follows:
  Camera Matrix – 9 double values separated with whitespace
  Distortion Coefficients – 5 to 14 double values separated with whitespace
  Rotation Matrix – 9 double values separated with whitespace
  Translation Vector – 3 double values separated with whitespace
  Camera Resolution – 2 values separated with whitespace (width, height)


Depth Map
---------

If you chose to calculate depth map, Calibration settings will be loaded from
calibration.txt
Loaded settings will be printed out before computing the depth map.

The application will show a list of available scanners and prompt the user to
connect to one. You can connect to a scanner by pressing the corresponding key.

After successful connection to scanner the application will ask you whether
you want to trigger a scan. Press 1 to trigger a scan or 2 to stop the process.
When the scan arrives the aligned depth map will be calculated and saved in the
project folder as device_1.tif.

The application will ask whether you want to continue the process. You can
continue the process by pressing 1.
The next aligned depth maps will be saved as:
{PROJECT_FOLDER}/device_2.tif, {PROJECT_FOLDER}/device_3.tif and so on.


Color Point Cloud
-----------------

Before using this option you need to implement the ExternalCamera::getColorImage
method in the code.

Calibration settings will be loaded from calibration.txt
and printed out before aligning the color texture to the pointcloud.

The application will show a list of available scanners and prompt the user to
connect to one. You can connect to a scanner by pressing the corresponding key.

After successful connection to scanner, the application will trigger a scan.
When the scan arrives, the application will make a call to ExternalCamera::getColorImage
which will get, this time, a color image from the external camera (remember 
that you need to implement this method yourself).

When the color image is retrieved, the application continues with the alignement of 
the color texture with the point cloud. The result is then saved to {PROJECT_FOLDER}/device_1.ply.
The application will ask whether you want to continue the process. 
You can continue the process by pressing 1. 
The next point cloud with color texture will be saved as:
{PROJECT_FOLDER}/device_2.ply, {PROJECT_FOLDER}/device_3.ply and so on.

/////////////////////////////////////////////////////////////////////////////

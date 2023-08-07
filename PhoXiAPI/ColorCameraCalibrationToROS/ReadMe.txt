========================================================================
    CONSOLE APPLICATION : ColorCameraCalibrationToROS Project Overview
========================================================================

This is a simple application that prints out ColorCamera ROS compatibe yaml frame calibration.
This is calibration for raw RGB image respecting currently set color resolution.
Note that this is different from device computed RGB texture.

You will learn how to:

* obtain ROS compatible ColorCamera calibration file

How to build:

1. Copy ColorCameraCalibrationToROS folder to a location with Read and Write
   permissions (using the name <source>)
2. Open CMake
   2.1. Set Source code to <source>
   2.2. Set Binaries to <source>/_build or any other writable location
   2.3. Click Configure and Generate
3. Build project

How to use:
1. Set camera parameters (ideally with ROS driver)
2. Run PhoXiControl
   2.1. Connect to a scanner
   2.2. Make sure `Structure->ColorCameraImage` transfer is enabled
   2.3. Make sure `ColorSettings->Resolution` is as desired
3. Run ColorCameraCalibrationToROS application and redirect output to yaml file
  3.1 e.g. `./ColorCameraCalibrationToROS > MyColorCalibration.yaml`

The application will print out frame level calibration params of ColorCamera of the connected scanner.
If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

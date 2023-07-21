========================================================================
    CONSOLE APPLICATION : FrameCalibrationToROS Project Overview
========================================================================

This is a simple application that prints out ROS compatibe yaml frame calibration.

You will learn how to:

* obtain ROS compatible calibration file

How to build:

1. Copy FrameCalibrationToROS folder to a location with Read and Write
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
   2.2. Make sure `OutputTopology == Regular grid` as this is required for optical model
3. Run FrameCalibrationToROS application and redirect output to yaml file
  3.1 e.g. `./FrameCalibrationToROS > MyCalibration.yaml`

The application will print out frame level calibration params of the connected scanner.
If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

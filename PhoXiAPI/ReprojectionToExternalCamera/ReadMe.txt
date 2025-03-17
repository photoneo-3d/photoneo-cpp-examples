===================================================================================================
    CONSOLE APPLICATION: ReprojectionToExternalCameraExample Project Overview
===================================================================================================

The ReprojectionToExternalCameraExample is a console application designed to compute
a calibration of an external camera and to obtain a scan from a main device,
reprojected from the perspective of the external camera. For simplicity, this example
is tailored to work with two Photoneo devices (one as the main camera and the other
as the external camera), but this configuration can be modified in the code.

You will learn how to:

* calculate the external camera parameters required to reproject the depth map from
  a Photoneo device as viewed from the external camera's perspective,
* setup on-device re-projection feature on the main Photoneo device and obtain a reprojected
  depth map which can be used to map depth and data directly from the external camera.


Prerequisites
-------------

- 2 Photoneo devices
  - both static or mechanically bound together
- marker pattern
  - printed marker pattern
  - text file with marker positions
  - (details in Calibration section)


Running the example application
-------------------------------

The application is controlled by entering numbers from a displayed menu
(no command-line arguments are required).

The application has three modes. At the start, it asks what mode you want to use.
Press 1 for calibration, 2 for scanning, or 3 to exit.


# 1 - Calibration

Before calibration, you need to place a 'MarkersPositions.txt' file in a 'Settings' folder,
which should be placed in {PROJECT_FOLDER}. Write marker positions to this file.
All the marker patterns with their corresponding marker positions can be accessed directly
from Phoxi Control (starting with version 1.2.13), in the menu under Tools and then the folder
patterns_with_metadata, or they can be downloaded from
https://www.photoneo.com/dl/markerpatternsandmetadata-REV-23A.

If you have the file in its place, you can connect to two scanners:
the first is called the "main device," and the second is called the "external."

Connect to a scanner:

The application will show a list of available scanners and prompt the user to connect
to the main one and then to the external one. You can connect to a scanner by entering
the corresponding number.

After successfully connecting to both scanners, the application will ask you whether you want
to trigger a scan. Press 1 to trigger a scan or 2 to stop the process.
When the scan from both scanners arrives, it will be stored in calibration settings.
You can repeat the process by pressing 1. The additional pair of scans will be triggered
and stored. To compute the calibration, at least five scans from each device are needed.
You should randomly rotate the physical marker board between all pairs of scans.

When you press 2, the calibration will automatically start.

This process does not need to be repeated since the calibration is stored in a file
that can be found in:
{PROJECT_FOLDER}/calibration.txt

The structure of the 'calibration.txt' file is as follows:
  Camera Matrix – 9 double values separated with whitespace
  Distortion Coefficients – 5 to 14 double values separated with whitespace
  Rotation Matrix – 9 double values separated with whitespace
  Translation Vector – 3 double values separated with whitespace
  Camera Resolution – 2 values separated with whitespace (width, height)


# 2 - Reprojection

If you choose the reprojection, the calibration settings will be loaded from
the 'calibration.txt' file. Loaded settings will be printed out before scanning.

The application will show a list of available scanners and prompt the user to choose one.
You can connect to a scanner by entering the corresponding number. Choose a scanner
that was calibrated as the main device.

After successful connection to the scanner, the application asks whether you want
to trigger a scan. Press 1 to trigger a scan or 2 to stop the process. After triggering
a scan, you can see it in the PhoXiControl, and the application asks if you want
to continue the process (by pressing 1) or stop it (pressing 2).

///////////////////////////////////////////////////////////////////////////////////////////////////

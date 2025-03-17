===================================================================================================
    CONSOLE APPLICATION: JointMarkerSpaceExample Project Overview
===================================================================================================

The JointMarkerSpaceExample is a console application designed to set up two Photoneo devices
to have their output point cloud in the same coordinate space, using marker recognition
on both devices.

Prerequisites
-------------

- 2 Photoneo devices
  - both static or mechanically bound together
- marker pattern
  - printed marker pattern
  - (details in Marker recognition section)

Running the example application
-------------------------------

The application is controlled by entering numbers from a displayed menu
(no command-line arguments are required).

The application asks what devices you want to connect to, tries to connect
to them, and then has three actions to do with them. Press 1 for marker recognition,
2 for scanning, or 3 for exit. The application is designed to run the marker recognition just once
as a preparation step, and then the scanning can be done as many times as you want.

1 - Marker space recognition
----------------------------

The application triggers a scan on both connected devices, enabling the Recognize Markers setting.
If both devices recognize the marker successfully, the application gets a camera to marker
transformation matrices from them. Then, the application computes a relative transformation
from the primary to the secondary device's camera space, prints the matrix, and sets it
to the primary device's coordinate space as a custom transformation.
The secondary device's coordinate space will be set to its primary camera.
Now, the devices are ready for scanning - You can trigger scans directly in PhoXiControl,
or use step 2 of this application.

2 - Scanning
------------

Trigger a scan on both devices and store point clouds from them to the project folder
as 'point_cloud_primary_1.ply' and 'point_cloud_secondary_1.ply'. This step assumes
that the marker recognition (step 1) was performed with the devices.

The application also counts the exported point cloud pairs, so for the next runs,
they will be saved as 'point_cloud_primary_2.ply' and 'point_cloud_secondary_2.ply',
and the number at the end of the file name will go on.

///////////////////////////////////////////////////////////////////////////////////////////////////

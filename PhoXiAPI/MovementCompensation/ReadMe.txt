========================================================================
    CONSOLE APPLICATION : MovementCompensation Example Project Overview
========================================================================

MovementCompensation example is a console application which demonstrates
use of the EventMap to compensate point cloud distortion due to object motion
during the acquisition.
The compensation requires that:
- the frame was acquired by a MotionCam in the Camera mode,
- the velocity vector of the object is known.

You will learn how to:

* how to compensate the motion distortion of the point cloud.

Prerequisites
-------------

- Download movement_compensation_example_1.0.zip from 
https://photoneo.com/files/installer/PhoXi/api/movement_compensation_example_1.0.zip
and extract it to MovementCompensationExample_CPP folder. This folder 
contains the data needed to run this example:
  - original.praw,
  - velocity.txt.
- Or use command cmake -DPHO_DOWNLOAD_API_EXAMPLES_DATA=ON to automatically download
  data using cmake build
/////////////////////////////////////////////////////////////////////////////

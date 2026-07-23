========================================================================
    CONSOLE APPLICATION : ApplyCustomProjectionExample Project Overview
========================================================================

This is a simple example of how to change camera space to custom space
and reproject depth map to this custom space and save it.

How to build:

1. Copy the ApplyCustomProjection folder to a location with Read and Write permissions (using the name <source>)
2. Install OpenCV (CMakeLists.txt does not pin a specific version)
3. Edit CMakeLists.txt file to set up paths to your OpenCV folder via the OPEN_CV_PATH cache variable
   3.1 windows
       - default path is C:/opencv/build
       - if your path is diferent replace "C:/opencv/build" with your correct path
   3.2 linux
       - default path is $ENV{HOME}/OpenCV ("user home folder"/OpenCV)
       - if your path is diferent replace "$ENV{HOME}/OpenCV" with your correct path
4. Open CMake
   4.1. Set Source code to <source>
   4.2. Set Binaries to <source>/_build or any other writable location
   4.3. Click Configure and Generate
5. Build project
6. Run PhoXiControl
7. Run ApplyCustomProjectionExample application
	7.1. Select scanner via commandline by writting its serialnumber
8. Uncomment Your scanner model in function "ChangeCameraAngleExample"
The application saves two reprojected images of depth map.

/////////////////////////////////////////////////////////////////////////////

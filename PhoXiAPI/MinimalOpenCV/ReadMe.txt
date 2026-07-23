========================================================================
    CONSOLE APPLICATION : MinimalOpenCVExample Project Overview
========================================================================

This is a simple example of how to find and connect an available PhoXi Device
and then convert point clouds from Photoneo format to OpenCV format.

You will learn how to:

* use OpenCV in a project with PhoXi API,
* convert scanned frame into OpenCV format.

How to build:

1. Copy the MinimalOpenCV folder to a location with Read and
   Write permissions (using the name <source>)
2. Install OpenCV (CMakeLists.txt does not pin a specific version)
3. Edit CMakeLists.txt file to set up paths to your OpenCV folder via the
   OPEN_CV_PATH cache variable
   3.1 windows
       - default path is C:/opencv/build
       - if your path is diferent replace "C:/opencv/build" with your correct
         path
   3.2 linux
       - default path is $ENV{HOME}/OpenCV ("user home folder"/OpenCV)
       - if your path is diferent replace "$ENV{HOME}/OpenCV" with your correct
         path
4. Open CMake
   4.1. Set Source code to <source>
   4.2. Set Binaries to <source>/_build or any other writable location
   4.3. Click Configure and Generate
5. Build project
6. Run PhoXiControl
   6.1. Connect to a scanner
7. Run MinimalOpenCVExample application
8. Add your code to function "convertToOpenCV" or call your code from this
   function

The application will print out basic info about converted Point Cloud.
If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

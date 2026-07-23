========================================================================
    CONSOLE APPLICATION : MinimalPclExample Project Overview
========================================================================

This is a simple example of how to find and connect an available PhoXi Device
and then convert point clouds from Photoneo format to PCL format.

You will learn how to:

* use Point Cloud Library in your project with PhoXi API,
* convert scanned frame into PCL format.

How to build:

1. Copy the MinimalPcl folder to a location with Read and Write
   permissions (using the name <source>)
2. Install the Point Cloud Library (PCL) so that it is discoverable by CMake's
   find_package(PCL) (e.g. set PCL_DIR, or install it to a standard location)
3. Open CMake
   3.1. Set Source code to <source>
   3.2. Set Binaries to <source>/_build or any other writable location
   3.3. Click Configure and Generate
4. Build project
5. Run PhoXiControl
   5.1. Connect to a scanner
6. Run MinimalPclExample application

CMakeLists.txt links against the PCL::PCL imported target when available,
falling back to PCL_COMMON_LIBRARIES otherwise, so no manual project property
setup (include/library paths, explicit .lib names) is required.

/////////////////////////////////////////////////////////////////////////////

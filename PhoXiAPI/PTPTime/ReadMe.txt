========================================================================
    CONSOLE APPLICATION : PTP Time Overview
========================================================================

This is a simple application that shows how to manipulate with the PTP synchronized time.

You will learn how to:

* Get the start acquisition time from the received frame
* Check if time is valid
* Print time in desired format
* Calculate duration between two time points with usage of standard std::chrono function

How to build:

1. Copy Profiles folder to a location with Read and Write
   permissions (using the name <source>)
2. Open CMake
   2.1. Set Source code to <source>
   2.2. Set Binaries to <source>/_build or any other writable location
   2.3. Click Configure and Generate
3. Build project
4. Run PhoXiControl
   4.1. Connect to a device
5. Run the application

The application will trigger two frames on one device and print start acquisition
time of both, then calculate duration between frames and print it in milliseconds.

If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

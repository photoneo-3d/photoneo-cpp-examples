========================================================================
    CONSOLE APPLICATION : Recording Project Overview
========================================================================

This is a simple application that setups frame recording for connected device.

You will learn how to:

* Start and stop recording
* Check if recording is running
* Setup recording container for recording into desired (supported) format

How to build:

1. Copy Profiles folder to a location with Read and Write
   permissions (using the name <source>)
2. Open CMake
   2.1. Set Source code to <source>
   2.2. Set Binaries to <source>/_build or any other writable location
   2.3. Click Configure and Generate
3. Build project
4. Run PhoXiControl
   4.1. Connect to a scanner
5. Run Recording application

The application will check if auto-enabled recording is configured and stop
it, setup options for recording into PLY container, trigger one frame and stop
recording. For other supported formats and available options, please see JSON
schema in: {PhoXiControl install directory}/API/RecordingOptionsSchema.json
If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

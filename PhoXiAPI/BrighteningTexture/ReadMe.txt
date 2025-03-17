========================================================================
   CONSOLE APPLICATION : Brightening Texture Example Project Overview
========================================================================

This is a simple application that shows way how to brighten / normalize
textures obtainable from the frame using min and max values.

You will learn how to:

* Find min and max values of the texture
* Normalize texture based on min and max values

How to build:

1. Open CMake
   1.1. Set Source code to <source>
   1.2. Set Binaries to <source>/_build or any other writable location
   1.3. Click Configure and Generate
2. Build project
3. Run PhoXiControl
   3.1. Connect to a scanner
4. Run example application

The application will set up device to obtain the texture from frame, trigger
one frame, find min and max values of the texture using two methods and print
those values. Then texture is normalized using min and max values.
If not connected to any scanner, it will automatically connect to the first
one in PhoXiControl.

/////////////////////////////////////////////////////////////////////////////

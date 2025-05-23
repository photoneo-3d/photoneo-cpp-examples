========================================================================
    CONSOLE APPLICATION : MarkerDotCorrection Project Overview
========================================================================

This is an example usage of the marker dot correction feature.

You will learn how to:

* make a marker dot reference scan,
* apply the marker dot correction onto a test scan,
* monitor displacements of the recognized marker dots.

How to build:

1. Copy MarkerDotCorrection folder to a location with Read and Write
   permissions (using the name <source>)
2. Open CMake
   2.1. Set Source code to <source>
   2.2. Set Binaries to <source>/_build or any other writable location
   2.3. Click Configure and Generate
3. Build project
4. Run PhoXiControl
   4.1. Connect to a scanner
5. Run MarkerDotCorrection application

If not connected to any scanner, it will automatically connect to the first
in PhoXiControl.
The application will print out the status of reference recording and of the
test scan. It will also print displacements of all marker dots identified
both in the test scan and in the reference scan.

/////////////////////////////////////////////////////////////////////////////

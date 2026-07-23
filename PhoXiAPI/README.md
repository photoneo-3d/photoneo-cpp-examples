# PhoXi API examples

This folder contains C++ (and one C) example applications that integrate with
Photoneo 3D scanners through the **PhoXi API**, Photoneo's native SDK. All
examples connect to devices through **PhoXi Control**, which must be
installed and running.

For the alternative, PhoXi-Control-free integration path over raw GigE
Vision, see [`GigEV/README.md`](../GigEV/README.md).

## Prerequisites

- CMake 3.10 or newer.
- [PhoXi Control](https://www.photoneo.com/kb/pxc) installed, so that
  `find_package(PhoXi REQUIRED CONFIG ...)` can locate the API. Point CMake at
  it via either:
  - the `PHO_API_CMAKE_CONFIG_PATH` CMake variable, or
  - the `PHOXI_CONTROL_PATH` environment variable (the default fallback used
    by every example).
- A PhoXi device connected and visible in PhoXi Control, or
  the bundled file camera for examples that support it.

## Building an example

Each subfolder below is an independent, self-contained CMake project (there is
no shared top-level build). To build one:

```
cmake -S PhoXiAPI/<ExampleName> -B build
cmake --build build
```

On Windows/MSVC, the PhoXi DLL is copied next to the built executable
automatically as a post-build step (override with `PHOXI_DLL_FOR_EXAMPLE`).
On Linux, examples link against `rt` and compile with `-pthread`.

Some examples pull in extra dependencies on top of the PhoXi API — check the
example's own `ReadMe.txt` and `CMakeLists.txt` first:

- `MinimalOpenCV`, `ApplyCustomProjection`, `ExternalCamera` — OpenCV (path
  via `OPEN_CV_PATH`, defaults to `C:/opencv/build` on Windows / `$HOME/OpenCV`
  on Linux).
- `MinimalPcl` — Point Cloud Library (`find_package(PCL)`).
- `TwoScannersMultithread` — `Threads`.
- `CAPI` — written in plain C.

Shared helper code (device-connection checks, calibration helpers, file-camera
helpers) lives in per-example `Utils/` folders (e.g. `Utils/`,
`ExternalCamera/Utils/`, `MovementCompensation/Utils/`,
`JointMarkerSpace/Utils/`, `ReprojectionToExternalCamera/Utils/`) rather than
one shared library — they are near-duplicates of each other by design, since
every example folder is meant to be copied out and modified independently.

## Examples

| Example | Description |
| --- | --- |
| [ConnectAndGrab](ConnectAndGrab) | Find and connect to an available PhoXi device and capture scans a few different ways. Good starting point. |
| [FullAPI](FullAPI) | Full-featured tour of the API: discovery, connection, free-run/manual capture, output formats, settings, clean disconnect. |
| [CAPI](CAPI) | Use the PhoXi API through its plain C interface instead of C++. |
| [ChangeSettings](ChangeSettings) | Retrieve and set capturing, processing, and coordinates settings. |
| [ChangeColorSettings](ChangeColorSettings) | Retrieve and set color camera settings. |
| [MotionCamChangeSettings](MotionCamChangeSettings) | Retrieve and set MotionCam-3D specific capturing/processing/coordinate settings. |
| [ReadPointCloud](ReadPointCloud) | Multiple ways to handle acquired point cloud data and save scans to different file formats. |
| [PointCloudCalculation](PointCloudCalculation) | Calculate a point cloud from a depth map. |
| [Recording](Recording) | Start/stop frame recording and configure the recording container/format. |
| [BrighteningTexture](BrighteningTexture) | Find min/max texture values and normalize (brighten) the texture from a frame. |
| [EarlyTransfer](EarlyTransfer) | Use the Early Transfer feature to receive texture data as a separate, earlier frame ahead of the full 3D data. |
| [GetProfiles](GetProfiles) | List, get/set, import/export, create/delete, and update device profiles. |
| [GetTemperatures](GetTemperatures) | Read sensor temperature values from a connected scanner. |
| [GetISCalibParams](GetISCalibParams) | Read image sensor calibration parameters from a connected scanner. |
| [PTPTime](PTPTime) | Work with PTP-synchronized acquisition timestamps and calculate inter-device frame delays. |
| [MaintenanceCommands](MaintenanceCommands) | Use PhoXiFactory maintenance commands: reboot, shutdown, factory reset. |
| [AutonomousMaintenance](AutonomousMaintenance) | Check and improve the consistency of the device calibration. |
| [MarkerDotCorrection](MarkerDotCorrection) | Record a marker dot reference scan and apply marker dot correction, monitoring dot displacements. |
| [ApplyCustomProjection](ApplyCustomProjection) | Change camera space to a custom space and reproject the depth map into it. |
| [RotatedCalibration](RotatedCalibration) | Calibrate scanning on a rotary table by recording marker-space transformations for several rotations. |
| [MovementCompensation](MovementCompensation) | Compensate point cloud distortion caused by object motion during acquisition (MotionCam Camera mode). |
| [ExternalCamera](ExternalCamera) | Calibrate an external 2D camera against the scanner and reproject depth/point cloud data into its perspective. |
| [ReprojectionToExternalCamera](ReprojectionToExternalCamera) | Calibrate a second Photoneo device as an "external camera" and reproject a main device's scan into its perspective. |
| [JointMarkerSpace](JointMarkerSpace) | Align the output point clouds of two devices into a shared coordinate space using marker recognition. |
| [DaisyChain](DaisyChain) | Connect two MotionCam-3D devices into a daisy chain, triggering scans on each other sequentially. |
| [TwoScanners](TwoScanners) | Connect to and trigger scans on multiple scanners sharing a single `PhoXiFactory`. |
| [TwoScannersMultithread](TwoScannersMultithread) | Handle multiple scanners from dedicated trigger, acquire, and control threads. |
| [MinimalOpenCV](MinimalOpenCV) | Convert a scanned point cloud from Photoneo format into OpenCV format. |
| [MinimalPcl](MinimalPcl) | Convert a scanned point cloud from Photoneo format into PCL format. |

Each example folder also has its own `ReadMe.txt` with a more detailed
description and, where relevant, extra build/setup steps (e.g. downloading
sample data, external dependency paths).

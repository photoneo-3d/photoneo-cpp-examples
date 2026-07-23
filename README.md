# Photoneo C++ examples
![image](https://photoneo.com/files/dw/dw/github/Personal_Linkedin_banner_v4.png)

## Introduction
This repository provides the building blocks necessary for developing your custom C++ application for working with [Photoneo](https://www.photoneo.com/) devices.
You may start your development based on one of the examples and modify it to suit your specific needs.

Each example is a standalone, independent project meant to be copied out and
adapted — there is no shared framework tying them together.

## Repository structure

This repository offers two independent ways to integrate with Photoneo 3D
sensors:

- [`PhoXiAPI/`](PhoXiAPI/README.md) — examples using Photoneo's own PhoXi API
  (via PhoXi Control): device connection, scanning, calibration,
  multi-scanner setups, external-camera reprojection, point cloud processing,
  and more. See [`PhoXiAPI/README.md`](PhoXiAPI/README.md) for the full list
  of examples.
- [`GigEV/aravis/`](GigEV/README.md) — examples using the GigE Vision
  protocol directly through the open-source
  [aravis](https://github.com/AravisProject/aravis) framework, bypassing
  PhoXi Control entirely. Requires PhoXi firmware ≥ 1.13.0. See
  [`GigEV/README.md`](GigEV/README.md) for details on the underlying GenICam
  features these examples use.

## Prerequisites

- CMake version 3.10 or above. You can download CMake from the following
  link: www.cmake.org/download/
- For `PhoXiAPI/` examples: [PhoXi Control](https://www.photoneo.com/kb/pxc)
  installed.
- For `GigEV/aravis/` examples: `aravis-0.8`, `glib-2.0`, and `gobject-2.0`
  (discovered via `pkg-config`); this is Linux-oriented, as aravis is not
  readily available on Windows.

## Building

Every `PhoXiAPI/<Example>/` folder is built independently:

```
cmake -S PhoXiAPI/<ExampleName> -B build
cmake --build build
```

All `GigEV/aravis/` examples are built together from its single root
`CMakeLists.txt`:

```
cmake -S GigEV/aravis -B build
cmake --build build
```

See [`PhoXiAPI/README.md`](PhoXiAPI/README.md) and
[`GigEV/README.md`](GigEV/README.md) for per-example details and additional
dependencies.

#### Support
Visit [www.photoneo.com](https://www.photoneo.com/) for the most up-to-date information and documents. If you encounter any issues while using the examples, please do not hesitate to contact our dedicated Support team at our [Help Center](https://www.photoneo.com/Help-Center) for prompt assistance.

#### License
Photoneo examples are distributed under the [BSD License](https://github.com/photoneo-3d/photoneo-cpp-examples/blob/main/LICENSE).

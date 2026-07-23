# Photoneo aravis (GigE Vision) examples

This folder contains C/C++ examples that connect to Photoneo 3D sensors over
the GigE Vision protocol using the open-source
[aravis](https://github.com/AravisProject/aravis) framework, bypassing PhoXi
Control entirely.

See [`../README.md`](../README.md) for the GenICam/GigE Vision reference
vocabulary (components, trigger modes, coordinate maps) that every example
below assumes without re-explaining it.

⚠️
These examples require PhoXi firmware version 1.13.0.
⚠️

## Prerequisites

- CMake 3.16 or newer.
- `aravis-0.8`, `glib-2.0`, and `gobject-2.0`, discovered via `pkg-config`.
  This is Linux-oriented, as aravis is not readily available on Windows.
- OpenCV (`core`, `highgui`, `imgproc`) — only needed for
  `ConnectAndGrab-ColorTexture`; if not found, that one example is skipped
  with a CMake warning rather than failing the whole build.
- A Photoneo device reachable on the network. Every example takes the
  device's IP address (or GenICam device ID, for `IPSettings`) as a
  command-line argument.

## Building

All examples are built together from the single root `CMakeLists.txt`:

```
cmake -S GigEV/aravis -B build
cmake --build build
```

New examples are registered by adding a `generate_example_app(<target>
SOURCES ...)` call in `CMakeLists.txt` — this helper macro (defined in
[`common/helper_functions.cmake`](common/helper_functions.cmake)) sets C++17,
wires up include directories, and links `PkgConfig::aravis_deps`
automatically.

`ConnectAndGrab-C` can alternatively be compiled standalone, without CMake:

```
gcc -o ConnectAndGrabC ConnectAndGrab-C/main.c -Wall $(pkg-config --libs --cflags aravis-0.8)
```

There are no automated tests in this repository — verification is done by
running the built examples against a real (or PhoXi Control-emulated)
device.

## Running an example

Most examples expect the device IP as the first argument:

```
./build/ConnectAndGrab 192.168.1.100
```

`IPSettings` is the exception — it also supports network-wide device
discovery without any prior connection:

```
./build/IPSettings discover
./build/IPSettings static <DEVICE ID> <IP> <MASK> <GATEWAY>
./build/IPSettings dhcp <DEVICE ID>
```

## Examples

| Example | Description |
| --- | --- |
| [ConnectAndGrab](ConnectAndGrab) | Connect, enable the Intensity/Range/Normal components, run continuous (freerun) acquisition, and retrieve 10 multipart buffers. Good starting point. |
| [ConnectAndGrab-C](ConnectAndGrab-C) | The same connect-and-grab flow written in plain C directly against the aravis API (no `common/` helpers). Demonstrates both multipart and chunk-data streaming, and freerun vs. software trigger. |
| [ConnectAndGrab-ColorTexture](ConnectAndGrab-ColorTexture) | Software-triggered grab of the color `Intensity` texture in both the `RGB8` and `Mono16` (YCoCg-encoded) pixel formats, converting the YCoCg data back to RGB with OpenCV via [`common/YCoCg.h`](common/YCoCg.h). |
| [ConnectAndGrab-SwTrigger](ConnectAndGrab-SwTrigger) | Software-triggered single-frame acquisition of the Intensity/Range components, including `NormalsEstimationRadius` handling. |
| [ComponentSelector](ComponentSelector) | Enumerate every `ComponentSelector` value the device supports together with its `ComponentIDValue`, enable all of them, and match the IDs against the parts of a received multipart buffer. |
| [GenICamSettings](GenICamSettings) | Read and write a broad set of GenICam device features — capturing, processing, coordinates, color, and calibration settings — through small typed helper wrappers ([`ReadWriteHelpers.h`](GenICamSettings/ReadWriteHelpers.h)/`.cpp`). |
| [IPSettings](IPSettings) | Discover Photoneo devices on the network and configure static IP or DHCP addressing (`arv-tool`-like `discover` / `static` / `dhcp` commands). |
| [ToggleJumboFrames](ToggleJumboFrames) | Enable or disable Jumbo Frames (`EnableJumboFrames`) on the device's network interface. |
| [UserSets](UserSets) | Create, save, and load multiple GenICam `UserSet` configurations (`UserSet0`, `UserSet1`, `Default`). |

## Architecture notes

Unlike the `PhoXiAPI/` examples (which duplicate helper code per example),
these examples share common code from [`common/`](common):

- [`PhoAravisCommon.h`](common/PhoAravisCommon.h) — RAII wrapper for aravis
  `GObject`s, and helpers for setting trigger mode, enabling/disabling output
  components, choosing multipart vs. image streaming, and issuing a software
  trigger. Used by every C++ example except `ConnectAndGrab-C`, which talks
  to the aravis API directly.
- [`CalculateNormals.h`](common/CalculateNormals.h) — converts the compact
  `Coord3D_AC8` normal encoding back into XYZ vectors.
- [`YCoCg.h`](common/YCoCg.h) — converts the `Mono16` YCoCg-encoded color
  texture back into RGB.
- [`helper_functions.cmake`](common/helper_functions.cmake) — the
  `generate_example_app` CMake macro used to declare every example target in
  the root `CMakeLists.txt`.

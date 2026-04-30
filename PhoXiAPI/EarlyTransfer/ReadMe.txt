========================================================================
    CONSOLE APPLICATION : EarlyTransfer Project Overview
========================================================================

The Early Transfer feature significantly reduces latency in processing
pipelines by allowing the device to send the selected texture/image data
as a separate frame as soon as it becomes available, before the rest of
the 3D data is ready.

When Early Transfer  feature is enabled, the device sends two frames per
one scan: the first frame contains the texture/image data, and the second
frame contains the complete 3D data (PointCloud, NormalMap, DepthMap,
Texture, TextureRGB, ConfidenceMap, ColorCameraImage according to the
current selection in OutputSettings).

You will learn how to:

* setting up the device to receive the Image as soon as possible
* receive all data in two frames per scan.

////////////////////////////////////////////////////////////////////////

This folder contains VPE interface source code, which is interface of
Verisilicon VPE codec, it provide interface for FFmpeg, GStreamer and
OpenMax.

Note: This code is not V4L2 based, so only suitable for non-V4L2 based
system integration.

1. File structure

--vpi               Top VPI folder
  │--inc            Include folder, include all external header files
  │--src            Source code folder
  │  |--dec         Decoder VPI implementation
  │  |--enc         Encoder VPI implementation
  │  |--filter      Filter VPI implementation
  │--utils          Tools

2. Verisilicon VPE implement overall framework:
┌────────────────────────────────────────────────────┐
│                   Application                      │
└────────────────────────┬───────────────────────────┘
                         │
┌───────────────┬────────┴─────────┬─────────────────┐
│    FFmpeg     │    GStreamer     │    OpenMax      │
│   ┌───────────┤   ┌──────────────┤   ┌─────────────┤
│   │VPE plugin │   │ VPE plugin   │   │ VPE plugin  │
└───┴───────────┴───┴─────┬────────┴───┴─────────────┘
                          │
┌─────────────────────────┴──────────────────────────┐
│                VPE Interface(VPI)                  │
└─────────────────────────┬──────────────────────────┘
                          │
┌─────────────────────────┴──────────────────────────┐
│                   VPE Codec Lib                    │
└─────────────────────────┬──────────────────────────┘
                          │
┌─────────────────────────┴──────────────────────────┐
│             VPE Hardware Wrapper Layer             │
└─────────────────────────┬──────────────────────────┘
                          │
┌─────────────────────────┴──────────────────────────┐
│                 VPE Kernel driver                  │
└─────────────────────────┬──────────────────────────┘
                          │
┌─────────────────────────┴──────────────────────────┐
│                 VPE Codec Hardware                 │
└────────────────────────────────────────────────────┘

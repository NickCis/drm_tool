# drm_tool

A simple tool for getting drm info and setting properties.

This tool came as an experiment when trying to use audio through HDMI in a ATI card. Audio started to work after starting Xorg.

This was tested on the following ATI cards:
* XFX R7 240
* XFX R7 850

Links of interest (none of this links solve the issue, but describe it):
* [No audio using alsa on HDMI until Xorg is started - arch linux forum](https://bbs.archlinux.org/viewtopic.php?id=188553)
* [Radeon HDMI audio out defaults to disabled - arch linux forum](https://bbs.archlinux.org/viewtopic.php?id=190743)
* [No audio using alsa on HDMI until Xorg is started :: linuxquestions - reddit](https://www.reddit.com/r/linuxquestions/comments/2jwz53/no_sound_over_radeon_hdmi_port_until_xorg_is/)


## Description

Aparently Xorg enable the audio property when it starts. The function resposible to do this in the kernel is `evergreen_hdmi_setmode` (`drivers/gpu/drm/radeon/evergreen_hdmi.c`), this cards use the `dce6` hdmi audio (`drivers/gpu/drm/radeon/dce6_afmt.c`). By default, when the cards are setted up audio is disabled.

In order to enable the audio the `drmModeConnectorSetProperty(drm_fd, connector_id, prop_id, value);` function of libdrm (`/usr/include/xf86drmMode.h`) must be called. The `audio` property id is usually `12` (in the tool the property is looked up by the name). Xorg sets this property with the value `2`.

For more information on how to use the drm api check the followin links:
* [docs - dvhrm :: github repository](https://github.com/dvdhrm/docs/)
* [Linux DRM Mode-Setting API | Ponyhof](https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api/)
* [Advanced DRM Mode-Setting API | Ponyhof](https://dvdhrm.wordpress.com/2012/12/21/advanced-drm-mode-setting-api/)


## Example

```
$ make
gcc -c -Wall -D_FILE_OFFSET_BITS=64 `pkg-config --cflags libdrm`  -o "src/drm_tool.o" "src/drm_tool.c"
gcc `pkg-config --libs libdrm` src/drm_tool.o -o "./drm_tool"
$ ./drm_tool list
Card /dev/dri/card0
 capability 'DUMP BUFFER' (1) : 1
 capability 'VBLANK HIGH CRTC' (2) : 1
 capability 'DUMB PREFERED DEPTH' (3) : 24
 capability 'DUMB PREFER SHADOW' (4) : 1
 capability 'PRIME' (5) : 3
 capability 'TIMESTAMP MONOTONIC' (6) : 1
  Connector: 27
    unused connector 27
    property (#1):  EDID
    property (#2):  DPMS
    property (#5):  coherent
    property (#9):  underscan
    property (#10):  underscan hborder
    property (#11):  underscan vborder
    property (#12):  audio
    property (#13):  dither
  Connector: 29
    unused connector 29
    property (#1):  EDID
    property (#2):  DPMS
    property (#5):  coherent
    property (#9):  underscan
    property (#10):  underscan hborder
    property (#11):  underscan vborder
    property (#12):  audio
    property (#13):  dither
  Connector: 31
    mode  0: 1920x1080       60Hz 1920x1080
    mode  1: 1920x1080       60Hz 1920x1080
    mode  2: 1920x1080i      60Hz 1920x1080
    mode  3: 1920x1080i      60Hz 1920x1080
    mode  4: 1920x1080       50Hz 1920x1080
    mode  5: 1920x1080i      50Hz 1920x1080
    mode  6: 1600x1200       60Hz 1600x1200
    mode  7: 1680x1050       60Hz 1680x1050
    mode  8: 1280x1024       60Hz 1280x1024
    mode  9: 1440x900        60Hz 1440x900
    mode 10: 1280x960        60Hz 1280x960
    mode 11: 1280x800        60Hz 1280x800
    mode 12: 1280x720        60Hz 1280x720
    mode 13: 1280x720        60Hz 1280x720
    mode 14: 1280x720        50Hz 1280x720
    mode 15: 1024x768        60Hz 1024x768
    mode 16: 800x600         60Hz 800x600
    mode 17: 800x600         56Hz 800x600
    mode 18: 720x576         50Hz 720x576
    mode 19: 720x480         60Hz 720x480
    mode 20: 720x480         60Hz 720x480
    mode 21: 640x480         60Hz 640x480
    mode 22: 640x480         60Hz 640x480
    property (#1):  EDID
    property (#2):  DPMS
    property (#5):  coherent
    property (#9):  underscan
    property (#10):  underscan hborder
    property (#11):  underscan vborder
    property (#12):  audio
    property (#13):  dither
  Connector: 33
    unused connector 33
    property (#1):  EDID
    property (#2):  DPMS
    property (#5):  coherent
    property (#9):  underscan
    property (#10):  underscan hborder
    property (#11):  underscan vborder
    property (#12):  audio
    property (#13):  dither
  Connector: 35
    unused connector 35
    property (#1):  EDID
    property (#2):  DPMS
    property (#5):  coherent
    property (#9):  underscan
    property (#10):  underscan hborder
    property (#11):  underscan vborder
    property (#12):  audio
    property (#13):  dither
    property (#6):  load detection
$ ./drm_tool set /dev/dri/card0 31 audio 2
Setting 'audio' -> 2 - ret: 0 :: connector_id: 31 prop_id: 12
```


Gimp Pixel Art Scalers Plugin
===========

GIMP plugin for rescaling images using Pixel Art Scalers

Supported image scalers:
 * HQX (hq2x, hq3x, hq4x)
 * XBR (xbr2x, xbr3x, xbr4x)

![GIMP Image Editor using Pixel Art Scalers Plugin](https://raw.githubusercontent.com/bbbbbr/gimp-plugin-pixel-art-scalers/master/info/gimp-plugin-pixel-art-scalers.png)


 OS binaries available for:
 * Linux
 * Windows
 

## Acknowledgement:
  * For an overview of the scalers see: https://en.wikipedia.org/wiki/Pixel-art_scaling_algorithms
  * HQX: https://web.archive.org/web/20130925011623/http://www.hiend3d.com/hq2x.html
  * XBR: https://forums.libretro.com/t/xbr-algorithm-tutorial/123 , https://github.com/Treeki/libxbr-standalone
  * Scale2x/3x : https://www.scale2x.it/ , https://opengameart.org/forumtopic/pixelart-scaler-scalenx-and-eaglenx-for-gimp

## Quick instructions:

Native compile/install on Linux using below.

```
gimptool-2.0 --install (TODO).c
    or
 make (and then copy to your GIMP plugin folder, depends on version)

Plug-in folder locations:
 Linux: ~/.gimp-2.8/plug-ins
 Windows: C:\Program Files\GIMP 2\lib\gimp\2.0\plug-ins

```


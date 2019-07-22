Gimp Pixel Art Scalers Plugin
===========

GIMP plugin for rescaling images using Pixel Art Scalers

These scalers are able to resize graphics in a way that avoids both bluring and pixelation, while also preserving the look and feel of the image. They are commonly used for upscaling pixel art from console games when run in an emulator. They can be useful in other realms too.

Supported image scalers:
 * HQX (hq2x, hq3x, hq4x)
 * HRIS (hris2x, hris3x)
 * XBR (xbr2x, xbr3x, xbr4x)
 * ScaleX (scale2x, scale3x, scale4x)
 * Gsample (gsample2x, gsample3x, gsample4x)
 * Nearest Neighbor (nearest2x, nearest3x, nearest4x)

![GIMP Image Editor using Pixel Art Scalers Plugin](https://raw.githubusercontent.com/bbbbbr/gimp-plugin-pixel-art-scalers/master/info/gimp-plugin-pixel-art-scalers.png)


OS binaries available for:
 * Linux
 * Windows


Hints:
 * The plugin is located in : Menu -> Filter -> Render -> Pixel Art Scalers
 * You can resize the plugin dialog for a larger preview window
 * Currently it only operates on RGB and RGBA images


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


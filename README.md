Gimp Pixel Art Scalers Plugin
===========

GIMP plugin for rescaling images using Pixel Art Scalers

These scalers are able to resize graphics in a way that avoids both bluring and pixelation, while also preserving the look and feel of the image. They are commonly used for upscaling pixel art from console games when run in an emulator. They can be useful in other realms too.

Download compiled executables here:
 * [Linux GIMP 2.8+ x64](/bin/linux)
 * [Windows GIMP 2.10.12+ x32](/bin/windows)
 * macOS / OS X: See [Install on macOS using homebrew](#install-on-macos-using-homebrew)
 
See [instructions below for where to install the binary](#install-folder--path-locations)

Supported image scalers:
 * HQX (hq2x, hq3x, hq4x)
 * XBR (xbr2x, xbr3x, xbr4x)
 * ScaleX (scale2x, scale3x, scale4x)

![GIMP Image Editor using Pixel Art Scalers Plugin](https://raw.githubusercontent.com/bbbbbr/gimp-plugin-pixel-art-scalers/master/info/gimp-plugin-pixel-art-scalers.png)


Options:
 * Force semi-transparent pixels to fully opaque / transparent (edge aliasing)
 * Suppress colors from alpha-hidden pixels (can sometimes cause discoloration at the edges)
 * Tiled and transparent temporary border options to improve scaling in special cases


Hints:
 * The plugin is located in : Menu -> Filter -> Render -> Pixel Art Scalers
 * You can resize the plugin dialog for a larger preview window
 * Currently it only operates on RGB and RGBA images


## Acknowledgement:
  * For an overview of the scalers see: https://en.wikipedia.org/wiki/Pixel-art_scaling_algorithms
  * HQX: https://web.archive.org/web/20130925011623/http://www.hiend3d.com/hq2x.html
  * XBR: https://forums.libretro.com/t/xbr-algorithm-tutorial/123 , https://github.com/Treeki/libxbr-standalone
  * Scale2x/3x : https://www.scale2x.it/ , https://opengameart.org/forumtopic/pixelart-scaler-scalenx-and-eaglenx-for-gimp

---
## Install folder & path locations

Copy the "plugin-pixel-art-scalers"(.exe) binary to your GIMP plugin folder. The location will depend on your GIMP version and Operating System.
```
Plug-in folder locations: 
(where 2.x is your gimp version. example: ~/.gimp-2.8/plug-ins)

  * Linux: ~/.gimp-2.x/plug-ins
  * Windows: %APPDATA%\GIMP\2.x\plug-ins  or  C:\Program Files\GIMP 2\lib\gimp\2.0\plug-ins
  * macOS / OSX: $HOME/Library/Application Support/GIMP/2.x/plug-ins
```
---
## Build instructions

#### Native compile on Linux:
```
If GIMP & build tools not yet installed:
(example for debian/ubuntu/mint)
 * sudo apt install gimp
 * sudo apt install build-essential
 * sudo apt install libgimp2.0-dev

Then:
* cd gimp-plugin-pixel-art-scalers
* make
```

---
#### Install on macOS using homebrew
([Formula](https://github.com/ryan-robeson/homebrew-gimp/releases) courtesy of [@ryan-robeson](https://github.com/ryan-robeson).)
```
1. Run: brew install ryan-robeson/gimp/pixel-art-scalers
2. Then add the new plugin folder to GIMP's settings
```
---
#### Guide for [Cross-compiling to Windows on Linux](https://github.com/bbbbbr/gimp-rom-bin/blob/master/doc/GIMP%20jhbuild%20for%20Windows%20on%20Linux.md)

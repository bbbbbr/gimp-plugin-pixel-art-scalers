//
// scaler_nearestneighbor.c
//

// ========================
//
// ScaleNx pixel art scaler
//
// See: https://www.scale2x.it/
// Also: https://web.archive.org/web/20140331104356/http://gimpscripts.com/2014/01/scale2x-plus/
//
// Adapted to C (was python) from : https://opengameart.org/forumtopic/pixelart-scaler-scalenx-and-eaglenx-for-gimp
//
// ========================

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "scaler_nearestneighbor.h"

static const char TRUE  = 1;
static const char FALSE = 0;
static const char BYTE_SIZE_RGBA_4BPP = 4; // RGBA 4BPP

static const char SCALE_NEAREST_2X = 2;
static const char SCALE_NEAREST_3X = 3;
static const char SCALE_NEAREST_4X = 4;

void scaler_nearest_nx(uint32_t *, uint32_t *, int, int, int);

// Upscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void scaler_nearest_nx(uint32_t * sp,  uint32_t * dp, int Xres, int Yres, int scale_factor)
{
    int       bpp;
    int       x, y, sx, sb;
    uint32_t * src, * dst;
    long      line_width_scaled;

    bpp = BYTE_SIZE_RGBA_4BPP;  // Assume 4BPP RGBA
    line_width_scaled = (Xres * scale_factor * (bpp/sizeof(uint32_t)) );
    src = (uint32_t *) sp;
    dst = (uint32_t *) dp;


    for (y=0; y < Yres; y++) {
        // Copy each line from the source image
        for (x=0; x < Xres; x++) {

            // Copy new pixels from left source pixel
            for (sx=0; sx < scale_factor; sx++) {
                // Copy each uint32 (RGBA 4BPP) pixel to the dest buffer
                    *dp++ = *sp;
                }
            // Move to next source pixel
            sp++;
        }

        // Duplicate the preceding line (at scaled size) N times if needed
        for (sx=0; sx < (scale_factor -1); sx++) {
            // memcopy operates on 8 bits, the data size is 32 bits
            memcpy(dp, dp - line_width_scaled, line_width_scaled * sizeof(uint32_t));
            dp+= line_width_scaled;
        }
    }
}



void scaler_nearest_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_nearest_nx(sp, dp, Xres, Yres, SCALE_NEAREST_2X);
}

void scaler_nearest_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_nearest_nx(sp, dp, Xres, Yres, SCALE_NEAREST_3X);
}

void scaler_nearest_4x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_nearest_nx(sp, dp, Xres, Yres, SCALE_NEAREST_4X);
}


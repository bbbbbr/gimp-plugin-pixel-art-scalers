/*
 * gsample.c
 *
 * ==================================
 *
 * Gradient Sample (the rules of RIS)
 *
 * ==================================
 *
 * Author: 2016 zvezdochiot (mykaralw@yandex.ru)
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 */

#include "common_ris.h"
#include "gsample.h"

// Upscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void gsample(uint32_t *sp,  uint32_t *dp, int Xres, int Yres, int scale_factor)
{
    int bpp = BYTE_SIZE_RGBA_4BPP;
    int i, j, d, k, l, il, stepl, deststep;
    int prevline2, prevline, nextline, nextline2;
    uint32_t wt;
    ARGBpixel w[9];
    ARGBpixel wr;
    uint32_t *dest = (uint32_t *) dp;

    double imx, dy, dx, di, dj, dd;
    double deltay[BYTE_SIZE_RGBA_4BPP];
    double deltax[BYTE_SIZE_RGBA_4BPP];

    deststep = Xres * scale_factor;
    dd = 1.0 / 16.0 / (double)scale_factor;

    for (j = 0; j < Yres; j++)
    {
        if (j > 0)      prevline = -Xres; else prevline = 0;
        if (j < Yres-1) nextline =  Xres; else nextline = 0;

        for (i = 0; i < Xres; i++)
        {
            wt = *(sp + prevline);
            w[1] = ARGBtoPixel(wt);
            wt = *sp;
            w[4] = ARGBtoPixel(wt);
            wt = *(sp + nextline);
            w[7] = ARGBtoPixel(wt);

            if (i > 0)
            {
                wt = *(sp + prevline - 1);
                w[0] = ARGBtoPixel(wt);
                wt = *(sp - 1);
                w[3] = ARGBtoPixel(wt);
                wt = *(sp + nextline - 1);
                w[6] = ARGBtoPixel(wt);
            }
            else
            {
                w[0] = w[1];
                w[3] = w[4];
                w[6] = w[7];
            }
            if (i < Xres-1)
            {
                wt = *(sp + prevline + 1);
                w[2] = ARGBtoPixel(wt);
                wt = *(sp + 1);
                w[5] = ARGBtoPixel(wt);
                wt = *(sp + nextline + 1);
                w[8] = ARGBtoPixel(wt);
            }
            else
            {
                w[2] = w[1];
                w[5] = w[4];
                w[8] = w[7];
            }

            for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
            {
                dy = (double)w[7].c[d];
                dy += (double)w[6].c[d];
                dy += (double)w[7].c[d];
                dy += (double)w[8].c[d];
                dy -= (double)w[1].c[d];
                dy -= (double)w[0].c[d];
                dy -= (double)w[1].c[d];
                dy -= (double)w[2].c[d];
                deltay[d] = dy * dd;
                dx = (double)w[5].c[d];
                dx += (double)w[2].c[d];
                dx += (double)w[5].c[d];
                dx += (double)w[8].c[d];
                dx -= (double)w[3].c[d];
                dx -= (double)w[0].c[d];
                dx -= (double)w[3].c[d];
                dx -= (double)w[6].c[d];
                deltax[d] = dx * dd;
            }
            for (k =  0; k < scale_factor; k++)
            {
                di = -scale_factor;
                di += 1.0;
                di *= 0.5;
                di += k;
                il = 0;
                stepl = 0;
                for (l =  0; l < scale_factor; l++)
                {
                    dj = -scale_factor;
                    dj += 1.0;
                    dj *= 0.5;
                    dj += l;
                    for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
                    {
                        imx = w[4].c[d];
                        imx += (deltay[d] * dj);
                        imx += (deltax[d] * di);
                        wr.c[d] = ByteClamp((int)(imx + 0.5));
                    }
                    wt = PixeltoARGB(wr);
                    *(dest + stepl) = wt;
                    il += scale_factor;
                    stepl += deststep;
                }
                dest++;
            }
            sp++;
        }
        dest += deststep * (scale_factor - 1);
    }
}

// gsample_2x
//
// Scales image in *sp up by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 2 * Xres * 2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void gsample_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    gsample(sp, dp, Xres, Yres, GSAMPLE_2X);
}

// gsample_3x
//
// Scales image in *sp up by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 3 * Xres * 3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void gsample_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    gsample(sp, dp, Xres, Yres, GSAMPLE_3X);
}

// gsample_4x
//
// Scales image in *sp up by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 4 * Xres * 4 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void gsample_4x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    gsample(sp, dp, Xres, Yres, GSAMPLE_4X);
}

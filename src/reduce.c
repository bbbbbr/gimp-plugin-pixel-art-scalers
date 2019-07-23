/*
 * reduce.c
 *
 * ==================================
 *
 * Reduce (the rules of RIS)
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
#include "reduce.h"

// Downscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void scaler_mean_x(uint32_t *sp, uint32_t *dp, int Xres, int Yres, int scale_factor)
{
    int bpp = BYTE_SIZE_RGBA_4BPP;
    int i, j, d, k, l, kj, li, m;
    int n = scale_factor * scale_factor;
    uint32_t wt;
    ARGBpixel w[n];
    ARGBpixel wr;
    uint32_t *dest = (uint32_t *) dp;

    double imx;

    for (j = 0; j < Yres; j+=scale_factor)
    {
        for (i = 0; i < Xres; i+=scale_factor)
        {
            m = 0;
            for (k = 0; k < scale_factor; k++)
            {
                kj = ((j + k) < Yres) ? (j + k) : (Yres - 1);
                kj *= Xres;
                for (l = 0; l < scale_factor; l++)
                {
                    li = ((i + l) < Xres) ? (i + l) : (Xres - 1);
                    wt = *(sp + kj + li);
                    w[m] = ARGBtoPixel(wt);
                    m++;
                }
            }

            for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
            {
                imx = 0.0;
                for (k = 0; k < n; k++)
                {
                    imx += (double)w[k].c[d];
                }
                imx /= (double)n;
                wr.c[d] = ByteClamp((int)(imx + 0.5));
            }
            wt = PixeltoARGB(wr);
            *(dest) = wt;
            dest++;
        }
    }
}

// mean_2x
//
// Scales image in *sp down by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/2 * Xres * 1/2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_mean_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_mean_x(sp, dp, Xres, Yres, REDUCE_2X);
}

// mean_3x
//
// Scales image in *sp down by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/3 * Xres * 1/3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_mean_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_mean_x(sp, dp, Xres, Yres, REDUCE_3X);
}

// mean_4x
//
// Scales image in *sp down by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/4 * Xres * 1/4 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_mean_4x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_mean_x(sp, dp, Xres, Yres, REDUCE_4X);
}

int pixel_compare(ARGBpixel a, ARGBpixel b)
{
    int d, imd, imr = 0;
    for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
    {
        imd = (int)a.c[d];
        imd -= (int)b.c[d];
        imr += ABS(imd);
    }

    return imr;
}

ARGBpixel pixel_mirror(ARGBpixel a, ARGBpixel b)
{
    int d, imm;
    for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
    {
        imm = (int)a.c[d];
        imm += imm;
        imm -= (int)b.c[d];
        a.c[d] = ByteClamp(imm);;
    }

    return a;
}

// Downscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void reduce_x(uint32_t *sp, uint32_t *dp, int Xres, int Yres, int scale_factor)
{
    int bpp = BYTE_SIZE_RGBA_4BPP;
    int i, j, d, k, l, kj, li, m, imin;
    int n = scale_factor * scale_factor;
    int nres = Xres * Yres;
    uint32_t wt;
    ARGBpixel w[n];
    ARGBpixel wm, wr;
    uint32_t *dest = (uint32_t *) dp;
    int dist, distmin;

    double imx;

    wm = threshold_bimod (sp, Xres, Yres);
    for (j = 0; j < Yres; j+=scale_factor)
    {
        for (i = 0; i < Xres; i+=scale_factor)
        {
            m = 0;
            for (k = 0; k < scale_factor; k++)
            {
                kj = ((j + k) < Yres) ? (j + k) : (Yres - 1);
                kj *= Xres;
                for (l = 0; l < scale_factor; l++)
                {
                    li = ((i + l) < Xres) ? (i + l) : (Xres - 1);
                    wt = *(sp + kj + li);
                    w[m] = ARGBtoPixel(wt);
                    m++;
                }
            }
            for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
            {
                imx = 0.0;
                for (k = 0; k < n; k++)
                {
                    imx += (double)w[k].c[d];
                }
                imx /= (double)n;
                wr.c[d] = ByteClamp((int)(imx + 0.5));
            }
            wr = pixel_mirror(wr, wm);
            imin = 0;
            distmin = pixel_compare(wr, w[0]);
            for (k = 0; k < n; k++)
            {
                dist =  pixel_compare(wr, w[k]);
                if (dist < distmin)
                {
                    imin = k;
                    distmin = dist;
                }
            }
            wt = PixeltoARGB(w[imin]);
            *(dest) = wt;
            dest++;
        }
    }
}

// reduce_2x
//
// Scales image in *sp down by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/2 * Xres * 1/2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void reduce_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    reduce_x(sp, dp, Xres, Yres, REDUCE_2X);
}

// reduce_3x
//
// Scales image in *sp down by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/3 * Xres * 1/3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void reduce_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    reduce_x(sp, dp, Xres, Yres, REDUCE_3X);
}

// reduce_4x
//
// Scales image in *sp down by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 1/4 * Xres * 1/4 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void reduce_4x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    reduce_x(sp, dp, Xres, Yres, REDUCE_4X);
}


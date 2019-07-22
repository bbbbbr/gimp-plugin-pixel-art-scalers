/*
 * hris.c
 *
 * =====================================
 *
 * Half Reverse Interpolate Scale (HRIS)
 *
 * =====================================
 *
 * Author: 2016 zvezdochiot (mykaralw@yandex.ru)
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 */

#include "common_ris.h"
#include "hris.h"

// Upscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void scaler_hris(uint32_t *sp,  uint32_t *dp, int Xres, int Yres, int scale_mode)
{
    int bpp = BYTE_SIZE_RGBA_4BPP;
    int i, j, d, k, l, il, stepl,deststep;
    int prevline2, prevline, nextline, nextline2;
    uint32_t wt;
    ARGBpixel w[25];
    ARGBpixel wr[9];
    uint32_t *dest = (uint32_t *) dp;

    double imx;
    double b11, b12, b13, b21, b22, b23, b31, b32, b33;
    double r11, r12, r13, r21, r22, r23, r31, r32, r33;

    double k0[2];
    double k1[3];
    double k2[3];

    if (scale_mode < 2) scale_mode = 2;
    if (scale_mode > 3) scale_mode = 3;

    deststep = Xres * scale_mode;

    if (scale_mode == SCALE_HRIS_2X)
    {
        k0[0] = 0.750;
        k0[1] = 0.250;
        k1[0] = k0[0] * k0[0];
        k1[1] = k0[0] * k0[1];
        k1[2] = k0[1] * k0[1];
        k2[0] = k1[0];
        k2[1] = k1[1] / 2;
        k2[2] = k1[2] / 4;
    }
    else
    {
        k0[0] = 2.0 / 3.0;
        k0[1] = 1.0 / 3.0;
        k1[0] = k0[0] * k0[0];
        k1[1] = k0[0] * k0[1];
        k1[2] = k0[1] * k0[1];
        k2[0] = (1 + 4 * k0[0] + 4 * k1[0]) / 9;
        k2[1] = (k0[1] + 2 * k1[1]) / 9;
        k2[2] = k1[2] / 9;
    }

    for (j = 0; j < Yres; j++)
    {
        if (j > 1)      prevline2 = -Xres-Xres;
        else prevline2 = 0;
        if (j > 0)      prevline = -Xres;
        else prevline = 0;
        if (j < Yres-1) nextline =  Xres;
        else nextline = 0;
        if (j < Yres-2) nextline2 = Xres+Xres;
        else nextline2 = 0;

        for (i = 0; i < Xres; i++)
        {
            wt = *(sp + prevline2);
            w[2] = ARGBtoPixel(wt);
            wt = *(sp + prevline);
            w[7] = ARGBtoPixel(wt);
            wt = *sp;
            w[12] = ARGBtoPixel(wt);
            wt = *(sp + nextline);
            w[17] = ARGBtoPixel(wt);
            wt = *(sp + nextline);
            w[22] = ARGBtoPixel(wt);

            if (i > 1)
            {
                wt = *(sp + prevline2 - 2);
                w[0] = ARGBtoPixel(wt);
                wt = *(sp + prevline - 2);
                w[5] = ARGBtoPixel(wt);
                wt = *(sp - 2);
                w[10] = ARGBtoPixel(wt);
                wt = *(sp + nextline - 2);
                w[15] = ARGBtoPixel(wt);
                wt = *(sp + nextline2 - 2);
                w[20] = ARGBtoPixel(wt);
            }
            else
            {
                w[0] = w[2];
                w[5] = w[7];
                w[10] = w[12];
                w[15] = w[17];
                w[20] = w[22];
            }
            if (i > 0)
            {
                wt = *(sp + prevline2 - 1);
                w[1] = ARGBtoPixel(wt);
                wt = *(sp + prevline - 1);
                w[6] = ARGBtoPixel(wt);
                wt = *(sp - 1);
                w[11] = ARGBtoPixel(wt);
                wt = *(sp + nextline - 1);
                w[16] = ARGBtoPixel(wt);
                wt = *(sp + nextline2 - 1);
                w[21] = ARGBtoPixel(wt);
            }
            else
            {
                w[1] = w[2];
                w[6] = w[7];
                w[11] = w[12];
                w[16] = w[17];
                w[21] = w[22];
            }
            if (i < Xres-1)
            {
                wt = *(sp + prevline2 + 1);
                w[3] = ARGBtoPixel(wt);
                wt = *(sp + prevline + 1);
                w[8] = ARGBtoPixel(wt);
                wt = *(sp + 1);
                w[13] = ARGBtoPixel(wt);
                wt = *(sp + nextline + 1);
                w[18] = ARGBtoPixel(wt);
                wt = *(sp + nextline2 + 1);
                w[23] = ARGBtoPixel(wt);
            }
            else
            {
                w[3] = w[2];
                w[8] = w[7];
                w[13] = w[12];
                w[18] = w[17];
                w[23] = w[22];
            }
            if (i < Xres-2)
            {
                wt = *(sp + prevline2 + 2);
                w[4] = ARGBtoPixel(wt);
                wt = *(sp + prevline + 2);
                w[9] = ARGBtoPixel(wt);
                wt = *(sp + 2);
                w[14] = ARGBtoPixel(wt);
                wt = *(sp + nextline + 2);
                w[19] = ARGBtoPixel(wt);
                wt = *(sp + nextline2 + 2);
                w[24] = ARGBtoPixel(wt);
            }
            else
            {
                w[4] = w[2];
                w[9] = w[7];
                w[14] = w[12];
                w[19] = w[17];
                w[24] = w[22];
            }

            for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
            {
                imx = (double)w[0].c[d];
                b11 = -(k2[2] * imx);
                imx = (double)w[1].c[d];
                b11 -= (k2[1] * imx);
                b12 = -(k2[2] * imx);
                imx = (double)w[2].c[d];
                b11 -= (k2[2] * imx);
                b12 -= (k2[1] * imx);
                b13 = -(k2[2] * imx);
                imx = (double)w[3].c[d];
                b12 -= (k2[2] * imx);
                b13 -= (k2[1] * imx);
                imx = (double)w[4].c[d];
                b13 -= (k2[2] * imx);
                imx = (double)w[5].c[d];
                b11 -= (k2[1] * imx);
                b21 = -(k2[2] * imx);
                imx = (double)w[6].c[d];
                b11 += ((2.0 - k2[0]) * imx);
                b12 -= (k2[1] * imx);
                b21 -= (k2[1] * imx);
                b22 = -(k2[2] * imx);
                imx = (double)w[7].c[d];
                b11 -= (k2[1] * imx);
                b12 += ((2.0 - k2[0]) * imx);
                b13 -= (k2[1] * imx);
                b21 -= (k2[2] * imx);
                b22 -= (k2[1] * imx);
                b23 = -(k2[2] * imx);
                imx = (double)w[8].c[d];
                b12 -= (k2[1] * imx);
                b13 += ((2.0 - k2[0]) * imx);
                b22 -= (k2[2] * imx);
                b23 -= (k2[1] * imx);
                imx = (double)w[9].c[d];
                b13 -= (k2[1] * imx);
                b23 -= (k2[2] * imx);
                imx = (double)w[10].c[d];
                b11 -= (k2[2] * imx);
                b21 -= (k2[1] * imx);
                b31 = -(k2[2] * imx);
                imx = (double)w[11].c[d];
                b11 -= (k2[1] * imx);
                b12 -= (k2[2] * imx);
                b21 += ((2.0 - k2[0]) * imx);
                b22 -= (k2[1] * imx);
                b31 -= (k2[1] * imx);
                b32 = -(k2[2] * imx);
                imx = (double)w[12].c[d];
                b11 -= (k2[2] * imx);
                b12 -= (k2[1] * imx);
                b13 -= (k2[2] * imx);
                b21 -= (k2[1] * imx);
                b22 += ((2.0 - k2[0]) * imx);
                b23 -= (k2[1] * imx);
                b31 -= (k2[2] * imx);
                b32 -= (k2[1] * imx);
                b33 = -(k2[2] * imx);
                imx = (double)w[13].c[d];
                b12 -= (k2[2] * imx);
                b13 -= (k2[1] * imx);
                b22 -= (k2[1] * imx);
                b23 += ((2.0 - k2[0]) * imx);
                b32 -= (k2[2] * imx);
                b33 -= (k2[1] * imx);
                imx = (double)w[14].c[d];
                b13 -= (k2[2] * imx);
                b23 -= (k2[1] * imx);
                b33 -= (k2[2] * imx);
                imx = (double)w[15].c[d];
                b21 -= (k2[2] * imx);
                b31 -= (k2[1] * imx);
                imx = (double)w[16].c[d];
                b21 -= (k2[1] * imx);
                b22 -= (k2[2] * imx);
                b31 += ((2.0 - k2[0]) * imx);
                b32 -= (k2[1] * imx);
                imx = (double)w[17].c[d];
                b21 -= (k2[2] * imx);
                b22 -= (k2[1] * imx);
                b23 -= (k2[2] * imx);
                b31 -= (k2[1] * imx);
                b32 += ((2.0 - k2[0]) * imx);
                b33 -= (k2[1] * imx);
                imx = (double)w[18].c[d];
                b22 -= (k2[2] * imx);
                b23 -= (k2[1] * imx);
                b32 -= (k2[1] * imx);
                b33 += ((2.0 - k2[0]) * imx);
                imx = (double)w[19].c[d];
                b23 -= (k2[2] * imx);
                b33 -= (k2[1] * imx);
                imx = (double)w[20].c[d];
                b31 -= (k2[2] * imx);
                imx = (double)w[21].c[d];
                b31 -= (k2[1] * imx);
                b32 -= (k2[2] * imx);
                imx = (double)w[22].c[d];
                b31 -= (k2[2] * imx);
                b32 -= (k2[1] * imx);
                b33 -= (k2[2] * imx);
                imx = (double)w[23].c[d];
                b32 -= (k2[2] * imx);
                b33 -= (k2[1] * imx);
                imx = (double)w[24].c[d];
                b33 -= (k2[2] * imx);

                if (scale_mode == SCALE_HRIS_2X)
                {
                    r11 = (k1[0] * b22 + k1[1] * (b12 + b21) + k1[2] * b11);
                    r12 = (k1[0] * b22 + k1[1] * (b12 + b23) + k1[2] * b13);
                    r21 = (k1[0] * b22 + k1[1] * (b32 + b21) + k1[2] * b31);
                    r22 = (k1[0] * b22 + k1[1] * (b32 + b23) + k1[2] * b33);

                    wr[0].c[d] = ByteClamp((int)(r11 + 0.5));
                    wr[1].c[d] = ByteClamp((int)(r12 + 0.5));
                    wr[2].c[d] = ByteClamp((int)(r21 + 0.5));
                    wr[3].c[d] = ByteClamp((int)(r22 + 0.5));
                }
                else
                {
                    r11 = (k1[0] * b22 + k1[1] * (b12 + b21) + k1[2] * b11);
                    r12 = (k0[0] * b22 + k0[1] * b12);
                    r13 = (k1[0] * b22 + k1[1] * (b12 + b23) + k1[2] * b13);
                    r21 = (k0[0] * b22 + k0[1] * b21);
                    r22 = (double)w[12].c[d];
                    r23 = (k0[0] * b22 + k0[1] * b23);
                    r31 = (k1[0] * b22 + k1[1] * (b32 + b21) + k1[2] * b31);
                    r32 = (k0[0] * b22 + k0[1] * b32);
                    r33 = (k1[0] * b22 + k1[1] * (b32 + b23) + k1[2] * b33);

                    wr[0].c[d] = ByteClamp((int)(r11 + 0.5));
                    wr[1].c[d] = ByteClamp((int)(r12 + 0.5));
                    wr[2].c[d] = ByteClamp((int)(r13 + 0.5));
                    wr[3].c[d] = ByteClamp((int)(r21 + 0.5));
                    wr[4].c[d] = ByteClamp((int)(r22 + 0.5));
                    wr[5].c[d] = ByteClamp((int)(r23 + 0.5));
                    wr[6].c[d] = ByteClamp((int)(r31 + 0.5));
                    wr[7].c[d] = ByteClamp((int)(r32 + 0.5));
                    wr[8].c[d] = ByteClamp((int)(r33 + 0.5));
                }
            }
            for (k =  0; k < scale_mode; k++)
            {
                il = 0;
                stepl = 0;
                for (l =  0; l < scale_mode; l++)
                {
                    wt = PixeltoARGB(wr[k+il]);
                    *(dest + stepl) = wt;
                    il += scale_mode;
                    stepl += deststep;
                }
                dest++;
            }
            sp++;
        }
        dest += deststep * (scale_mode - 1);
    }
}

// scaler_hris_2x
//
// Scales image in *sp up by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 2 * Xres * 2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_hris_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_hris(sp, dp, Xres, Yres, SCALE_HRIS_2X);
}


// scaler_hris_3x
//
// Scales image in *sp up by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 3 * Xres * 3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_hris_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    scaler_hris(sp, dp, Xres, Yres, SCALE_HRIS_3X);
}

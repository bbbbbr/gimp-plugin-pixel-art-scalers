//
// scaler_scalex.c
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
#include <string.h>

#include "scaler_scalex.h"

static const char TRUE  = 1;
static const char FALSE = 0;
static const char BYTE_SIZE_RGBA_4BPP = 4; // RGBA 4BPP

void pixel_copy(uint8_t *, uint32_t, uint8_t *, uint32_t,int);
uint8_t pixel_eql(uint8_t *, uint32_t, uint32_t, int);
void scale_scale2x(uint8_t *, uint32_t *, int, int, int, int, int);
void scale_scale3x(uint8_t *, uint32_t *, int, int, int, int, int);
void scale_scale4x(uint8_t *, uint32_t *, int, int, int, int, int);


// Copy a pixel from src to dst
void pixel_copy(uint8_t * dst, uint32_t dpos, uint8_t * src, uint32_t spos, int bpp)
{
    int i;
    for(i=0; i < bpp; i++)
        dst[dpos + i] = src[spos + i];
}


// Check if two pixels are equal
// TODO: RGBA Alpha handling, ignore Alpha byte?
uint8_t pixel_eql(uint8_t * src, uint32_t pos0, uint32_t pos1, int bpp)
{
    int i;

    if (pos0 == pos1)
        return TRUE;

    for(i=0; i < bpp; i++)
        if (src[pos0 + i] != src[pos1 + i])
            return FALSE;

    return TRUE;
}

// Return adjacent pixel values for given pixel
void scale_scale2x(uint8_t * src, uint32_t * ret_pos, int x, int y, int w, int h, int bpp)
{
    int x0, x2;
    int y0, y2;
    uint32_t B, D, E, F, H;

    if (x > 0)
    {
        x0 = x - 1;
    }
    else
    {
        x0 = 0;
    }
    if (x < w - 1)
    {
        x2 = x + 1;
    }
    else
    {
        x2 = w - 1;
    }
    if (y > 0)
    {
        y0 = y - 1;
    }
    else
    {
        y0 = 0;
    }
    if (y < h - 1)
    {
        y2 = y + 1;
    }
    else
    {
        y2 = h - 1;
    }

    x0 *= bpp;
    x  *= bpp;
    x2 *= bpp;
    y0 *= bpp * w;
    y  *= bpp * w;
    y2 *= bpp * w;

    B = x  + y0;
    D = x0 + y;
    E = x  + y;
    F = x2 + y;
    H = x  + y2;

    if ((!pixel_eql(src, B, H, bpp)) && (!pixel_eql(src, D, F, bpp)))
    {

        if (pixel_eql(src, B, D, bpp))
        {
            ret_pos[0] = D;
        }
        else
        {
            ret_pos[0] = E;
        }
        if (pixel_eql(src, B, F, bpp))
        {
            ret_pos[1] = F;
        }
        else
        {
            ret_pos[1] = E;
        }
        if (pixel_eql(src, H, D, bpp))
        {
            ret_pos[2] = D;
        }
        else
        {
            ret_pos[2] = E;
        }
        if (pixel_eql(src, H, F, bpp))
        {
            ret_pos[3] = F;
        }
        else
        {
            ret_pos[3] = E;
        }
    }
    else
    {
        ret_pos[0] = ret_pos[1] = ret_pos[2] = ret_pos[3] = E;
    }
}


void scale_scale3x(uint8_t * src, uint32_t * ret_pos, int x, int y, int w, int h, int bpp)
{
    int x0, x2;
    int y0, y2;
    uint32_t A, B, C, D, E, F, G, H, I;
    uint8_t  D_B, D_H, F_B, F_H, E_A, E_G, E_C, E_I;

    if (x > 0)
    {
        x0 = x - 1;
    }
    else
    {
        x0 = 0;
    }
    if (x < w - 1)
    {
        x2 = x + 1;
    }
    else
    {
        x2 = w - 1;
    }
    if (y > 0)
    {
        y0 = y - 1;
    }
    else
    {
        y0 = 0;
    }
    if (y < h - 1)
    {
        y2 = y + 1;
    }
    else
    {
        y2 = h - 1;
    }

    x0 *= bpp;
    x  *= bpp;
    x2 *= bpp;
    y0 *= bpp * w;
    y  *= bpp * w;
    y2 *= bpp * w;


    A = x0 + y0;
    B = x  + y0;
    C = x2 + y0;
    D = x0 + y;
    E = x  + y;
    F = x2 + y;
    G = x0 + y2;
    H = x  + y2;
    I = x2 + y2;

    if ((!pixel_eql(src, B, H, bpp)) && (!pixel_eql(src, D, F, bpp)))
    {
        D_B = pixel_eql(src, D, B, bpp);
        D_H = pixel_eql(src, D, H, bpp);
        F_B = pixel_eql(src, F, B, bpp);
        F_H = pixel_eql(src, F, H, bpp);

        E_A = pixel_eql(src, E, A, bpp);
        E_G = pixel_eql(src, E, G, bpp);
        E_C = pixel_eql(src, E, C, bpp);
        E_I = pixel_eql(src, E, I, bpp);

        if (D_B)
        {
            ret_pos[0]  = D;
        }
        else
        {
            ret_pos[0] = E;
        }

        if ((D_B && (!E_C)) || (F_B && (!E_A)))
            ret_pos[1] = B;
        else
            ret_pos[1] = E;

        if (F_B)
        {
            ret_pos[2]  = F;
        }
        else
        {
            ret_pos[2] = E;
        }

        if ((D_B && (!E_G)) || (D_H && (!E_A)))
            ret_pos[3] = D;
        else
            ret_pos[3] = E;

        ret_pos[4] = E;

        if ((F_B && (!E_I)) || (F_H && (!E_C)))
            ret_pos[5] = F;
        else
            ret_pos[5] = E;

        if (D_H)
        {
            ret_pos[6]  = D;
        }
        else
        {
            ret_pos[6] = E;
        }

        if ((D_H && (!E_I)) || (F_H && (!E_G)))
            ret_pos[7] = H;
        else
            ret_pos[7] = E;

        if (F_H)
        {
            ret_pos[8]  = F;
        }
        else
        {
            ret_pos[8] = E;
        }
    }
    else
    {
        ret_pos[0] = ret_pos[1] = ret_pos[2] = ret_pos[3] = E;
        ret_pos[4] = ret_pos[5] = ret_pos[6] = ret_pos[7] = ret_pos[8] = E;
    }

}



// scaler_scalex_2x
//
// Scales image in *sp up by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 2 * Xres * 2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_scalex_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    int       bpp;
    int       x, y;
    uint32_t  pos;
    uint32_t  return_pos[4];
    uint8_t * src, * dst;

    bpp = BYTE_SIZE_RGBA_4BPP;  // Assume 4BPP RGBA
    src = (uint8_t *) sp;
    dst = (uint8_t *) dp;


    for (y=0; y < Yres; y++)
        for (x=0; x < Xres; x++)
        {
            scale_scale2x(src, &return_pos[0], x, y, Xres, Yres, bpp);

            pos = (4 * y * Xres + 2 * x) * bpp;
            pixel_copy(dst, pos,       src, return_pos[0], bpp);
            pixel_copy(dst, pos + bpp, src, return_pos[1], bpp);

            pos += 2 * Xres * bpp;
            pixel_copy(dst, pos,       src, return_pos[2], bpp);
            pixel_copy(dst, pos + bpp, src, return_pos[3], bpp);
        }
}



// scaler_scalex_3x
//
// Scales image in *sp up by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 3 * Xres * 3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_scalex_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    int       bpp;
    int       x, y;
    uint32_t  pos;
    uint32_t  return_pos[9];
    uint8_t * src, * dst;

    bpp = BYTE_SIZE_RGBA_4BPP;  // Assume 4BPP RGBA
    src = (uint8_t *) sp;
    dst = (uint8_t *) dp;

    for (y=0; y < Yres; y++)
        for (x=0; x < Xres; x++)
        {
            scale_scale3x(src, &return_pos[0], x, y, Xres, Yres, bpp);


            pos = (9 * y * Xres + 3 * x) * bpp;
            pixel_copy(dst, pos,           src, return_pos[0], bpp);
            pixel_copy(dst, pos + bpp,     src, return_pos[1], bpp);
            pixel_copy(dst, pos + 2 * bpp, src, return_pos[2], bpp);

            pos += 3 * Xres * bpp;
            pixel_copy(dst, pos,           src, return_pos[3], bpp);
            pixel_copy(dst, pos + bpp,     src, return_pos[4], bpp);
            pixel_copy(dst, pos + 2 * bpp, src, return_pos[5], bpp);

            pos += 3 * Xres * bpp;
            pixel_copy(dst, pos,           src, return_pos[6], bpp);
            pixel_copy(dst, pos + bpp,     src, return_pos[7], bpp);
            pixel_copy(dst, pos + 2 * bpp, src, return_pos[8], bpp);

        }
}



// scaler_scalex_4x
//
// 4x is just the 2x scaler run twice
// Scales image in *sp up by 4x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 4 * Xres * 4 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void scaler_scalex_4x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    long       buffer_size_bytes_2x;
    uint32_t * p_tempbuf;

    // Apply the first 2x scaling
    scaler_scalex_2x(sp, dp, Xres, Yres);

    // Copy the 2x scaled image into a temp buffer
    // then scale it up 2x again
    buffer_size_bytes_2x = Xres * 2 * Yres * 2 * BYTE_SIZE_RGBA_4BPP;
    p_tempbuf = malloc(buffer_size_bytes_2x);

    memcpy(p_tempbuf, dp, buffer_size_bytes_2x);

    // Apply the second 2x scaling
    scaler_scalex_2x(p_tempbuf, dp, Xres * 2, Yres * 2);

    free(p_tempbuf);
}



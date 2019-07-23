/*
 * common_ris.c
 *
 * ===============================
 *
 * Reverse Interpolate Scale (RIS)
 * (common code)
 *
 * ===============================
 *
 * Author: 2016 zvezdochiot (mykaralw@yandex.ru)
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 */

#include "common_ris.h"

uint8_t ByteClamp(int c)
{
    uint8_t buff[3] = {(uint8_t)c, 255, 0};
    return buff[ (c < 0) + ((unsigned)c > 255) ];
}

ARGBpixel ARGBtoPixel(uint32_t targb)
{
    ARGBpixel tp;
    tp.c[0] = (uint8_t)((targb & maskA) >> 24);
    tp.c[1] = (uint8_t)((targb & maskB) >> 16);
    tp.c[2] = (uint8_t)((targb & maskG) >> 8);
    tp.c[3] = (uint8_t)(targb & maskR);

    return tp;
}

uint32_t PixeltoARGB(ARGBpixel tp)
{
    uint32_t targb;

    targb = tp.c[0] << 24 | tp.c[1] << 16 | tp.c[2] << 8 | tp.c[3];

    return targb;
}

ARGBpixel threshold_bimod (uint32_t *sp, int Xres, int Yres)
{
    unsigned k, n, d, im;
    double T, Tw, Tb, Tn, iw, ib;
    uint32_t pt;
    ARGBpixel p, threshold;
    n = Xres * Yres;

    for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
    {
        T = 127.5;
        Tn = 0.0;
        while ( T != Tn )
        {
            Tn = T;
            Tb = 0.0;
            Tw = 0.0;
            ib = 0.0;
            iw = 0.0;
            for (k = 0; k < n; k++ )
            {
                pt = *(sp + k);
                p = ARGBtoPixel(pt);
                im = p.c[d];
                if (im > T)
                {
                    Tw += im;
                    iw++;
                } else {
                    Tb += im;
                    ib++;
                }
            }
            if (iw == 0.0 && ib == 0.0)
            {
                T = Tn;
            } else if (iw == 0.0) {
                T = Tb/ib;
            } else if (ib == 0.0) {
                T = Tw/iw;
            } else {
                T = ((Tw/iw) + (Tb/ib)) / 2.0;
            }
        }
        threshold.c[d] = ByteClamp((int)(T + 0.5));
    }

    return threshold;
}

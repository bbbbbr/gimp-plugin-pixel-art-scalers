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

/*
 * Author: 2016 zvezdochiot (mykaralw@yandex.ru)
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 *   This work has been identified as being free of known restrictions
 * under copyright law, including all related and neighboring rights.
 *
 *
 *   You can copy, modify, distribute and perform the work, even for
 * commercial purposes, all without asking permission. See Other
 * Information below.
 *
 * Other Information
 *
 *   The work may not be free of known copyright restrictions in all
 * jurisdictions.
 *   Persons may have other rights in or related to the work, such as
 * patent or trademark rights, and others may have rights in how the
 * work is used, such as publicity or privacy rights.
 *   In some jurisdictions moral rights of the author may persist beyond
 * the term of copyright. These rights may include the right to be
 * identified as the author and the right to object to derogatory treatments.
 *   Unless expressly stated otherwise, the person who identified the
 * work makes no warranties about the work, and disclaims liability for
 * all uses of the work, to the fullest extent permitted by applicable law.
 *   When using or citing the work, you should not imply endorsement by
 * the author or the person who identified the work.
 */

#ifndef __HRIS_H_
#define __HRIS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define maskA  0xFF000000
#define maskR  0x00FF0000
#define maskG  0x0000FF00
#define maskB  0x000000FF
#define BYTE_SIZE_RGBA_4BPP  4
#define SCALE_HRIS_2X  2
#define SCALE_HRIS_3X  3

typedef struct mpixel
{
    uint8_t c[4];
} ARGBpixel;

void scaler_hris_2x(uint32_t *src,  uint32_t *dest, int width, int height);
void scaler_hris_3x(uint32_t *src,  uint32_t *dest, int width, int height);

#endif //__HRIS_H_//

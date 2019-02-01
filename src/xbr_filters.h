/*
 * XBR filter: from the FFmpeg project
 *
 * Copyright (c) 2011, 2012 Hyllian/Jararaca <sergiogdb@gmail.com>
 * Copyright (c) 2014 Arwa Arif <arwaarif1994@gmail.com>
 * Copyright (c) 2015 Treeki <treeki@gmail.com>
 *
 *
 * hqx filter: from the hqx project
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
 * Copyright (c) 2015 Treeki <treeki@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __LIBXBR_FILTERS_H_INCLUDED
#define __LIBXBR_FILTERS_H_INCLUDED

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
	#define XBR_INLINE __inline

	#ifdef XBR_INTERNAL
		#define XBR_EXPORT __declspec(dllexport)
	#else
		#define XBR_EXPORT __declspec(dllimport)
	#endif
#else
	#define XBR_INLINE inline
	#define XBR_EXPORT
#endif


/*
typedef struct {
    uint32_t rgbtoyuv[1<<24]; // ~16 Megabytes of lookup table
} xbr_data;


typedef struct {
    const uint8_t *input;
    uint8_t *output;
    int inWidth, inHeight;
    int inPitch, outPitch;
    const xbr_data *data;
} xbr_params;
*/

/*
XBR_EXPORT void xbr_filter_xbr2x(const xbr_params *ctx);
XBR_EXPORT void xbr_filter_xbr3x(const xbr_params *ctx);
XBR_EXPORT void xbr_filter_xbr4x(const xbr_params *ctx);
*/

XBR_EXPORT void xbr_filter_xbr2x(uint32_t*, uint32_t*, int, int);
XBR_EXPORT void xbr_filter_xbr3x(uint32_t*, uint32_t*, int, int);
XBR_EXPORT void xbr_filter_xbr4x(uint32_t*, uint32_t*, int, int);


//XBR_EXPORT void xbr_init_data(xbr_data *data);
XBR_EXPORT void xbr_init_data();


#ifdef __cplusplus
}
#endif

#endif


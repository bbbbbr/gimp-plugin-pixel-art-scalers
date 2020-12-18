//
// libpas.h
//

// ========================
//
// Pixel art scaler
//
// ========================


#ifndef __LIBPAS_H_
#define __LIBPAS_H_

#include <stdint.h>

void hqxInit(void);
void hq2x_32( uint32_t * src, uint32_t * dest, int width, int height );
void hq3x_32( uint32_t * src, uint32_t * dest, int width, int height );
void hq4x_32( uint32_t * src, uint32_t * dest, int width, int height );

void hq2x_32_rb( uint32_t * src, uint32_t src_rowBytes, uint32_t * dest, uint32_t dest_rowBytes, int width, int height );
void hq3x_32_rb( uint32_t * src, uint32_t src_rowBytes, uint32_t * dest, uint32_t dest_rowBytes, int width, int height );
void hq4x_32_rb( uint32_t * src, uint32_t src_rowBytes, uint32_t * dest, uint32_t dest_rowBytes, int width, int height );

void scaler_nearest_2x(uint32_t *,  uint32_t *, int, int);
void scaler_nearest_3x(uint32_t *,  uint32_t *, int, int);
void scaler_nearest_4x(uint32_t *,  uint32_t *, int, int);

void scaler_scalex_2x(uint32_t *,  uint32_t *, int, int);
void scaler_scalex_3x(uint32_t *,  uint32_t *, int, int);
void scaler_scalex_4x(uint32_t *,  uint32_t *, int, int);

void xbr_filter_xbr2x(uint32_t*, uint32_t*, int, int);
void xbr_filter_xbr3x(uint32_t*, uint32_t*, int, int);
void xbr_filter_xbr4x(uint32_t*, uint32_t*, int, int);

void xbr_init_data(void);

#endif //END __LIBPAS_H_ //

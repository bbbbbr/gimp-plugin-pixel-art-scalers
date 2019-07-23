//
// filter_scalers.h
//

#ifndef __FILTER_SCALERS_H_
#define __FILTER_SCALERS_H_


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>


#define BYTE_SIZE_RGBA_4BPP 4
#define BYTE_SIZE_RGB_3BPP  3

#define ALPHA_MASK_OPAQUE   0xFF  // When adding alpha mask byte, set to 100% opaque / visible

#define SCALER_STR_MAX      30


// List of available scalers
// Order here controls order in dialog drop-down selector
enum scalev_list
{
    SCALEV_ENUM_FIRST = 0,

    SCALEV_2X = SCALEV_ENUM_FIRST,
    SCALEV_3X,
    SCALEV_4X,

    SCALEV_ENUM_LAST
};

enum scaler_list
{
    SCALER_ENUM_FIRST = 0,

    SCALER_HQX = SCALER_ENUM_FIRST,
    SCALER_HRIS,
    SCALER_XBR,
    SCALER_SCALEX,
    SCALER_GSAMPLE,
    SCALER_NEAREST,
    SCALER_MEAN,
    SCALER_REDUCE,

    SCALER_ENUM_LAST
};

typedef struct
{
    void (*function)(uint32_t*, uint32_t*, int, int);
    int  factor;
    char name[SCALER_STR_MAX];
} scaler_factor_info;

typedef struct
{
    scaler_factor_info scale[SCALEV_ENUM_LAST];
    char scaler_name[SCALER_STR_MAX];
    int direction;
} scaler_info;

typedef struct
{
    gint       x,y;
    gint       width, height;
    gint       scale_factor;
    gint       scaler_mode;
    gint       bpp;
    glong      size_bytes; // scaledbuf_size;
    gboolean   valid_image;
    gint       direction;

    uint32_t * p_scaledbuf;
} scaled_output_info;

const char * scaler_name_get(gint);
const char * scaler_namevalue_get(gint);

gint scaler_size(gint, gint, gint);
void scaler_mode_set(gint);
gint scaler_mode_get(void);
void scaler_factor_set(gint);
gint scaler_factor_index_get(void);
gint scaler_factor_get(void);
gint scaler_direction_get(void);

scaled_output_info * scaled_info_get(void);

void scalers_init(void);
void pixel_art_scalers_release_resources(void);
void scaler_apply(int, int,  uint32_t *, uint32_t *, int, int);

gint scaled_output_check_reapply_scalers(gint, gint, gint, gint, gint);
void scaled_output_check_reallocate(gint, gint, gint, gint);

void scaled_output_init(void);
void buffer_add_alpha_byte(guchar *, glong);
void buffer_remove_alpha_byte(guchar *, glong);

#endif

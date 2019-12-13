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

    #define ALPHA_PIXEL_REPLACE_THRESHOLD_DEFAULT  254
    #define ALPHA_PIXEL_REPLACE_VALUE_BELOW        0
    #define ALPHA_PIXEL_REPLACE_VALUE_ABOVE        255

    #define HIDDEN_PIXEL_BLEND_WEIGHT_ADJACENT 4
    #define HIDDEN_PIXEL_BLEND_WEIGHT_DIAGONAL 1

    #define HIDDEN_PIXEL_ALPHA_THRESHOLD_DEFAULT   32 // 0 works in many cases. Don't use colors at or below this threshold

    #define BORDER_STR_MAX      40
    #define BORDER_NO    0
    #define BORDER_DEF   2
    #define TILE_NO      0
    #define TILE_YES     1


    // List of available scalers
    // Order here controls order in dialog drop-down selector
    enum scaler_list {
        SCALER_ENUM_FIRST = 0,

        SCALER_2X_HQX = SCALER_ENUM_FIRST,
        SCALER_2X_XBR,
        SCALER_2X_SCALEX,
        SCALER_2X_NEAREST,

        SCALER_3X_HQX,
        SCALER_3X_XBR,
        SCALER_3X_SCALEX,
        SCALER_3X_NEAREST,

        SCALER_4X_HQX,
        SCALER_4X_XBR,
        SCALER_4X_SCALEX,
        SCALER_4X_NEAREST,

        SCALER_ENUM_LAST
    };

    // List of available scalers
    // Order here controls order in dialog drop-down selector
    enum border_options_list {
        BORDER_ENUM_FIRST = 0,

        BORDER_NONE = BORDER_ENUM_FIRST,

        BORDER_EMPTY_ALL,
        BORDER_EMPTY_VERT,
        BORDER_EMPTY_HORIZ,

        BORDER_TILE_ALL,
        BORDER_TILE_VERT,
        BORDER_TILE_HORIZ,

        BORDER_ENUM_LAST
    };


    typedef struct {
        void (*scaler_function)(uint32_t*, uint32_t*, int, int);
        int  scale_factor;
        char scaler_name[SCALER_STR_MAX];
    } scaler_info;


    typedef struct {
        gint       x,y;
        gint       width, height;
        gint       scale_factor;
        gint       scaler_mode;
        gint       bpp;
        glong      size_bytes; // scaledbuf_size;
        gboolean   valid_image;

        uint32_t * p_imagebuf;
    } image_info;


    typedef struct {
        char    name[BORDER_STR_MAX];
        guchar  border_x;
        guchar  border_y;
        guchar  tile_horiz;
        guchar  tile_vert;
    } border_info;

    const char * scaler_name_get(gint);
    gint scaler_scale_factor_get(gint);

    void scaler_mode_set(gint);
    gint scaler_mode_get(void);


    void border_mode_set(gint);
    gint border_mode_get(void);
    border_info border_options_get(void);
    const char * border_mode_name_get(gint mode);

    image_info * scaled_info_get(void);

    void scalers_init(void);
    void pixel_art_scalers_release_resources(void);
    void scaler_apply(int, uint32_t *, uint32_t *, int, int);

    void scaled_output_invalidate(void);
    gint scaled_output_check_reapply_scalers(gint, image_info);
    void scaled_output_check_reallocate(gint, image_info);

    void image_info_init(image_info *);

#endif

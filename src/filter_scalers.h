// filter_scalers.h

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


    enum scaler_list {
        SCALER_ENUM_FIRST = 0,

        SCALER_2X_HQX = SCALER_ENUM_FIRST,
        SCALER_2X_XBR,
        SCALER_2X_SCALEX,

        SCALER_3X_HQX,
        SCALER_3X_XBR,
        SCALER_3X_SCALEX,

        SCALER_4X_HQX,
        SCALER_4X_XBR,
        SCALER_4X_SCALEX,

        SCALER_ENUM_LAST
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

        uint32_t * p_scaledbuf;
    } scaled_output_info;

    const char * scaler_name_get(gint);
    gint scaler_scale_factor_get(gint);

    void scaler_mode_set(gint);
    gint scaler_mode_get(void);

    scaled_output_info * scaled_info_get(void);

    void scalers_init(void);
    void pixel_art_scalers_release_resources(void);
    void scaler_apply(int, uint32_t *, uint32_t *, int, int);

    gint scaled_output_check_reapply_scalers(gint, gint, gint);
    void scaled_output_check_reallocate(gint, gint, gint);

    void scaled_output_init(void);
    void buffer_add_alpha_byte(guchar *, glong);
    void buffer_remove_alpha_byte(guchar *, glong);

#endif

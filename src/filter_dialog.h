#ifndef __FILTER_DIALOG_H_
#define __FILTER_DIALOG_H_

    #include <stdint.h>


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

    gboolean pixel_art_scalers_dialog (GimpDrawable *drawable);
    void     pixel_art_scalers_release_resources(void);
    void     pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview);

#endif
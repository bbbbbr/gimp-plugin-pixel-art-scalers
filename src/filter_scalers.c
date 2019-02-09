// filter_scalers.c

#include "filter_scalers.h"

// Filter includes
#include "hqx.h"
#include "xbr_filters.h"
#include "filter_scalex.h"

static scaler_info scalers[SCALER_ENUM_LAST];

static scaled_output_info scaled_output;
static gint scaler_mode;



const char * scaler_name_get(gint scaler_index) {

    return (scalers[scaler_index].scaler_name);
}


gint scaler_scale_factor_get(gint scaler_index) {

    return (scalers[scaler_mode].scale_factor);
}


void scaler_mode_set(gint scaler_mode_new) {
    scaler_mode = scaler_mode_new;
}


gint scaler_mode_get() {
    return (scaler_mode);
}


scaled_output_info * scaled_info_get() {
    return &scaled_output;
}


gint scaled_output_check_reapply_scalers(gint scaler_mode_new, gint x_new, gint y_new) {

  gint result;

  result = ((scaled_output.scaler_mode != scaler_mode_new) ||
            (scaled_output.x != x_new) ||
            (scaled_output.y != y_new) ||
            (scaled_output.valid_image == FALSE));

  scaled_output.x = x_new;
  scaled_output.y = y_new;

  return (result);
}


// Update output buffer size and re-allocate if needed
void scaled_output_check_reallocate(gint scale_factor_new, gint width_new, gint height_new)
{
    if ((scale_factor_new != scaled_output.scale_factor) ||
        ((width_new  * scale_factor_new) != scaled_output.width) ||
        ((height_new * scale_factor_new) != scaled_output.height) ||
        (scaled_output.p_scaledbuf == NULL)) {

        // Update the buffer size and re-allocate. The x uint32_t is for RGBA buffer size
        scaled_output.width        = width_new  * scale_factor_new;
        scaled_output.height       = height_new * scale_factor_new;
        scaled_output.scale_factor = scale_factor_new;
        scaled_output.size_bytes = scaled_output.width * scaled_output.height * BYTE_SIZE_RGBA_4BPP;

        if (scaled_output.p_scaledbuf)
            g_free(scaled_output.p_scaledbuf);

        // 32 bit to ensure alignment, divide size since it's in BYTES
        scaled_output.p_scaledbuf = (uint32_t *) g_new (guint32, scaled_output.size_bytes / BYTE_SIZE_RGBA_4BPP);

        // Invalidate the image
        scaled_output.valid_image = FALSE;
    }
}


void scaled_output_init()
{
      scaled_output.p_scaledbuf  = NULL;
      scaled_output.width        = 0;
      scaled_output.height       = 0;
      scaled_output.x            = 0;
      scaled_output.y            = 0;
      scaled_output.scale_factor = 0;
      scaled_output.scaler_mode  = 0;
      scaled_output.size_bytes   = 0;
      scaled_output.bpp          = 0;
      scaled_output.valid_image  = FALSE;
}




void scaler_apply(int scaler_mode, uint32_t * p_srcbuf, uint32_t * p_destbuf, int width, int height) {

    if ((p_srcbuf == NULL) || (p_destbuf == NULL))
        return;

    if (scaler_mode < SCALER_ENUM_LAST) {

        // Call the requested scaler function
        scalers[scaler_mode].scaler_function( (uint32_t*) p_srcbuf,
                                              (uint32_t*) p_destbuf,
                                                    (int) width,
                                                    (int) height);
        scaled_output.bpp = BYTE_SIZE_RGBA_4BPP;
        scaled_output.scaler_mode = scaler_mode;
        scaled_output.valid_image = TRUE;
    }
}



void buffer_add_alpha_byte(guchar * p_srcbuf, glong srcbuf_size) {

    // Iterates through the buffer backward, from END to START
    // and remaps from RGB to RGBA (adds alpha byte)
    // --> Copies the buffer on top of itself, so ORDER IS IMPORTANT!

    guchar * p_3bpp = p_srcbuf + ((srcbuf_size / 4) *3) - 3; // Last pixel of 3BPP buffer
    guchar * p_4bpp = p_srcbuf + srcbuf_size -  4;          // Last pixel of 4BPP Buffer
    glong idx = 0;

    while(idx < srcbuf_size) {
        p_4bpp[3] = ALPHA_MASK_OPAQUE;  // Set alpha mask byte to 100% opaque / visible
        p_4bpp[2] = p_3bpp[2]; // copy B
        p_4bpp[1] = p_3bpp[1]; // copy G
        p_4bpp[0] = p_3bpp[0]; // copy R

        p_3bpp -= 3;  // Advance 3BPP image pointer to previous pixel
        p_4bpp -= 4;  // Advance 4BPP image pointer to previous pixel
        idx += 4;
    }
}



void buffer_remove_alpha_byte(guchar * p_srcbuf, glong srcbuf_size) {

    // Iterates through the buffer forward, from START to END
    // and remaps from RGBA to RGB (removes alpha byte)
    // --> Copies the buffer on top of itself, so ORDER IS IMPORTANT!

    guchar * p_3bpp = p_srcbuf; // Last First of 3BPP buffer
    guchar * p_4bpp = p_srcbuf; // Last First of 4BPP Buffer
    glong idx = 0;

    while(idx < srcbuf_size) {
          p_3bpp[0] = p_4bpp[0]; // copy R
          p_3bpp[1] = p_4bpp[1]; // copy G
          p_3bpp[2] = p_4bpp[2]; // copy B

        p_3bpp += 3;  // Advance 3BPP image pointer to next pixel
        p_4bpp += 4;  // Advance 4BPP image pointer to next pixel
        idx += 4;
    }
}



// Release the scaled output buffer
void pixel_art_scalers_release_resources() {

  if (scaled_output.p_scaledbuf)
      g_free(scaled_output.p_scaledbuf);
}



void scalers_init(void) {

    // Init HQX scaler library
    hqxInit();
    xbr_init_data();
    scaled_output_init();

    // HQX
    scalers[SCALER_2X_HQX].scaler_function = &hq2x_32;
    scalers[SCALER_2X_HQX].scale_factor    = 2;
    snprintf(scalers[SCALER_2X_HQX].scaler_name, SCALER_STR_MAX, "2x HQx");

    scalers[SCALER_3X_HQX].scaler_function = &hq3x_32;
    scalers[SCALER_3X_HQX].scale_factor    = 3;
    snprintf(scalers[SCALER_3X_HQX].scaler_name, SCALER_STR_MAX, "3x HQx");

    scalers[SCALER_4X_HQX].scaler_function = &hq4x_32;
    scalers[SCALER_4X_HQX].scale_factor    = 4;
    snprintf(scalers[SCALER_4X_HQX].scaler_name, SCALER_STR_MAX, "4x HQx");


    // XBR
    scalers[SCALER_2X_XBR].scaler_function = &xbr_filter_xbr2x;
    scalers[SCALER_2X_XBR].scale_factor    = 2;
    snprintf(scalers[SCALER_2X_XBR].scaler_name, SCALER_STR_MAX, "2x XBR");

    scalers[SCALER_3X_XBR].scaler_function = &xbr_filter_xbr3x;
    scalers[SCALER_3X_XBR].scale_factor    = 3;
    snprintf(scalers[SCALER_3X_XBR].scaler_name, SCALER_STR_MAX, "3x XBR");

    scalers[SCALER_4X_XBR].scaler_function = &xbr_filter_xbr4x;
    scalers[SCALER_4X_XBR].scale_factor    = 4;
    snprintf(scalers[SCALER_4X_XBR].scaler_name, SCALER_STR_MAX, "4x XBR");


    // SCALEX
    scalers[SCALER_2X_SCALEX].scaler_function = &filter_scalex_2x;
    scalers[SCALER_2X_SCALEX].scale_factor    = 2;
    snprintf(scalers[SCALER_2X_SCALEX].scaler_name, SCALER_STR_MAX, "2x ScaleX");

    scalers[SCALER_3X_SCALEX].scaler_function = &filter_scalex_3x;
    scalers[SCALER_3X_SCALEX].scale_factor    = 3;
    snprintf(scalers[SCALER_3X_SCALEX].scaler_name, SCALER_STR_MAX, "3x ScaleX");

    scalers[SCALER_4X_SCALEX].scaler_function = &filter_scalex_4x;
    scalers[SCALER_4X_SCALEX].scale_factor    = 4;
    snprintf(scalers[SCALER_4X_SCALEX].scaler_name, SCALER_STR_MAX, "4x ScaleX");


    // Now set the default scaler
    // TODO: accept last values for plugin so it remembers
    scaler_mode = SCALER_2X_HQX;
 }

//
// filter_scalers.c
//

// ========================
//
// Initializes scalers, applies them,
// some utility functions, resource cleanup
//
// ========================



#include "filter_scalers.h"

// Filter includes
#include "hqx.h"
#include "xbr_filters.h"
#include "scaler_scalex.h"
#include "scaler_nearestneighbor.h"

static scaler_info scalers[SCALER_ENUM_LAST];

static image_info scaled_output;
static gint scaler_mode;


// scaler_name_get
//
// Returns string name of a scaler
//
// scaler_index: desired scaler (from the enum scaler_list)
//
const char * scaler_name_get(gint scaler_index) {

    return (scalers[scaler_index].scaler_name);
}


// scaler_scale_factor_get
//
// Returns scale factor (x 2, x 3, etc) of a scaler
//
// scaler_index: desired scaler (from the enum scaler_list)
//
gint scaler_scale_factor_get(gint scaler_index) {

    return (scalers[scaler_mode].scale_factor);
}


// scaler_mode_set
//
// Sets the current scaler mode
// Preserves current mode across different callers
// (filter_dialog.c,  filter_pixel_art_scalers.c)
//
void scaler_mode_set(gint scaler_mode_new) {
    // Don't update the mode if it's outside the allowed range
    if ((scaler_mode_new >= SCALER_ENUM_FIRST) &&
        (scaler_mode_new < SCALER_ENUM_LAST))
        scaler_mode = scaler_mode_new;
}

// scaler_mode_get
//
// Returns the current scaler mode
//
gint scaler_mode_get(void) {
    return (scaler_mode);
}


// scaled_info_get
//
// Returns structure (type image_info)
// with details about the current rendered
// output image (scale mode, width, height, factor, etc)
//
// Used to assist with output caching
//
image_info * scaled_info_get(void) {
    return &scaled_output;
}


// scaled_output_invalidate
//
// Flags the scaled output as requiring a redraw
void scaled_output_invalidate(void) {
    scaled_output.valid_image = FALSE;
}


// scaled_output_check_reapply_scalers
//
// Checks whether the scaler needs to be re-applied
// depending on whether the scaler mode or
// x/y source image offset location have changed
//
// Used to assist with output caching
//
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


// scaled_output_check_reallocate
//
// Update output buffer size and re-allocate if needed
//
void scaled_output_check_reallocate(gint scale_factor_new, gint width_new, gint height_new)
{
    if ((scale_factor_new != scaled_output.scale_factor) ||
        ((width_new  * scale_factor_new) != scaled_output.width) ||
        ((height_new * scale_factor_new) != scaled_output.height) ||
        (scaled_output.p_imagebuf == NULL)) {

        // Update the buffer size and re-allocate. The x uint32_t is for RGBA buffer size
        scaled_output.width        = width_new  * scale_factor_new;
        scaled_output.height       = height_new * scale_factor_new;
        scaled_output.scale_factor = scale_factor_new;
        scaled_output.size_bytes = scaled_output.width * scaled_output.height * BYTE_SIZE_RGBA_4BPP;

        if (scaled_output.p_imagebuf)
            g_free(scaled_output.p_imagebuf);

        // 32 bit to ensure alignment, divide size since it's in BYTES
        scaled_output.p_imagebuf = (uint32_t *) g_new (guint32, scaled_output.size_bytes / BYTE_SIZE_RGBA_4BPP);

        // Invalidate the image
        scaled_output.valid_image = FALSE;
    }
}


// scaled_output_init
//
// Initialize rendered output shared structure
//
void image_info_init(image_info * p_image_info)
{
      p_image_info->p_imagebuf  = NULL;
      p_image_info->width        = 0;
      p_image_info->height       = 0;
      p_image_info->x            = 0;
      p_image_info->y            = 0;
      p_image_info->scale_factor = 0;
      p_image_info->scaler_mode  = 0;
      p_image_info->size_bytes   = 0;
      p_image_info->bpp          = 0;
      p_image_info->valid_image  = FALSE;
}



// scaler_apply
//
// Calls selected scaler function
// Updates valid_image to assist with caching
//
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


// buffer_add_alpha_byte
//
// Utility function to convert 3BPP RGB to 4BPP RGBA
// by inserting a fourth (alpha channel) byte after every 3 bytes
//
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


// buffer_remove_alpha_byte
//
// Utility function to convert 4BPP RGBA to 3BPP RGB
// by removing the fourth (alpha channel) byte after every 3 bytes
//
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


// Expands an image by N pixels on all edges
//
//
// Used for image border features (empty border, seamless tile assist)
//
// Typically operates on INPUT image
//
// NOTE: Expects 4bpp image, will abort if not
//
image_info buffer_grow_image_border (image_info * p_src_image, gint grow_size) {

    image_info new_image;
    glong      src_rowstride, new_rowstride;
    glong      c, y;
    uint32_t * p_pix;
    uint32_t * new_image_buf;


    // Require 4bpp (RGBA)
    if (p_src_image == NULL) return *p_src_image;
    if (p_src_image->bpp != BYTE_SIZE_RGBA_4BPP) return *p_src_image;

    // Copy source image data to new image structure
    new_image = *p_src_image;

    // Update size by adding growth size to top/bottom/left/right
    // Then calculate new buffer size
    new_image.width  += (grow_size * 2);
    new_image.height += (grow_size * 2);
    new_image.size_bytes = new_image.width * new_image.height * new_image.bpp;

    // 32 bit to ensure alignment, divide size since it's in BYTES
    new_image.p_imagebuf = (uint32_t *) g_new (guint32, new_image.size_bytes / BYTE_SIZE_RGBA_4BPP);

    // Zero out the new image buffer with Alpha= transparent, color = rgb(0,0,0), 0xFF000000
    p_pix = new_image.p_imagebuf;
    for(c = 0; c < new_image.size_bytes / BYTE_SIZE_RGBA_4BPP; c++) {
        *p_pix = 0x00000000;
        p_pix++;
    }

    // Copy the source image to the center of the new image buffer
    src_rowstride = (p_src_image->width * p_src_image->bpp);

    for (y = 0; y < p_src_image->height; y++) {
        // Offset the destination pointer for each line by the growth size in pixels
        // Operating on 32 bit pointer, so ignore bpp
        memcpy(new_image.p_imagebuf    + ((y + grow_size) * new_image.width) + grow_size,
               p_src_image->p_imagebuf + (y * p_src_image->width),
               src_rowstride);
    }


    // Release source image buffer
    if (p_src_image->p_imagebuf) {
        g_free(p_src_image->p_imagebuf);
        p_src_image->p_imagebuf = NULL;
    }

    // Return updated image to caller by copying
    // new image data into source structure
    printf("Old: %d,%d,%ld,%d\n", p_src_image->width, p_src_image->height, p_src_image->size_bytes, grow_size);
    printf("New: %d,%d,%ld,%d\n", new_image.width, new_image.height, new_image.size_bytes, grow_size);

    // Return updated image info and buffer
    return new_image;
}



// Expands an image by N pixels on all edges
//
//
// Used for image border features (empty border, seamless tile assist)
//
// Typically operates on INPUT image
//
// NOTE: Expects 4bpp image, will abort if not
//
image_info buffer_shrink_image_border (image_info * p_src_image, gint shrink_size) {

    image_info new_image;
    glong      src_rowstride, new_rowstride;
    glong      c, y;
    uint32_t * p_pix;
    uint32_t * new_image_buf;


    // Require 4bpp (RGBA)
    if (p_src_image == NULL) return *p_src_image;
    if (p_src_image->bpp != BYTE_SIZE_RGBA_4BPP) return *p_src_image;

    // Copy source image data to new image structure
    new_image = *p_src_image;

    // Update size by adding growth size to top/bottom/left/right
    // Then calculate new buffer size
    new_image.width  -= (shrink_size * 2);
    new_image.height -= (shrink_size * 2);
    new_image.size_bytes = new_image.width * new_image.height * new_image.bpp;

    // 32 bit to ensure alignment, divide size since it's in BYTES
    new_image.p_imagebuf = (uint32_t *) g_new (guint32, new_image.size_bytes / BYTE_SIZE_RGBA_4BPP);

/*
    // Zero out the new image buffer with Alpha= transparent, color = rgb(0,0,0), 0xFF000000
    p_pix = new_image.p_imagebuf;
    for(c = 0; c < new_image.size_bytes / BYTE_SIZE_RGBA_4BPP; c++) {
        *p_pix = 0x00000000;
        p_pix++;
    }
*/
    // Copy the source image to the center of the new image buffer
    new_rowstride = (new_image.width * new_image.bpp);

    for (y = 0; y < new_image.height; y++) {
        // Offset the destination pointer for each line by the growth size in pixels
        // Operating on 32 bit pointer, so ignore bpp
        memcpy(new_image.p_imagebuf    + (y * new_image.width),
               p_src_image->p_imagebuf + ((y + shrink_size) * p_src_image->width) + shrink_size,
               new_rowstride);
    }


    // Release source image buffer
    if (p_src_image->p_imagebuf) {
        g_free(p_src_image->p_imagebuf);
        p_src_image->p_imagebuf = NULL;
    }

    // Return updated image to caller by copying
    // new image data into source structure
    printf("Old: %d,%d,%ld,%d\n", p_src_image->width, p_src_image->height, p_src_image->size_bytes, shrink_size);
    printf("New: %d,%d,%ld,%d\n", new_image.width, new_image.height, new_image.size_bytes, shrink_size);

    // Return updated image info and buffer
    return new_image;

}

// Forces partially transparent pixels below
// a given threshold to entirely transparent.
//
// Used to prevent filters from adding anti-aliased edges.
//
// Typically operates on OUTPUT image
//
// NOTE: Expects 4bpp image, will abort if not
//
void buffer_remove_partial_alpha(guchar * p_buf, glong buf_size, gint bpp, guchar alpha_threshold, guchar replace_value_below, guchar replace_value_above) {

    // Require 4bpp (RGBA)
    if (bpp != BYTE_SIZE_RGBA_4BPP)
        return;

    // Iterates through the buffer from START to END
    while(buf_size) {

        // if ALPHA value is below threshold, force it to 0
        if (p_buf[3] <= alpha_threshold)
            p_buf[3] = replace_value_below;
        else
            p_buf[3] = replace_value_above;

        p_buf += 4;  // Advance image pointer to next pixel
        buf_size -= 4; // Decrenebt size counter
    }
}


// TODO: convert input to accept image_info * p_image

// Forces partially transparent pixels below
// a given threshold to entirely transparent.
//
// Used to prevent filters from adding anti-aliased edges.
//
// Typically operates on INPUT image
//
// NOTE: Expects 4bpp image, will abort if not
//
void buffer_set_alpha_hidden_to_adjacent_visible(guchar * p_buf, glong buf_size, gint bpp, gint width, gint height, guchar alpha_threshold) {

    gint       x,y;
    gint       adj_x,adj_y;
    gint       col[3];
    gint       col_count;
    gint       col_weight;
    guchar *   p_adj_px;

    // Require 4bpp (RGBA)
    if (bpp != BYTE_SIZE_RGBA_4BPP)
        return;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {

            // if pixel ALPHA value is below threshold, replace it's color
            if (p_buf[3] <= alpha_threshold) {


                // Reset color accumulator
                col[0] = col[1] = col[2] = 0;
                col_count = 0;

                // Check all adjacent pixels to see if
                // they have desired opacity to donate color
                for (adj_y = -1; adj_y <= 1; adj_y++) {
                    for (adj_x = -1; adj_x <= 1; adj_x++) {

                        // Stay within image bounds
                        if (((x + adj_x) >= 0) &&
                            ((x + adj_x) < width) &&
                            ((y + adj_y) >= 0) &&
                            ((y + adj_y) < height)) {

                            // Set pointer to the adjacent pixel
                            p_adj_px = p_buf + ((adj_x * bpp) + (adj_y * width * bpp));

                            // Accumulate it's color values
                            if (p_adj_px[3] > alpha_threshold)
                            {
                                // Weight directly adjacent colors twice
                                // as much as diaglonally adjacent
                                if ((adj_x == 0) || (adj_y == 0))
                                    col_weight = HIDDEN_PIXEL_BLEND_WEIGHT_ADJACENT;
                                else
                                    col_weight = HIDDEN_PIXEL_BLEND_WEIGHT_DIAGONAL;

                                col[0] += p_adj_px[0] * col_weight;
                                col[1] += p_adj_px[1] * col_weight;
                                col[2] += p_adj_px[2] * col_weight;
                                col_count += col_weight;

                                // col_count = 1;
                            }
                        }
                    }
                }

                // Set pixel to mix of neighboring pixel colors
                // TODO: improve color mixing algorithm
                if (col_count > 0) {
                    p_buf[0] = col[0] / col_count;
                    p_buf[1] = col[1] / col_count;
                    p_buf[2] = col[2] / col_count;
                }
            }

            p_buf += 4;  // Advance image pointer to next pixel
        } // for (y = 0; y < height; y++) {
    } // for (x = 0; x < width; x++) {
}



// pixel_art_scalers_release_resources
//
// Release the scaled output buffer.
// Should be called only at the very
// end of the plugin shutdown (not on dialog close)
//
void pixel_art_scalers_release_resources(void) {

  if (scaled_output.p_imagebuf)
      g_free(scaled_output.p_imagebuf);
}


// scalers_init
//
// Populate the shared list of available scalers with their names
// calling functions and scale factors.
//
void scalers_init(void) {

    // Init HQX scaler library
    hqxInit();
    xbr_init_data();
    image_info_init(&scaled_output);

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
    scalers[SCALER_2X_SCALEX].scaler_function = &scaler_scalex_2x;
    scalers[SCALER_2X_SCALEX].scale_factor    = 2;
    snprintf(scalers[SCALER_2X_SCALEX].scaler_name, SCALER_STR_MAX, "2x ScaleX");

    scalers[SCALER_3X_SCALEX].scaler_function = &scaler_scalex_3x;
    scalers[SCALER_3X_SCALEX].scale_factor    = 3;
    snprintf(scalers[SCALER_3X_SCALEX].scaler_name, SCALER_STR_MAX, "3x ScaleX");

    scalers[SCALER_4X_SCALEX].scaler_function = &scaler_scalex_4x;
    scalers[SCALER_4X_SCALEX].scale_factor    = 4;
    snprintf(scalers[SCALER_4X_SCALEX].scaler_name, SCALER_STR_MAX, "4x ScaleX");


    // NEAREST
    scalers[SCALER_2X_NEAREST].scaler_function = &scaler_nearest_2x;
    scalers[SCALER_2X_NEAREST].scale_factor    = 2;
    snprintf(scalers[SCALER_2X_NEAREST].scaler_name, SCALER_STR_MAX, "2x Nearest");

    scalers[SCALER_3X_NEAREST].scaler_function = &scaler_nearest_3x;
    scalers[SCALER_3X_NEAREST].scale_factor    = 3;
    snprintf(scalers[SCALER_3X_NEAREST].scaler_name, SCALER_STR_MAX, "3x Nearest");

    scalers[SCALER_4X_NEAREST].scaler_function = &scaler_nearest_4x;
    scalers[SCALER_4X_NEAREST].scale_factor    = 4;
    snprintf(scalers[SCALER_4X_NEAREST].scaler_name, SCALER_STR_MAX, "4x Nearest");

    // Now set the default scaler
    scaler_mode = SCALER_2X_HQX;
 }

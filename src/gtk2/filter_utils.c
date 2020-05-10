//
// filter_utils.c
//

// ========================
//
// Utility image processing functions
//
// ========================

#include "filter_scalers.h"

#include "filter_utils.h"


// TODO: convert input to accept image_info * p_image for : buffer_set_alpha_hidden_to_adjacent_visible, buffer_remove_partial_alpha
// TODO: convert to accept border_info for : buffer_tiled_edge_copy



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



// Fills an image border with tiled copies
// of a centered source image in the buffer
//
//
// Used for image border features (seamless tile assist)
//
// Typically operates on INPUT image
//
// NOTE: Expects 4bpp image, will abort if not
// NOTE: Expects a previously added image border via buffer_grow_image_border()
//
// TODO: memcopies are clunky, needs cleanup
//
void buffer_tiled_edge_copy (image_info * p_image, gint border_width, gint border_height, gint do_tile_x, gint do_tile_y) {

    gint  copy_bytes;
    gint  y;

    // Require 4bpp (RGBA)
    if (p_image == NULL) return;
    if (p_image->bpp != BYTE_SIZE_RGBA_4BPP) return;


    // Fill in top and bottom tiled edges
    if (do_tile_y) {
        copy_bytes = ((p_image->width - (border_width * 2)) * p_image->bpp);

        // Operating on 32 bit pointers, so ignore bpp for pointer offsets

        // Original Bottom Lines -> Copied Top Lines
        for (y = 0; y < border_height; y++) {
            memcpy(p_image->p_imagebuf + border_width + (y * p_image->width),
                   p_image->p_imagebuf + border_width + ((y + p_image->height - (border_width * 2)) * p_image->width), copy_bytes);
        }

        // Original Top Lines -> Copied Bottom Lines
        for (y = p_image->height - border_height; y < p_image->height; y++) {
            memcpy(p_image->p_imagebuf + border_width + (y * p_image->width),
                   p_image->p_imagebuf + border_width + ((y - (p_image->height - (border_width * 2))) * p_image->width), copy_bytes);
        }
    }


    // Fill in left and right tiled edges
    if (do_tile_x) {
        copy_bytes = border_width * p_image->bpp;

        // Operating on 32 bit pointers, so ignore bpp for pointer offsets

        for (y = border_height; y < p_image->height - border_height; y++) {
            // Original Right Edge -> Copied Left Edge
            memcpy(p_image->p_imagebuf + (y * p_image->width),
                   p_image->p_imagebuf + (y * p_image->width) + p_image->width - (border_width * 2), copy_bytes);

            // Original Left Edge -> Copied Right Edge
            memcpy(p_image->p_imagebuf + (y * p_image->width) + p_image->width - border_width,
                   p_image->p_imagebuf + (y * p_image->width) + border_width, copy_bytes);
        }
    }


    // If tiling in both directions, then fill in the diagonal corners
    if ((do_tile_y) && (do_tile_x)) {
        // Operating on 32 bit pointer, so ignore bpp

        copy_bytes = border_width * p_image->bpp;

        // Operating on 32 bit pointers, so ignore bpp for pointer offsets

        for (y = 0; y < border_height; y++) {
            // Original Lower Right Corner -> Copied Upper Left Corner
            memcpy(p_image->p_imagebuf + (y * p_image->width),
                   p_image->p_imagebuf + ((y + (p_image->height - (border_height * 2))) * p_image->width)
                                       + p_image->width - (border_width * 2),
                   copy_bytes);

            // Original Lower Left Corner -> Copied Upper Right Corner
            memcpy(p_image->p_imagebuf + (y * p_image->width)
                                       + p_image->width - border_width,
                   p_image->p_imagebuf + ((y + (p_image->height - (border_height * 2))) * p_image->width)
                                       + border_width,
                   copy_bytes);

            // Original Upper Right Corner -> Copied Lower Left Corner
            memcpy(p_image->p_imagebuf + ((y + (p_image->height - border_height)) * p_image->width),
                   p_image->p_imagebuf + ((y + border_height) * p_image->width)
                                       + p_image->width - (border_width * 2),
                   copy_bytes);

            // Original Upper Left Corner -> Copied Lower Right Corner
            memcpy(p_image->p_imagebuf + ((y + (p_image->height - border_height)) * p_image->width)
                                       + p_image->width - border_width,
                   p_image->p_imagebuf + ((y + border_height) * p_image->width)
                                       + border_width,
                   copy_bytes);

        }
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
image_info buffer_grow_image_border (image_info * p_src_image, gint grow_x, gint grow_y) {

    image_info new_image;
    glong      src_rowstride;
    glong      c, y;
    uint32_t * p_pix;


    // Require 4bpp (RGBA)
    if (p_src_image == NULL) return *p_src_image;
    if (p_src_image->bpp != BYTE_SIZE_RGBA_4BPP) return *p_src_image;

    // Copy source image data to new image structure
    new_image = *p_src_image;

    // Update size by adding growth size to top/bottom/left/right
    // Then calculate new buffer size
    new_image.width  += (grow_x * 2);
    new_image.height += (grow_y * 2);
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
        memcpy(new_image.p_imagebuf    + ((y + grow_y) * new_image.width) + grow_x,
               p_src_image->p_imagebuf + (y * p_src_image->width),
               src_rowstride);
    }


    // Release source image buffer
    if (p_src_image->p_imagebuf) {
        g_free(p_src_image->p_imagebuf);
        p_src_image->p_imagebuf = NULL;
    }

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
image_info buffer_shrink_image_border (image_info * p_src_image, gint shrink_x, gint shrink_y) {

    image_info new_image;
    glong      new_rowstride;
    glong      y;


    // Require 4bpp (RGBA)
    if (p_src_image == NULL) return *p_src_image;
    if (p_src_image->bpp != BYTE_SIZE_RGBA_4BPP) return *p_src_image;

    // Copy source image data to new image structure
    new_image = *p_src_image;

    // Update size by adding growth size to top/bottom/left/right
    // Then calculate new buffer size
    new_image.width  -= (shrink_x * 2);
    new_image.height -= (shrink_y * 2);
    new_image.size_bytes = new_image.width * new_image.height * new_image.bpp;

    // 32 bit to ensure alignment, divide size since it's in BYTES
    new_image.p_imagebuf = (uint32_t *) g_new (guint32, new_image.size_bytes / BYTE_SIZE_RGBA_4BPP);

    // Copy the source image to the center of the new image buffer
    new_rowstride = (new_image.width * new_image.bpp);

    for (y = 0; y < new_image.height; y++) {
        // Offset the destination pointer for each line by the growth size in pixels
        // Operating on 32 bit pointer, so ignore bpp
        memcpy(new_image.p_imagebuf    + (y * new_image.width),
               p_src_image->p_imagebuf + ((y + shrink_y) * p_src_image->width) + shrink_x,
               new_rowstride);
    }


    // Release source image buffer
    if (p_src_image->p_imagebuf) {
        g_free(p_src_image->p_imagebuf);
        p_src_image->p_imagebuf = NULL;
    }

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

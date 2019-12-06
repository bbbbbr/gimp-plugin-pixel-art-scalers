//
// filter_utils.c
//

// ========================
//
// Initializes scalers, applies them,
// some utility functions, resource cleanup
//
// ========================

#include "filter_scalers.h"

#include "filter_utils.h"



// Expands an image by N pixels on all edges
//
//
// Used for image border features (empty border, seamless tile assist)
//
// Typically operates on INPUT image
//
// NOTE: Expects 4bpp image, will abort if not
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
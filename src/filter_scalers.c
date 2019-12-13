//
// filter_scalers.c
//

// ========================
//
// Initializes scalers, applies them, resource cleanup
//
// ========================



#include "filter_scalers.h"

// Filter includes
#include "hqx.h"
#include "xbr_filters.h"
#include "scaler_scalex.h"
#include "scaler_nearestneighbor.h"

static scaler_info scalers[SCALER_ENUM_LAST];
static border_info border_types[BORDER_ENUM_LAST] =
          { {"None",      BORDER_NO,   BORDER_NO, TILE_NO, TILE_NO},

            {"Transparent  All",   BORDER_DEF, BORDER_DEF, TILE_NO, TILE_NO},
            {"Transparent  Horiz", BORDER_DEF, BORDER_NO,  TILE_NO, TILE_NO},
            {"Transparent  Vert",  BORDER_NO,  BORDER_DEF, TILE_NO, TILE_NO},

            {"Tile  All",    BORDER_DEF, BORDER_DEF, TILE_YES, TILE_YES},
            {"Tile  Horiz",  BORDER_DEF, BORDER_NO,  TILE_YES, TILE_NO },
            {"Tile  Vert",   BORDER_NO,  BORDER_DEF, TILE_NO,  TILE_YES} };


static image_info scaled_output;
static gint scaler_mode;
static gint border_mode;


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


// border_mode_set
//
// Sets border processing mode
//
void border_mode_set(gint new_mode) {

    if ((new_mode >= BORDER_ENUM_FIRST) && (new_mode < BORDER_ENUM_LAST))
        border_mode = new_mode;
}


// border_mode_get
//
// Returns border processing mode
//
gint border_mode_get(void) {

    return border_mode;
}


// border_mode_get
//
// Returns current morder mode settings (type border_info)
//
border_info border_options_get(void) {
    return border_types[border_mode];
}


// border_mode_name_get
//
// Returns current morder mode settings (type border_info)
//
const char * border_mode_name_get(gint mode) {
    return border_types[mode].name;
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
gint scaled_output_check_reapply_scalers(gint scaler_mode_new, image_info src_image) {

    gint result;
    gint scale_factor_new = scaler_scale_factor_get( scaler_mode_new );

    result = ((scaled_output.scaler_mode != scaler_mode_new) ||
             (scaled_output.x != src_image.x) ||
             (scaled_output.y != src_image.y) ||
             (scaled_output.width  != (src_image.width  * scale_factor_new)) ||
             (scaled_output.height != (src_image.height * scale_factor_new)) ||
             (scaled_output.valid_image == FALSE));

    scaled_output.x = src_image.x;
    scaled_output.y = src_image.y;

    if (result)
        scaled_output_invalidate();

    return (result);
}


// scaled_output_check_reallocate
//
// Update output buffer size and re-allocate if needed
//
void scaled_output_check_reallocate(gint scale_factor_new, image_info src_image)
{
    if ((scale_factor_new != scaled_output.scale_factor) ||
        ((src_image.width  * scale_factor_new) != scaled_output.width) ||
        ((src_image.height * scale_factor_new) != scaled_output.height) ||
        (scaled_output.p_imagebuf == NULL)) {

        // Update the buffer size and re-allocate. The x uint32_t is for RGBA buffer size
        scaled_output.width        = src_image.width  * scale_factor_new;
        scaled_output.height       = src_image.height * scale_factor_new;
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

    // Set the default processing mode
    border_mode = BORDER_NONE;
 }


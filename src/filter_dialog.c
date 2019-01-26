
//#include "config.h"
//#include <string.h>

#include <stdio.h>
#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "filter_dialog.h"

#include <stdint.h>
#include "hqx.h"


extern const char PLUG_IN_PROCEDURE[];
extern const char PLUG_IN_ROLE[];
extern const char PLUG_IN_BINARY[];

static void filter_apply (int, uint32_t *, uint32_t *, int, int);
static void copy_scaled_to_unscaled(guchar *, guchar *, guint, guint, guint, guint);

enum scaler_list {
    SCALER_HQ2X,
    SCALER_HQ3X,
    SCALER_HQ4X,
    SCALER_ENUM_LAST
};

typedef struct {
    void (*scaler_function)(uint32_t*, uint32_t*, int, int);
    int  scale_factor;
} scaler_info;

scaler_info scalers[SCALER_ENUM_LAST];


void scalers_init() {

    // Init HQX scaler library
    hqxInit();

    scalers[SCALER_HQ2X].scaler_function = &hq2x_32;
    scalers[SCALER_HQ2X].scale_factor    = 2;

    scalers[SCALER_HQ3X].scaler_function = &hq3x_32;
    scalers[SCALER_HQ3X].scale_factor    = 3;

    scalers[SCALER_HQ4X].scaler_function = &hq4x_32;
    scalers[SCALER_HQ4X].scale_factor    = 4;
 }


/*******************************************************/
/*                    Dialog                           */
/*******************************************************/

// TODO: there are probably better ways to do this than a global var
static   GtkWidget *preview_scaled;


gboolean pixel_art_scalers_dialog (GimpDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview_hbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkObject *scale_data;
  gboolean   run;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new ("Pixel Art Scalers", PLUG_IN_ROLE,
                            NULL, 0,
                            gimp_standard_help_func, PLUG_IN_PROCEDURE,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gimp_window_set_transient (GTK_WINDOW (dialog));


  // Create a main vertical box for the preview
  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);


  // Create a side-by-side sub-box for the pair of preview windows
  preview_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (preview_hbox), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox),
                      preview_hbox, TRUE, TRUE, 0);
  gtk_widget_show (preview_hbox);


  preview = gimp_drawable_preview_new (drawable, NULL);


  // Add source image preview area, set it to not expand if window grows
  gtk_box_pack_start (GTK_BOX (preview_hbox), preview, FALSE, TRUE, 0);
  gtk_widget_show (preview);
  // Wire up preview redraw to call the pixel scaler filter
  g_signal_connect_swapped (preview,
                            "invalidated",
                            G_CALLBACK (pixel_art_scalers_run),
                            drawable);


  // Add a scaled preview area
  preview_scaled = gimp_preview_area_new();
  // ---- // gtk_widget_set_size_request (preview_scaled, PREVIEW_SIZE, PREVIEW_SIZE);
   gtk_box_pack_start (GTK_BOX (preview_hbox), preview_scaled, TRUE, TRUE, 0);
   gtk_widget_show (preview_scaled);



  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);


/*
  // MASK RADIUS CONTROL / cvals.mask_radius
  //
  // Creates a GtkLabel, a GtkHScale and a GtkSpinButton
  // and attaches them to a 3-column GtkTable
  scale_data = gimp_scale_entry_new (GTK_TABLE (table), 0, 0,
                                     _("_Mask radius:"), 100, 5,
                                     cvals.mask_radius, 1.0, 50.0, 1, 5.0, 2,
                                     TRUE, 0, 0,
                                     NULL, NULL);

  // Connect updates from the control to the preview
  g_signal_connect (scale_data,
                    "value-changed",
                    G_CALLBACK (gimp_double_adjustment_update),
                    &cvals.mask_radius);

  g_signal_connect_swapped (scale_data,
                            "value-changed",
                            G_CALLBACK (gimp_preview_invalidate),
                            preview);

  // MASK RADIUS CONTROL / cvals.pct_black
  //
  // Creates a GtkLabel, a GtkHScale and a GtkSpinButton
  // and attaches them to a 3-column GtkTable.
  scale_data = gimp_scale_entry_new (GTK_TABLE (table), 0, 1,
                                     _("_Percent black:"), 50, 5,
                                     cvals.pct_black, 0.0, 1.0, 0.01, 0.1, 3,
                                     TRUE, 0, 0,
                                     NULL, NULL);

  // Connect updates from the control to the preview
  g_signal_connect (scale_data,
                    "value-changed",
                    G_CALLBACK (gimp_double_adjustment_update),
                    &cvals.pct_black);

  g_signal_connect_swapped (scale_data,
                            "value-changed",
                            G_CALLBACK (gimp_preview_invalidate),
                            preview);
*/

  gtk_widget_show (dialog);


  run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);


  gtk_widget_destroy (dialog);

  return run;
}





/*******************************************************/
/*                    APPLY SCALER                     */
/*******************************************************/

// TODO: This would be less brittle and easier to understand if
//       preview vs. apply was either seperated out or abstracted.


void pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview)
{
    GimpPixelRgn src_rgn, dest_rgn;
    gint         bpp, has_alpha;
    gint         width, height;

    gint         x, y;
    gint         alpha;

    uint32_t     * p_srcbuf = NULL;
    uint32_t     * p_scaledbuf = NULL;


    // Initialize the scalers
    scalers_init();

    // Get the working image area for either the preview sub-window or the entire image
    if (preview) {
        gimp_preview_get_position (preview, &x, &y);
        gimp_preview_get_size (preview, &width, &height);
    }
    else if (! gimp_drawable_mask_intersect (drawable->drawable_id,
                                             &x, &y, &width, &height)) {
        return;
    }

    // Get bit depth and alpha mask status
    bpp = drawable->bpp;
    has_alpha = gimp_drawable_has_alpha (drawable->drawable_id);
    alpha = (has_alpha) ? drawable->bpp - 1 : drawable->bpp;

    // Allocate a working buffer to copy the source image into
    p_srcbuf = (uint32_t *) g_new (guchar, width * height * bpp);

    // FALSE, FALSE : region will be used to read the actual drawable datas
    // Initialize source pixel region with drawable
    gimp_pixel_rgn_init (&src_rgn,
                         drawable,
                         x, y,
                         width, height,
                         FALSE, FALSE);


    // Copy source image to working buffer
    gimp_pixel_rgn_get_rect (&src_rgn,
                             (guchar *) p_srcbuf,
                             x, y, width, height);



    // BEGIN SCALER
    guint scaler_mode = SCALER_HQ4X;
    guint scale_factor = scalers[scaler_mode].scale_factor;

    // Allocate output buffer for the results
    // guchar = unsigned 8 bits, guint32 = unsigned 32 bits, uint32_t = unsigned 32 bits
    p_scaledbuf = (uint32_t *) g_new (guint, width * scale_factor * height * scale_factor * sizeof(uint32_t));

    if (p_scaledbuf) {

        // TODO: Careful! Making assumptions about p_srcbuf : bpp = 4 / uint32_t here
        filter_apply (scaler_mode,
                      p_srcbuf,
                      p_scaledbuf,
                      (int) width, (int) height);
    }


    // Filter is done, apply the update
    if (preview) {

        // RGBA 4pp assumption
        // Draw scaled image onto preview area
        gimp_preview_area_draw (GIMP_PREVIEW_AREA (preview_scaled),
                                0, 0,
                                width * scale_factor,
                                height * scale_factor,
                                GIMP_RGBA_IMAGE,
                                (guchar *) p_scaledbuf,
                                width * scale_factor * bpp);

        // Resize scaled preview area to full buffer size
        gtk_widget_set_size_request (preview_scaled, width * scale_factor, height * scale_factor);
    }
    else
    {

        // TODO: Fix assumption that image has alpha layer (BytesPerPixel = 4)
        // TRUE,  TRUE  : region will be used to write to the shadow tiles
        //                i.e. make changes that will be written back to source tiles
        // Initialize dest pixel region with drawable
        gimp_pixel_rgn_init (&dest_rgn,
                             drawable,
                             x, y,
                             width, height,
                             TRUE, TRUE);

        // TODO: REplace this with resizing canvas and pass
        copy_scaled_to_unscaled((guchar *) p_srcbuf,
                                (guchar *)p_scaledbuf,
                                width, height,
                                bpp, scale_factor);


        // Copy working buffer to the shadow image
        gimp_pixel_rgn_set_rect (&dest_rgn,
                                  (guchar *) p_srcbuf,
                                  x, y, width, height);

        // Apply the changes to the image (merge shadow, update drawable)
        gimp_drawable_flush (drawable);
        gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
        gimp_drawable_update (drawable->drawable_id, x, y, width, height);
    }

    // Free the working buffer
    g_free (p_srcbuf);

    if (p_scaledbuf)
        g_free(p_scaledbuf);
}



static void copy_scaled_to_unscaled(guchar * p_srcbuf, guchar * p_scaledbuf, guint width, guint height, guint bpp, guint scale_factor) {

    // Copy the output back on top of the working/preview buffer
    //   This only copies a windowed subset of the output (size matching input buffer pre-scaled)
    for (gint y = 0; y < height; y++) {

         // TODO: fix assumptions about bpp/etc here - NOTE note using BPP, since increment is in uint32_ts
         // Because I can never remember:  memcopy direction (p1, <-- p2)
         memcpy(p_srcbuf, p_scaledbuf, width * bpp);
         p_srcbuf +=  width * bpp;                // increment row for dest buffer
         p_scaledbuf +=  width * scale_factor * bpp; // increment row for source buffer

        // TODO: if (has_alpha)
    }
}


static void filter_apply(int scaler_mode, uint32_t * p_srcbuf, uint32_t * p_destbuf, int width, int height) {

    if (scaler_mode < SCALER_ENUM_LAST) {

        // Call the requested scaler function
        scalers[scaler_mode].scaler_function( (uint32_t*) p_srcbuf,
                                              (uint32_t*) p_destbuf,
                                                    (int) width,
                                                    (int) height);
    }
}

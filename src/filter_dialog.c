//#include "config.h"
//#include <string.h>

#include <stdio.h>
#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "filter_dialog.h"

extern const char PLUG_IN_PROCEDURE[];
extern const char PLUG_IN_ROLE[];
extern const char PLUG_IN_BINARY[];


/*******************************************************/
/*                    Dialog                           */
/*******************************************************/

gboolean pixel_art_scalers_dialog (GimpDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
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

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  preview = gimp_drawable_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview,
                            "invalidated",
                            G_CALLBACK (pixel_art_scalers_run),
                            drawable);

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





/*
// TODO
 * Cartoon algorithm
 * -----------------
 * Mask radius = radius of pixel neighborhood for intensity comparison
 * Threshold   = relative intensity difference which will result in darkening
 * Ramp        = amount of relative intensity difference before total black
 * Blur radius = mask radius / 3.0
 *
 * Algorithm:
 * For each pixel, calculate pixel intensity value to be: avg (blur radius)
 * relative diff = pixel intensity / avg (mask radius)
 * If relative diff < Threshold
 *   intensity mult = (Ramp - MIN (Ramp, (Threshold - relative diff))) / Ramp
 *   pixel intensity *= intensity mult
 */


// static void pixel_art_scalers_run (GimpDrawable *drawable)
//                    gint          pixelwidth,
//                    gint          pixelheight,
//                    gint          tile_width,
//                    gint          tile_height)
void pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview)
{
    GimpPixelRgn src_rgn, dest_rgn;
    gint         bpp, has_alpha;
    gint         width, height;
    gint         progress, max_progress;

    guchar *src_row, *s;
    guchar *dest_row, *d;
    gint    x, y;
    gint    alpha;
    gpointer pixel_tile_sub_region;

    guchar       *preview_buffer = NULL;


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

    // FALSE, FALSE : region will be used to read the actual drawable datas
    // Initialize source pixel region with drawable
    gimp_pixel_rgn_init (&src_rgn,
                         drawable,
                         x, y,
                         width, height,
                         FALSE, FALSE);

    if (preview) {
        // Allocate preview buffer
        preview_buffer = g_new (guchar, width * height * bpp);

        // Register the source pixel tile sub region
        pixel_tile_sub_region = gimp_pixel_rgns_register (1, &src_rgn);
    }
    else {
        // TRUE,  TRUE  : region will be used to write to the shadow tiles
        //                i.e. make changes that will be written back to source tiles
        // Initialize dest pixel region with drawable
        gimp_pixel_rgn_init (&dest_rgn,
                             drawable,
                             x, y,
                             width, height,
                             TRUE, TRUE);

        // Register the source and destination pixel regions
        pixel_tile_sub_region = gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn);
    }


    // Initialize progress indicator
    progress = 0;
    max_progress = width * height;


  // Process the image
    while (pixel_tile_sub_region)
    {
        // Set source pointer to start of tile sub-region
        src_row = src_rgn.data;

        if (preview) {
            // Set dest pointer using offset to matching sub-region within it's full size region buffer
            dest_row = preview_buffer  + ((src_rgn.y - y) * width + (src_rgn.x - x)) * bpp;
        }
        else {
            // Set dest pointer to start of matching tile sub-region
            dest_row = dest_rgn.data;
        }


        // Loop through each row, and then each pixel for the rows
        for (int y1 = 0; y1 < src_rgn.h; y1++) {

            // Set/Reset pixel pointers to start of upcoming row
            s = src_row;
            d = dest_row;

            for (int x1 = 0; x1 < src_rgn.w; x1++) {

                d[0] = s[0] ^ 0xFF;
                d[1] = s[1] ^ 0xFF;
                d[2] = s[2] ^ 0xFF;

                if (has_alpha)
                    d[alpha] = s[alpha];

                s += src_rgn.bpp;

                if (preview) d += bpp;
                else         d += dest_rgn.bpp;
            }

            // Advance pixel row pointers to start of upcoming row
            src_row += src_rgn.rowstride;

            if (preview) dest_row += width * bpp;
            else         dest_row += dest_rgn.rowstride;
        }


        // Only update the progress bar status if it's not a preview
        if (!preview) {
            progress += src_rgn.w * src_rgn.h;
            gimp_progress_update ((double) progress / (double) max_progress);
        }

        // Iterate to next pixel tile sub-region
        pixel_tile_sub_region = gimp_pixel_rgns_process (pixel_tile_sub_region);
    }


    // Filter is done, apply the update
    if (preview) {
        gimp_preview_draw_buffer (preview, preview_buffer, width * bpp);
        g_free (preview_buffer);
    }
    else
    {
        // Update progress to 100% complete
        gimp_progress_update (1.0);

        // Apply the changes to the image (merge shadow, update drawable)
        gimp_drawable_flush (drawable);
        gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
        gimp_drawable_update (drawable->drawable_id, x, y, width, height);
    }
}




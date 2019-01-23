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
 *  algorithm
 * -----------------
 *
 * Algorithm:
 */


// static void pixel_art_scalers_run (GimpDrawable *drawable)
//                    gint          pixelwidth,
//                    gint          pixelheight,
//                    gint          tile_width,
//                    gint          tile_height)
//
// TODO: This would be less brittle and easier to understand if
//       preview vs. apply was either seperated out or abstracted.
void pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview)
{
    GimpPixelRgn src_rgn, dest_rgn;
    gint         bpp, has_alpha;
    gint         width, height;
    gint         progress, max_progress;

    guchar       *src_row, *s;
    guchar       *dest_row, *d;
    gint         x, y;
    gint         alpha;
    gpointer     pixel_tile_sub_region;

    guchar       * p_workbuf = NULL;
    guchar       * p_pix = NULL;


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
    p_workbuf = g_new (guchar, width * height * bpp);

    // FALSE, FALSE : region will be used to read the actual drawable datas
    // Initialize source pixel region with drawable
    gimp_pixel_rgn_init (&src_rgn,
                         drawable,
                         x, y,
                         width, height,
                         FALSE, FALSE);

    if (!preview) {
        // TRUE,  TRUE  : region will be used to write to the shadow tiles
        //                i.e. make changes that will be written back to source tiles
        // Initialize dest pixel region with drawable
        gimp_pixel_rgn_init (&dest_rgn,
                             drawable,
                             x, y,
                             width, height,
                             TRUE, TRUE);
    }


    // Copy source image to working buffer
    gimp_pixel_rgn_get_rect (&src_rgn,
                              p_workbuf,
                              x, y, width, height);


    // Use a temp pointer to operate on the working buffer
    p_pix = p_workbuf;

    // Iterate over the working buffer and modify it
    for (gint y = 0; y < height; y++) {
        for (gint x = 0; x < width; x++) {
              *(p_pix++) ^= 0xFF; //[0]
              *(p_pix++) ^= 0xFF; //[1]
              *(p_pix++) ^= 0xFF; //[2]

              if (has_alpha) //
                  p_pix++;
        }

        // Only update the progress bar status if it's not a preview
        if (!preview) {
            gimp_progress_update ((double) y / (double) height);
        }
    }


    // Filter is done, apply the update
    if (preview) {
        gimp_preview_draw_buffer (preview, p_workbuf, width * bpp);
    }
    else
    {
        // Update progress to 100% complete
        gimp_progress_update (1.0);

        // Copy working buffer to the shadow image
        gimp_pixel_rgn_set_rect (&dest_rgn,
                                  p_workbuf,
                                  x, y, width, height);

        // Apply the changes to the image (merge shadow, update drawable)
        gimp_drawable_flush (drawable);
        gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
        gimp_drawable_update (drawable->drawable_id, x, y, width, height);
    }

    // Free the working buffer
    g_free (p_workbuf);
}




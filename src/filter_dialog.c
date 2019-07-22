//
// filter_dialog.c
//

// ========================
//
// Creates and shows plug-in dialog window,
// displays preview of upscaled images
//
// ========================

//#include "config.h"
//#include <string.h>

#include <stdio.h>
#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "filter_dialog.h"
#include "filter_scalers.h"

extern const char PLUG_IN_PROCEDURE[];
extern const char PLUG_IN_ROLE[];
extern const char PLUG_IN_BINARY[];

static void dialog_scaled_preview_check_resize(GtkWidget *, gint, gint, gint);
static void resize_image_and_apply_changes(GimpDrawable *, guchar *, guint);
static void on_settings_scaler_combo_changed (GtkComboBox *, gpointer);
static void on_settings_scalev_combo_changed(GtkComboBox *combo, gpointer callback_data);
gboolean preview_scaled_size_allocate_event(GtkWidget *, GdkEvent *, GtkWidget *);

// Widget for displaying the upscaled image preview
static GtkWidget * preview_scaled;

/*******************************************************/
/*               Main Plug-in Dialog                   */
/*******************************************************/
gboolean pixel_art_scalers_dialog (GimpDrawable *drawable)
{
    GtkWidget * dialog;
    GtkWidget * main_vbox;
    GtkWidget * preview_hbox;
    GtkWidget * preview;
    GtkWidget * scaled_preview_window;

    GtkWidget * settings_table;
    GtkWidget * settings_scaler_combo;
    GtkWidget * settings_scaler_label;
    GtkWidget * settings_scalev_combo;
    GtkWidget * settings_scalev_label;

    gboolean   run;
    gint       idx;


    gimp_ui_init (PLUG_IN_BINARY, FALSE);

    dialog = gimp_dialog_new ("Pixel Art Scalers", PLUG_IN_ROLE, NULL, 0, gimp_standard_help_func, PLUG_IN_PROCEDURE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

    // Resize to show more of scaled preview by default (this sets MIN size)
    gtk_widget_set_size_request (dialog, 500, 400);

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

    gimp_window_set_transient (GTK_WINDOW (dialog));

    // Create a main vertical box for the preview
    main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 6);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), main_vbox, TRUE, TRUE, 0);
    gtk_widget_show (main_vbox);

    // Create a side-by-side sub-box for the pair of preview windows
    preview_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width (GTK_CONTAINER (preview_hbox), 6);
    gtk_box_pack_start (GTK_BOX (main_vbox), preview_hbox, TRUE, TRUE, 0);
    gtk_widget_show (preview_hbox);

    // Create source image preview and scaled preview areas
    // along with a scrolled window area for the scaled preview
    preview = gimp_drawable_preview_new (drawable, NULL);
    preview_scaled = gimp_preview_area_new();
    scaled_preview_window = gtk_scrolled_window_new (NULL, NULL);

    // Display the source image preview area, set it to not expand if window grows
    gtk_box_pack_start (GTK_BOX (preview_hbox), preview, FALSE, TRUE, 0);
    gtk_widget_show (preview);

    // Automatic scrollbars for scrolled preview window
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scaled_preview_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Add the scaled preview to the scrolled window
    // and then display them both (with auto-resize)
    gtk_scrolled_window_add_with_viewport((GtkScrolledWindow *)scaled_preview_window, preview_scaled);
    gtk_box_pack_start (GTK_BOX (preview_hbox), scaled_preview_window, TRUE, TRUE, 0);
    gtk_widget_show (scaled_preview_window);
    gtk_widget_show (preview_scaled);

    // Wire up source image preview redraw to call the pixel scaler filter
    g_signal_connect_swapped (preview, "invalidated", G_CALLBACK (pixel_art_scalers_run), drawable);

    // resize scaled preview -> destroys scaled preview buffer -> resizes scroll window -> size-allocate -> redraw preview buffer
    // This fixes the scrolled window inhibiting the redraw when the size changed
    g_signal_connect(preview_scaled, "size-allocate", G_CALLBACK(preview_scaled_size_allocate_event), (gpointer)scaled_preview_window);

    // Create 1 x 3 table for Settings, non-homogonous sizing, attach to main vbox
    // TODO: Consider changing from a table to a grid (tables are deprecated)
    settings_table = gtk_table_new (1, 3, FALSE);
    gtk_box_pack_start (GTK_BOX (main_vbox), settings_table, FALSE, FALSE, 0);
    gtk_table_set_homogeneous(GTK_TABLE (settings_table), TRUE);

    // Add a Combo box for the SCALER FACTOR
    settings_scalev_label = gtk_label_new ("Scale:  " );
    gtk_misc_set_alignment(GTK_MISC(settings_scalev_label), 1.0f, 0.5f);
    settings_scalev_combo = gtk_combo_box_text_new ();

    // Create label and right-align it
    settings_scaler_label = gtk_label_new ("Scaler type:  " );
    gtk_misc_set_alignment(GTK_MISC(settings_scaler_label), 1.0f, 0.5f);

    // Add a Combo box for the SCALER MODE
    // then add entries for the scaler types and then set default
    settings_scaler_combo = gtk_combo_box_text_new ();

    for (idx = SCALER_ENUM_FIRST; idx < SCALER_ENUM_LAST; idx++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(settings_scaler_combo), scaler_name_get(idx));

    gtk_combo_box_set_active(GTK_COMBO_BOX(settings_scaler_combo), scaler_mode_get());

    for (idx = SCALEV_ENUM_FIRST; idx < SCALEV_ENUM_LAST; idx++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(settings_scalev_combo), scaler_namevalue_get(idx));

    gtk_combo_box_set_active(GTK_COMBO_BOX(settings_scalev_combo), scaler_factor_index_get());


    // Attach the label and combo to the table and show them all
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scalev_label, 1, 2, 0, 1); // Middle of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scalev_combo, 2, 3, 0, 1); // Right side of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scaler_label, 3, 4, 0, 1); // Middle of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scaler_combo, 4, 5, 0, 1); // Right side of table

    gtk_widget_show (settings_table);
    gtk_widget_show (settings_scalev_label);
    gtk_widget_show (settings_scalev_combo);
    gtk_widget_show (settings_scaler_label);
    gtk_widget_show (settings_scaler_combo);

    // Connect the changed signal to update the scaler mode
    g_signal_connect (settings_scaler_combo, "changed", G_CALLBACK (on_settings_scaler_combo_changed), NULL);
    g_signal_connect (settings_scalev_combo, "changed", G_CALLBACK (on_settings_scalev_combo_changed), NULL);

    // Then connect a second signal to trigger a preview update
    g_signal_connect_swapped (settings_scaler_combo, "changed", G_CALLBACK (gimp_preview_invalidate), preview);
    g_signal_connect_swapped (settings_scalev_combo, "changed", G_CALLBACK (gimp_preview_invalidate), preview);

    gtk_widget_show (dialog);

    run = (gimp_dialog_run(GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

    gtk_widget_destroy (dialog);

    return run;
}

// preview_scaled_size_allocate_event
//
// Handler for widget resize changes of the scaled output preview area
//
//   GimpPreviewArea inherits GtkWidget::size-allocate from GtkDrawingArea
//   Preview-area resets buffer on size change so it needs a redraw
//
gboolean preview_scaled_size_allocate_event(GtkWidget * widget, GdkEvent *event, GtkWidget *window)
{
    scaled_output_info * scaled_output;

    scaled_output = scaled_info_get();

    if (widget == NULL)
        return 1; // Exit, failed

    // Redraw the scaled preview if it's available
    if ((scaled_output->p_scaledbuf != NULL) && (scaled_output->valid_image == TRUE))
    {
        // Calling widget should be preview_scaled
        gimp_preview_area_draw (GIMP_PREVIEW_AREA (widget), 0, 0, scaled_output->width, scaled_output->height, GIMP_RGBA_IMAGE, (guchar *) scaled_output->p_scaledbuf, scaled_output->width * BYTE_SIZE_RGBA_4BPP);
    }

    return FALSE;
}

// on_settings_scaler_combo_changed
//
// Handler for "changed" signal of SCALER MODE combo box
// When the user changes the scaler type -> Update the scaler mode
//
//   callback_data not used currently
//
static void on_settings_scaler_combo_changed(GtkComboBox *combo, gpointer callback_data)
{
    gint idx;
    gchar * selected_string;

    selected_string = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

    for (idx=0; idx < SCALER_ENUM_LAST; idx++)
    {
        // If the mode string matched the one in the combo, select it as the current mode
        if (!(g_strcmp0(selected_string, scaler_name_get(idx))))
            scaler_mode_set(idx);
    }
}
static void on_settings_scalev_combo_changed(GtkComboBox *combo, gpointer callback_data)
{
    gint idx;
    gchar * selected_string;

    selected_string = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

    for (idx=0; idx < SCALEV_ENUM_LAST; idx++)
    {
        // If the mode string matched the one in the combo, select it as the current mode
        if (!(g_strcmp0(selected_string, scaler_namevalue_get(idx))))
            scaler_factor_set(idx);
    }
}

// dialog_scaled_preview_check_resize
//
// Checks to see whether the scaled preview area needs
// to be resized. Handles resizing if needed.
//
// Called from pixel_art_scalers_run() which is used for
// previewing and final rendering of the selected scaler mode
//
static void dialog_scaled_preview_check_resize(GtkWidget * preview_scaled, gint width_new, gint height_new, gint scale_factor_new)
{
    gint width_current, height_current;

    // Get current size for scaled preview area
    gtk_widget_get_size_request (preview_scaled, &width_current, &height_current);

    // Only resize if the width, height or scaling changed
    if ( (width_current  != (width_new  * scale_factor_new)) ||
            (height_current != (height_new * scale_factor_new)) )
    {
        // Resize scaled preview area
        gtk_widget_set_size_request (preview_scaled, width_new * scale_factor_new, height_new * scale_factor_new);

        // when set_size_request and then draw are called repeatedly on a preview_area
        // it causes redraw glitching in the surrounding scrolled_window region
        // Calling set_max_size appears to fix this
        // (though it may be treating the symptom and not the cause of the glitching)
        gimp_preview_area_set_max_size(GIMP_PREVIEW_AREA (preview_scaled), width_new * scale_factor_new, height_new * scale_factor_new);
    }
}

/*******************************************************/
/*                    APPLY SCALER                     */
/*******************************************************/
//
// pixel_art_scalers_run
//
// Previews and performs the final output rendering of
// the selected scaler.
//
// Called from:
// * gimp_preview_invalidate signal
//   -> window redraw events
//   -> user changed scaler type in dropdown combo box
// * The end of filter_pixel_art_scalers.c (if user pressed "OK" to apply)
//
void pixel_art_scalers_run(GimpDrawable *drawable, GimpPreview  *preview)
{
    GimpPixelRgn src_rgn;
    gint         bpp;
    gint         width, height;
    gint         x, y;
    guint        scale_factor_index, scale_factor;
    uint32_t   * p_srcbuf = NULL;
    glong        srcbuf_size = 0;
    scaled_output_info * scaled_output;

    scaled_output = scaled_info_get();
    scale_factor = scaler_factor_get();
    scale_factor_index = scaler_factor_index_get();

    // Get the working image area for either the preview sub-window or the entire image
    if (preview)
    {
        gimp_preview_get_position (preview, &x, &y);
        gimp_preview_get_size (preview, &width, &height);

        dialog_scaled_preview_check_resize(preview_scaled, width, height, scale_factor);
    }
    else if (! gimp_drawable_mask_intersect (drawable->drawable_id, &x, &y, &width, &height))
    {
        return;
    }

    // Get bit depth and alpha mask status
    bpp = drawable->bpp;

    // Allocate output buffer for upscaled image
    scaled_output_check_reallocate(scale_factor, width, height);

    if (scaled_output_check_reapply_scalers(scaler_mode_get(), scaler_factor_get(), x, y))
    {

        // ====== GET THE SOURCE IMAGE ======

        // Allocate a working buffer to copy the source image into - always RGBA 4BPP
        // 32 bit to ensure alignment, divide size since it's in BYTES
        srcbuf_size = width * height * BYTE_SIZE_RGBA_4BPP;
        p_srcbuf = (uint32_t *) g_new (guint32, srcbuf_size / BYTE_SIZE_RGBA_4BPP);

        // FALSE, FALSE : region will be used to read the actual drawable datas
        // Initialize source pixel region with drawable
        gimp_pixel_rgn_init (&src_rgn, drawable, x, y, width, height, FALSE, FALSE);

        // Copy source image to working buffer
        gimp_pixel_rgn_get_rect (&src_rgn, (guchar *) p_srcbuf, x, y, width, height);

        // Add alpha channel byte to source buffer if needed (scalers expect 4BPP RGBA)
        if (bpp == BYTE_SIZE_RGB_3BPP)  // i.e. !has_alpha
            buffer_add_alpha_byte((guchar *) p_srcbuf, srcbuf_size);

        // ====== APPLY THE SCALER ======

        // Expects 4BPP RGBA in p_srcbuf, outputs same to p_scaledbuf
        scaler_apply(scaler_mode_get(), scaler_factor_index_get(), p_srcbuf, scaled_output->p_scaledbuf, (int) width, (int) height);
    }
    // Filter is done, apply the update
    if (preview)
    {
        // Draw scaled image onto preview area
        // Expects 4BPP RGBA
        gimp_preview_area_draw (GIMP_PREVIEW_AREA (preview_scaled), 0, 0, scaled_output->width, scaled_output->height, GIMP_RGBA_IMAGE, (guchar *) scaled_output->p_scaledbuf, scaled_output->width * BYTE_SIZE_RGBA_4BPP);
    }
    else
    {
        // Remove the alpha byte from the scaled output if the source image was 3BPP RGB
        if ((bpp == BYTE_SIZE_RGB_3BPP) & (scaled_output->bpp != BYTE_SIZE_RGB_3BPP))   // i.e. !has_alpha
        {
            buffer_remove_alpha_byte((guchar *) scaled_output->p_scaledbuf, scaled_output->size_bytes);
            scaled_output->bpp = BYTE_SIZE_RGB_3BPP;
        }

        // Apply image result with full resize
        resize_image_and_apply_changes(drawable, (guchar *) scaled_output->p_scaledbuf, scaled_output->scale_factor);
    }

    // Free the working buffer
    if (p_srcbuf)
        g_free (p_srcbuf);
}

// resize_image_and_apply_changes
//
// Resizes image and then draws the newly scaled output onto it.
// This is only for FINAL, NON-PREVIEW rendered output
//
// Called from pixel_art_scalers_run()
//
// Params:
// * GimpDrawable          : from source image
// * guchar * buffer       : the previously rendered scaled output
// * guint    scale_factor : image scale multiplier
//
void resize_image_and_apply_changes(GimpDrawable * drawable, guchar * p_scaledbuf, guint scale_factor)
{
    GimpPixelRgn  dest_rgn;
    gint          x,y, width, height;
    GimpDrawable  * resized_drawable;

    if (! gimp_drawable_mask_intersect (drawable->drawable_id, &x, &y, &width, &height))
        return;

    // == START UNDO GROUPING
    gimp_image_undo_group_start(gimp_item_get_image(drawable->drawable_id));

    // Resize source image
    if (gimp_image_resize(gimp_item_get_image(drawable->drawable_id), width * scale_factor, height * scale_factor, 0,0))
    {

        // Resize the current layer to match the resized image
        gimp_layer_resize_to_image_size(gimp_image_get_active_layer(gimp_item_get_image(drawable->drawable_id)));

        // Get a new drawable handle from the resized layer/image
        resized_drawable = gimp_drawable_get(gimp_image_get_active_drawable(gimp_item_get_image(drawable->drawable_id)));

        // Initialize destination pixel region with drawable
        // TRUE,  TRUE  : region will be used to write to the shadow tiles
        //                i.e. make changes that will be written back to source tiles
        gimp_pixel_rgn_init (&dest_rgn, resized_drawable, 0, 0, width * scale_factor, height * scale_factor, TRUE, TRUE);

        // Copy the previously rendered scaled output buffer
        // to the shadow image buffer in the drawable
        gimp_pixel_rgn_set_rect (&dest_rgn, (guchar *) p_scaledbuf, 0, 0, width * scale_factor, height * scale_factor);

        // Apply the changes to the image (merge shadow, update drawable)
        gimp_drawable_flush (resized_drawable);
        gimp_drawable_merge_shadow (resized_drawable->drawable_id, TRUE);
        gimp_drawable_update (resized_drawable->drawable_id, 0, 0, width * scale_factor, height * scale_factor);

        // Free the extra resized drawable
        gimp_drawable_detach (resized_drawable);
    }

    // == END GROUPING
    gimp_image_undo_group_end(gimp_item_get_image(drawable->drawable_id));
}


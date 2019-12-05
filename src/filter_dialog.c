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

#include "filter_pixel_art_scalers.h"
#include "filter_dialog.h"
#include "filter_scalers.h"



extern const char PLUG_IN_PROCEDURE[];
extern const char PLUG_IN_ROLE[];
extern const char PLUG_IN_BINARY[];

static void dialog_scaled_preview_check_resize(GtkWidget *, gint, gint);
static void resize_image_and_apply_changes(GimpDrawable *, guchar *, guint);

// UI handling
static void on_settings_scaler_combo_changed (GtkComboBox *, gpointer);

static void on_settings_semi_transparency_checkbutton_changed(GtkToggleButton *, gpointer);
static void on_setting_hidden_colors_checkbutton_changed(GtkToggleButton *, gpointer);

static void on_settings_semi_transparency_spinbutton_changed(GtkSpinButton *, gpointer);
static void on_setting_hidden_colors_spinbutton_changed(GtkSpinButton *, gpointer);

gboolean preview_scaled_size_allocate_event(GtkWidget *, GdkEvent *, GtkWidget *);

static PluginPixelArtScalerVals dialog_settings;

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

  GtkWidget * settings_semi_transparency_checkbutton;
  GtkWidget * settings_hidden_colors_checkbutton;
  GtkWidget * settings_semi_transparency_spinbutton;
  GtkWidget * settings_hidden_colors_spinbutton;

  gboolean   run;
  gint       idx;


  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new ("Pixel Art Scalers", PLUG_IN_ROLE,
                            NULL, 0,
                            gimp_standard_help_func, PLUG_IN_PROCEDURE,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  // Resize to show more of scaled preview by default (this sets MIN size)
  gtk_widget_set_size_request (dialog,
                               500,
                               400);


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


    // Create source image preview and scaled preview areas
    // along with a scrolled window area for the scaled preview
    preview = gimp_drawable_preview_new (drawable, NULL);
    preview_scaled = gimp_preview_area_new();
    scaled_preview_window = gtk_scrolled_window_new (NULL, NULL);


    // Display the source image preview area, set it to not expand if window grows
    gtk_box_pack_start (GTK_BOX (preview_hbox), preview, FALSE, TRUE, 0);
    gtk_widget_show (preview);


    // Automatic scrollbars for scrolled preview window
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scaled_preview_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    // Add the scaled preview to the scrolled window
    // and then display them both (with auto-resize)
    gtk_scrolled_window_add_with_viewport((GtkScrolledWindow *)scaled_preview_window,
                                              preview_scaled);
    gtk_box_pack_start (GTK_BOX (preview_hbox), scaled_preview_window, TRUE, TRUE, 0);
    gtk_widget_show (scaled_preview_window);
    gtk_widget_show (preview_scaled);


    // Wire up source image preview redraw to call the pixel scaler filter
    g_signal_connect_swapped (preview,
                              "invalidated",
                              G_CALLBACK (pixel_art_scalers_run),
                              drawable);

    // resize scaled preview -> destroys scaled preview buffer -> resizes scroll window -> size-allocate -> redraw preview buffer
    // This fixes the scrolled window inhibiting the redraw when the size changed
    g_signal_connect(preview_scaled, "size-allocate", G_CALLBACK(preview_scaled_size_allocate_event), (gpointer)scaled_preview_window);


    // Create 1 x 4 table for Settings, non-homogonous sizing, attach to main vbox
    // TODO: Consider changing from a table to a grid (tables are deprecated)
    settings_table = gtk_table_new (1, 4, FALSE);
    gtk_box_pack_start (GTK_BOX (main_vbox), settings_table, FALSE, FALSE, 0);
    gtk_table_set_homogeneous(GTK_TABLE (settings_table), FALSE);

    // Create label and right-align it
    settings_scaler_label = gtk_label_new ("Scaler type:  " );
    gtk_misc_set_alignment(GTK_MISC(settings_scaler_label), 1.0f, 0.5f);

    // Add a Combo box for the SCALER MODE
    // then add entries for the scaler types and then set default
    settings_scaler_combo = gtk_combo_box_text_new ();

    for (idx = SCALER_ENUM_FIRST; idx < SCALER_ENUM_LAST; idx++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(settings_scaler_combo), scaler_name_get(idx));

    gtk_combo_box_set_active(GTK_COMBO_BOX(settings_scaler_combo), scaler_mode_get() );


    // Transparency and alpha blending options (checkbox and value entry spin buttons)
    settings_semi_transparency_checkbutton = gtk_check_button_new_with_label("Remove semi-transparency. Threshold :");
    settings_semi_transparency_spinbutton = gtk_spin_button_new_with_range(0,254,1); // Min/Max/Step

    settings_hidden_colors_checkbutton = gtk_check_button_new_with_label("Suppress color for pixels below opacity :");
    settings_hidden_colors_spinbutton = gtk_spin_button_new_with_range(0,254,1); // Min/Max/Step

        // Load settigns for ui controls
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settings_semi_transparency_checkbutton),
                                     dialog_settings.remove_semi_transparent);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(settings_semi_transparency_spinbutton),
                                     dialog_settings.remove_semi_transparent_threshold);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settings_hidden_colors_checkbutton),
                                     dialog_settings.suppress_hidden_pixel_colors);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(settings_hidden_colors_spinbutton),
                                     dialog_settings.suppress_hidden_pixel_colors_threshold);


        // Enable/disable value inputs
        gtk_widget_set_sensitive(settings_semi_transparency_spinbutton, dialog_settings.remove_semi_transparent == TRUE);
        gtk_widget_set_sensitive(settings_hidden_colors_spinbutton, dialog_settings.suppress_hidden_pixel_colors == TRUE);


    // Attach the label and combo to the table and show them all
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scaler_label, 2, 3, 0, 1); // Middle of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_scaler_combo, 3, 4, 0, 1); // Right side of table

    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_semi_transparency_checkbutton, 0, 1, 0, 1); // Left side of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_semi_transparency_spinbutton, 1, 2, 0, 1); // Left side of table

    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_hidden_colors_checkbutton, 0, 1, 1, 2);     // Left side of table
    gtk_table_attach_defaults (GTK_TABLE (settings_table), settings_hidden_colors_spinbutton, 1, 2, 1, 2);     // Left side of table




    gtk_widget_show (settings_table);
    gtk_widget_show (settings_scaler_label);
    gtk_widget_show (settings_scaler_combo);
    gtk_widget_show (settings_semi_transparency_checkbutton);
    gtk_widget_show (settings_semi_transparency_spinbutton);
    gtk_widget_show (settings_hidden_colors_checkbutton);
    gtk_widget_show (settings_hidden_colors_spinbutton);


    // Connect the changed signal to update the UI controls
    g_signal_connect (settings_scaler_combo,
                      "changed",
                      G_CALLBACK (on_settings_scaler_combo_changed),
                      NULL);

    g_signal_connect(G_OBJECT(settings_semi_transparency_checkbutton), "toggled",
                      G_CALLBACK(on_settings_semi_transparency_checkbutton_changed),
                      settings_semi_transparency_spinbutton); // Spin button is passed so it can be enabled/disabled
    g_signal_connect (settings_semi_transparency_spinbutton, "value-changed",
                      G_CALLBACK (on_settings_semi_transparency_spinbutton_changed), NULL);


    g_signal_connect(G_OBJECT(settings_hidden_colors_checkbutton), "toggled",
                      G_CALLBACK(on_setting_hidden_colors_checkbutton_changed),
                      settings_hidden_colors_spinbutton); // Spin button is passed so it can be enabled/disabled
    g_signal_connect (settings_hidden_colors_spinbutton, "value-changed",
                      G_CALLBACK (on_setting_hidden_colors_spinbutton_changed), NULL);


    // Then connect a second signal to trigger a preview updates
    g_signal_connect_swapped (settings_scaler_combo, "changed",
                              G_CALLBACK (gimp_preview_invalidate), preview);

    g_signal_connect_swapped (settings_semi_transparency_checkbutton, "toggled",
                              G_CALLBACK (gimp_preview_invalidate), preview);
    g_signal_connect_swapped (settings_semi_transparency_spinbutton, "value-changed",
                              G_CALLBACK (gimp_preview_invalidate), preview);


    g_signal_connect_swapped (settings_hidden_colors_checkbutton, "toggled",
                              G_CALLBACK (gimp_preview_invalidate), preview);
    g_signal_connect_swapped (settings_hidden_colors_spinbutton, "value-changed",
                              G_CALLBACK (gimp_preview_invalidate), preview);


  gtk_widget_show (dialog);

  // Flag the scaled image as needing a recalculation
  scaled_output_invalidate();

  run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);


  gtk_widget_destroy (dialog);


  return run;
}


// For calling plugin to set dialog settings, including in headless mode
//
void dialog_settings_set(PluginPixelArtScalerVals * p_plugin_config_vals) {

    // Copy plugin settings to dialog settings
    memcpy (&dialog_settings, p_plugin_config_vals, sizeof(PluginPixelArtScalerVals));
    scaler_mode_set(dialog_settings.scaler_mode);
}

// For calling plugin to retrieve dialog settings (to persist for next-run)
//
void dialog_settings_get(PluginPixelArtScalerVals * p_plugin_config_vals) {

    // Copy dialog settings to plugin settings
    dialog_settings.scaler_mode = scaler_mode_get();
    memcpy (p_plugin_config_vals, &dialog_settings, sizeof(PluginPixelArtScalerVals));
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
    image_info * p_scaled_output;

    p_scaled_output = scaled_info_get();

    if (widget == NULL)
      return 1; // Exit, failed

    // Redraw the scaled preview if it's available
    if ( (p_scaled_output->p_imagebuf != NULL) &&
         (p_scaled_output->valid_image == TRUE) ) {
        gimp_preview_area_draw (GIMP_PREVIEW_AREA (widget),  // Calling widget should be preview_scaled
                                0, 0,
                                p_scaled_output->width,
                                p_scaled_output->height,
                                GIMP_RGBA_IMAGE,
                                (guchar *) p_scaled_output->p_imagebuf,
                                p_scaled_output->width * BYTE_SIZE_RGBA_4BPP);
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

    selected_string = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT(combo) );

    for (idx=0; idx < SCALER_ENUM_LAST; idx++) {
        // If the mode string matched the one in the combo, select it as the current mode
        if (!(g_strcmp0(selected_string, scaler_name_get(idx))))
          scaler_mode_set(idx);
    }

    // Flag the scaled image as needing a recalculation
    scaled_output_invalidate();
}


static void on_settings_semi_transparency_checkbutton_changed(GtkToggleButton * p_togglebutton, gpointer callback_data) {

    dialog_settings.remove_semi_transparent = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_togglebutton));

    // Enable/disable threshold input field alongside checkbox
    gtk_widget_set_sensitive((GtkWidget *) callback_data, dialog_settings.remove_semi_transparent == TRUE);

    // Request a preview image update
    scaled_output_invalidate();
}


static void on_setting_hidden_colors_checkbutton_changed(GtkToggleButton * p_togglebutton, gpointer callback_data) {

    dialog_settings.suppress_hidden_pixel_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_togglebutton));

    // Enable/disable threshold input field alongside checkbox
    gtk_widget_set_sensitive((GtkWidget *) callback_data, dialog_settings.suppress_hidden_pixel_colors == TRUE);

    // Request a preview image update
    scaled_output_invalidate();
}


static void on_settings_semi_transparency_spinbutton_changed(GtkSpinButton * spinbutton, gpointer callback_data) {
    dialog_settings.remove_semi_transparent_threshold = gtk_spin_button_get_value_as_int(spinbutton);
    scaled_output_invalidate(); // Request a preview image update
}


static void on_setting_hidden_colors_spinbutton_changed(GtkSpinButton * spinbutton, gpointer callback_data) {
    dialog_settings.suppress_hidden_pixel_colors_threshold = gtk_spin_button_get_value_as_int(spinbutton);
    scaled_output_invalidate(); // Request a preview image update
}



// dialog_scaled_preview_check_resize
//
// Checks to see whether the scaled preview area needs
// to be resized. Handles resizing if needed.
//
// Called from pixel_art_scalers_run() which is used for
// previewing and final rendering of the selected scaler mode
//
static void dialog_scaled_preview_check_resize(GtkWidget * preview_scaled, gint width_new, gint height_new)
{
    gint width_current, height_current;

    // Get current size for scaled preview area
    gtk_widget_get_size_request (preview_scaled, &width_current, &height_current);

    // Only resize if the width, height or scaling changed
    if ( (width_current  != width_new) ||
         (height_current != height_new) )
    {
        // Resize scaled preview area
        gtk_widget_set_size_request (preview_scaled, width_new, height_new);

        // when set_size_request and then draw are called repeatedly on a preview_area
        // it causes redraw glitching in the surrounding scrolled_window region
        // Calling set_max_size appears to fix this
        // (though it may be treating the symptom and not the cause of the glitching)
        gimp_preview_area_set_max_size(GIMP_PREVIEW_AREA (preview_scaled),
                                       width_new, height_new);

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
    gint         x, y;
    gint         original_bpp;
    guint        scale_factor;
    image_info   source_image;
    image_info * p_scaled_output;

    image_info_init(&source_image);
    p_scaled_output = scaled_info_get();
    scale_factor = scaler_scale_factor_get( scaler_mode_get() );

#define TEST_GROW_PX_BORDER 0
#define TEST_GROW_PX_BORDER_TOTAL  TEST_GROW_PX_BORDER * 2

    // Get the working image area for either the preview sub-window or the entire image
    if (preview) {
        gimp_preview_get_position (preview, &x, &y);
        gimp_preview_get_size (preview, &source_image.width, &source_image.height);
    }
    else if (! gimp_drawable_mask_intersect (drawable->drawable_id,
                                             &x, &y, &source_image.width, &source_image.height)) {
        return;
    }

    // Get bit depth and alpha mask status
    source_image.bpp = drawable->bpp;
    original_bpp = source_image.bpp;

    if (scaled_output_check_reapply_scalers(scaler_mode_get(), x, y)) {

        // ====== GET THE SOURCE IMAGE ======

        // Allocate a working buffer to copy the source image into - always RGBA 4BPP
        // 32 bit to ensure alignment, divide size since it's in BYTES
        source_image.size_bytes = source_image.width * source_image.height * BYTE_SIZE_RGBA_4BPP;
        source_image.p_imagebuf = (uint32_t *) g_new (guint32, source_image.size_bytes / BYTE_SIZE_RGBA_4BPP);

        // FALSE, FALSE : region will be used to read the actual drawable datas
        // Initialize source pixel region with drawable
        gimp_pixel_rgn_init (&src_rgn,
                             drawable,
                             x, y,
                             source_image.width, source_image.height,
                             FALSE, FALSE);

        // Copy source image to working buffer
        gimp_pixel_rgn_get_rect (&src_rgn,
                                 (guchar *) source_image.p_imagebuf,
                                 x, y, source_image.width, source_image.height);

        // Add alpha channel byte to source buffer if needed (scalers expect 4BPP RGBA)
        if (source_image.bpp == BYTE_SIZE_RGB_3BPP) { // i.e. !has_alpha
            buffer_add_alpha_byte((guchar *) source_image.p_imagebuf, source_image.size_bytes);
            source_image.bpp = BYTE_SIZE_RGBA_4BPP;   // Note: original_bpp retains value for actual image
        }

        // Alters image size and returns a new re-allocated buffer
        // The re-size has to occur *before* scaled_output_check_reallocate
        source_image = buffer_grow_image_border(&source_image, TEST_GROW_PX_BORDER);


        if (dialog_settings.suppress_hidden_pixel_colors) {
            // Suppress hidden colors for INPUT pixels with alpha set to non-visible
            // Those colors can often be for pixels that the creating tool
            // "deleted" by making them non-visible (alpha = 0, etc)
            buffer_set_alpha_hidden_to_adjacent_visible((guchar *) source_image.p_imagebuf,
                                                       source_image.size_bytes,
                                                       BYTE_SIZE_RGBA_4BPP, // Input image should already be forced to 4bpp RGBA
                                                       source_image.width, source_image.height,
                                                       dialog_settings.suppress_hidden_pixel_colors_threshold); // Don't use colors at or below this threshold
        }


        // ====== APPLY THE SCALER ======

        // Allocate output buffer for upscaled image
        scaled_output_check_reallocate(scale_factor, source_image.width, source_image.height);

        // Expects 4BPP RGBA in source_image.p_imagebuf, outputs same to scaled_output->p_imagebuf
        scaler_apply(scaler_mode_get(),
                     source_image.p_imagebuf,
                     p_scaled_output->p_imagebuf,
                     (int) source_image.width, (int) source_image.height);


        if (dialog_settings.remove_semi_transparent) {
            // This will force partially transparent OUTPUT pixels to solid
            buffer_remove_partial_alpha((guchar *) p_scaled_output->p_imagebuf,
                                         p_scaled_output->size_bytes,
                                         p_scaled_output->bpp,
                                         dialog_settings.remove_semi_transparent_threshold, // alpha_threshold (values at or below this are replaced)
                                         ALPHA_PIXEL_REPLACE_VALUE_BELOW, // alpha value for below threshold, typically 0: fully transparent
                                         ALPHA_PIXEL_REPLACE_VALUE_ABOVE); // alpha value for above threshold, typically 255: fully opaque
        }

        printf("buffer_shrink_image_border\n");
        *p_scaled_output = buffer_shrink_image_border(p_scaled_output, TEST_GROW_PX_BORDER * scale_factor);

    }

    // Filter is done, apply the update
    if (preview) {

        dialog_scaled_preview_check_resize( preview_scaled,
                                            p_scaled_output->width, p_scaled_output->height);

        // Draw scaled image onto preview area
        // Expects 4BPP RGBA
        gimp_preview_area_draw (GIMP_PREVIEW_AREA (preview_scaled),
                                0, 0,
                                p_scaled_output->width,
                                p_scaled_output->height,
                                GIMP_RGBA_IMAGE,
                                (guchar *) p_scaled_output->p_imagebuf,
                                p_scaled_output->width * BYTE_SIZE_RGBA_4BPP);
    }
    else
    {
        // Remove the alpha byte from the scaled output if the source image was 3BPP RGB
        if ((original_bpp == BYTE_SIZE_RGB_3BPP) & (p_scaled_output->bpp != BYTE_SIZE_RGB_3BPP)) { // i.e. !has_alpha
            buffer_remove_alpha_byte((guchar *) p_scaled_output->p_imagebuf, p_scaled_output->size_bytes);
            p_scaled_output->bpp = BYTE_SIZE_RGB_3BPP;
        }

        // Apply image result with full resize
        resize_image_and_apply_changes(drawable,
                                       (guchar *) p_scaled_output->p_imagebuf,
                                       p_scaled_output->scale_factor);
    }

    // Free the working buffer
    if (source_image.p_imagebuf) {
      g_free (source_image.p_imagebuf);
      source_image.p_imagebuf = NULL;
  }
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
void resize_image_and_apply_changes(GimpDrawable * drawable, guchar * p_imagebuf, guint scale_factor)
{
    GimpPixelRgn  dest_rgn;
    gint          x,y, width, height;
    GimpDrawable  * resized_drawable;

    if (! gimp_drawable_mask_intersect (drawable->drawable_id,
                                         &x, &y, &width, &height))
        return;

    // == START UNDO GROUPING
    gimp_image_undo_group_start(gimp_item_get_image(drawable->drawable_id));

    // Resize source image
    if (gimp_image_resize(gimp_item_get_image(drawable->drawable_id),
                          width * scale_factor,
                          height * scale_factor,
                          0,0))
    {

        // Resize the current layer to match the resized image
        gimp_layer_resize_to_image_size( gimp_image_get_active_layer(
                                           gimp_item_get_image(drawable->drawable_id) ) );


        // Get a new drawable handle from the resized layer/image
        resized_drawable = gimp_drawable_get( gimp_image_get_active_drawable(
                                                gimp_item_get_image(drawable->drawable_id) ) );

        // Initialize destination pixel region with drawable
        // TRUE,  TRUE  : region will be used to write to the shadow tiles
        //                i.e. make changes that will be written back to source tiles
        gimp_pixel_rgn_init (&dest_rgn,
                             resized_drawable,
                             0, 0,
                             width * scale_factor,
                             height * scale_factor,
                             TRUE, TRUE);

        // Copy the previously rendered scaled output buffer
        // to the shadow image buffer in the drawable
        gimp_pixel_rgn_set_rect (&dest_rgn,
                                 (guchar *) p_imagebuf,
                                 0,0,
                                 width * scale_factor,
                                 height * scale_factor);


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


#ifndef __FILTER_DIALOG_H_
#define __FILTER_DIALOG_H_

    #include <stdint.h>


    gboolean pixel_art_scalers_dialog (GimpDrawable *drawable);
    void     pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview);

    static void dialog_scaled_preview_check_resize(GtkWidget *, gint, gint, gint);
    static void on_settings_scaler_combo_changed (GtkComboBox *, gpointer);
#endif

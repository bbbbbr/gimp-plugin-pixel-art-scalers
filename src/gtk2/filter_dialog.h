//
// filter_dialog.h
//

#ifndef __FILTER_DIALOG_H_
#define __FILTER_DIALOG_H_

    #include <stdint.h>

    gboolean pixel_art_scalers_dialog (GimpDrawable *drawable);
    void     pixel_art_scalers_run (GimpDrawable *drawable, GimpPreview  *preview);

    void dialog_settings_set(PluginPixelArtScalerVals * p_plugin_config_vals);
    void dialog_settings_get(PluginPixelArtScalerVals * p_plugin_config_vals);

#endif

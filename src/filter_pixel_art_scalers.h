//
// filter_pixel_art_scalers.h
//

#ifndef __FILTER_PIXEL_ART_SCALERS_H_
#define __FILTER_PIXEL_ART_SCALERS_H_

    typedef struct
    {
        gint  scaler_mode;
        gint  remove_semi_transparent;
        gint  remove_semi_transparent_threshold;
        gint  suppress_hidden_pixel_colors;
        gint  suppress_hidden_pixel_colors_threshold;
    } PluginPixelArtScalerVals;


#endif
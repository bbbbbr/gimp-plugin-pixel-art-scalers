//
// filter_scalers.h
//

#ifndef __FILTER_UTILS_H_
#define __FILTER_UTILS_H_


    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>

    #include <libgimp/gimp.h>
    #include <libgimp/gimpui.h>

    void buffer_tiled_edge_copy(image_info *, gint, gint, gint, gint);

    void buffer_add_alpha_byte(guchar *, glong);
    void buffer_remove_alpha_byte(guchar *, glong);

    image_info buffer_grow_image_border (image_info *, gint, gint);
    image_info buffer_shrink_image_border (image_info *, gint, gint);

    void buffer_remove_partial_alpha(guchar *, glong, gint, guchar, guchar, guchar);
    void buffer_set_alpha_hidden_to_adjacent_visible(guchar *, glong, gint, gint, gint, guchar);

#endif

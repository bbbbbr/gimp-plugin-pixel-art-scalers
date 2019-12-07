//
// filter_pixel_art_scalers.c
//

// ========================
//
// Registers filter with Gimp,
// calls dialog and applies processing
//
// ========================


#include <string.h>
#include <stdint.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "filter_pixel_art_scalers.h"

#include "filter_dialog.h"
#include "filter_scalers.h"



const char PLUG_IN_PROCEDURE[] = "filter-pixel-art-scalers-proc";
const char PLUG_IN_ROLE[]      = "gimp-pixel-art-scalers";
const char PLUG_IN_BINARY[]    = "plugin-pixel-art-scalers";


// Predeclare entrypoints
static void query(void);
static void run(const gchar *, gint, const GimpParam *, gint *, GimpParam **);




// Declare plugin entry points
GimpPlugInInfo PLUG_IN_INFO = {
    NULL,
    NULL,
    query,
    run
};


// Default settings for semi-persistant plugin config
static PluginPixelArtScalerVals plugin_config_vals =
{
  0,                                     // scaler_mode, default is HQ2X
  0,                                     // remove_semi_transparent;
  ALPHA_PIXEL_REPLACE_THRESHOLD_DEFAULT, // remove_semi_transparent_threshold;
  0,                                     // suppress_hidden_pixel_colors
  HIDDEN_PIXEL_ALPHA_THRESHOLD_DEFAULT,  // suppress_hidden_pixel_colors_threshold
  0,                                     // default border mode is NONE
};



MAIN()

// The query function
static void query(void)
{
    static const GimpParamDef args[] =
    {
        { GIMP_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
        { GIMP_PDB_IMAGE,    "image",       "Input image (unused)" },
        { GIMP_PDB_DRAWABLE, "drawable",    "Input drawable" },
        { GIMP_PDB_INT32,    "scalar-mode", "Scaler mode to use for up-scaling the image (0-N)" },
        { GIMP_PDB_INT32,    "remove-semi-transparent",                 "Converts semi-transparent pixels in scaled OUTPUT image to fully transparent or opaque(0-1)" },
        { GIMP_PDB_INT32,    "remove-semi-transparent-threshold",       "Threshold for converting semi-transparency (0-254)" },
        { GIMP_PDB_INT32,    "suppress-hidden-pixel-colors",            "Suppress color from hidden pixels on INPUT image (0-1)" },
        { GIMP_PDB_INT32,    "suppress-hidden-pixel-colors-threshold",  "Threshold for suppressing hidden pixel color (0-254)" },
        { GIMP_PDB_INT32,    "border-mode", "Border processing mode (0-N)" },
    };


    gimp_install_procedure (PLUG_IN_PROCEDURE,
                            "Pixel Art Scalers",
                            "Pixel Art Scalers",
                            "--",
                            "Copyright --",
                            "2019",
                            "Pi_xel Art Scalers ...",
                            "RGB*", // TODO: INDEXED IMAGE SUPPORT
                            GIMP_PLUGIN,
                            G_N_ELEMENTS (args),
                            0,
                            args,
                            NULL);

    gimp_plugin_menu_register (PLUG_IN_PROCEDURE, "<Image>/Filters/Render");
}





// The run function
static void run(const gchar      * name,
                      gint         nparams,
                const GimpParam  * param,
                      gint       * nreturn_vals,
                      GimpParam ** return_vals)
{
    // Create the return value.
    static GimpParam   return_values[2];
    GimpRunMode        run_mode;
    GimpDrawable       *drawable;
    GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

    run_mode      = param[0].data.d_int32;

    //  Get the specified drawable
    drawable = gimp_drawable_get (param[2].data.d_drawable);

    *nreturn_vals = 1;
    *return_vals  = return_values;

    // Set the return value to success by default
    return_values[0].type          = GIMP_PDB_STATUS;
    // return_values[0].data.d_status = GIMP_PDB_SUCCESS;
    return_values[0].data.d_status = status;


    // Initialize the scalers
    scalers_init();


  switch (run_mode)
    {
    case GIMP_RUN_INTERACTIVE:
        //  Try to retrieve plugin settings, then apply them
        gimp_get_data (PLUG_IN_PROCEDURE, &plugin_config_vals);
        dialog_settings_set (&plugin_config_vals);

        //  First acquire information with a dialog
        if (! pixel_art_scalers_dialog (drawable))
            return;

        break;

    case GIMP_RUN_NONINTERACTIVE:
        // Read in non-interactive mode plug settings, then apply them
        plugin_config_vals.scaler_mode                       = param[3].data.d_int32;
        plugin_config_vals.remove_semi_transparent           = param[4].data.d_int32;
        plugin_config_vals.remove_semi_transparent_threshold = param[5].data.d_int32;
        plugin_config_vals.suppress_hidden_pixel_colors           = param[6].data.d_int32;
        plugin_config_vals.suppress_hidden_pixel_colors_threshold = param[7].data.d_int32;
        plugin_config_vals.border_mode                            = param[8].data.d_int32;
        // Set settings/config in dialog
        dialog_settings_set (&plugin_config_vals);
        break;

    case GIMP_RUN_WITH_LAST_VALS:
        //  Try to retrieve plugin settings, then apply them
        gimp_get_data (PLUG_IN_PROCEDURE, &plugin_config_vals);
        dialog_settings_set (&plugin_config_vals);

        break;

    default:
      break;
    }

  if (status == GIMP_PDB_SUCCESS)
  {
      /*  Make sure that the drawable is RGB color  */
      if (gimp_drawable_is_rgb (drawable->drawable_id))
        {
          gimp_progress_init ("Pixel-Art-Scalers");


          // Apply image filter (user confirmed in preview dialog)
          pixel_art_scalers_run(drawable, NULL);

          if (run_mode != GIMP_RUN_NONINTERACTIVE)
            gimp_displays_flush ();

          // Retrieve and then save plugin config settings
          if (run_mode == GIMP_RUN_INTERACTIVE)
            dialog_settings_get (&plugin_config_vals);
            gimp_set_data (PLUG_IN_PROCEDURE, &plugin_config_vals, sizeof (PluginPixelArtScalerVals));
        }
      else
        {
          status        = GIMP_PDB_EXECUTION_ERROR;
          *nreturn_vals = 2;
          return_values[1].type          = GIMP_PDB_STRING;
          return_values[1].data.d_string = "Cannot operate on greyscale or indexed color images.";
          // TODO: INDEXED IMAGE SUPPORT
        }
  }

  pixel_art_scalers_release_resources();

  return_values[0].data.d_status = status;

  gimp_drawable_detach (drawable);
}



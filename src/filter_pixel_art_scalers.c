// filter_pixel_art_scalers.c

#include <string.h>
#include <stdint.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

//#include "filter_pixel_art_scalers.h"
#include "filter_dialog.h"
#include "filter_scalers.h"



const char PLUG_IN_PROCEDURE[] = "filter-pixel-art-scalers-proc";
const char PLUG_IN_ROLE[]      = "gimp-pixel-art-scalers";
const char PLUG_IN_BINARY[]    = "plugin-pixel-art-scalers";


// Predeclare our entrypoints
static void query(void);
static void run(const gchar *, gint, const GimpParam *, gint *, GimpParam **);



// Declare our plugin entry points
GimpPlugInInfo PLUG_IN_INFO = {
    NULL,
    NULL,
    query,
    run
};

/*
static PixelArtScalerVals cvals =
{
  1  // scaler_type
};
*/

/*
// TODO
static CartoonVals cvals =
{
  7.0,  // mask_radius
  1.0,  // threshold
  0.2   // pct_black
};
*/



MAIN()

// The query function
static void query(void)
{
    static const GimpParamDef args[] =
    {
        { GIMP_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
        { GIMP_PDB_IMAGE,    "image",       "Input image (unused)" },
        { GIMP_PDB_DRAWABLE, "drawable",    "Input drawable" }
        //,
        //{ GIMP_PDB_FLOAT,    "mask-radius", "Cartoon mask radius (radius of pixel neighborhood)" },
        //{ GIMP_PDB_FLOAT,    "pct-black",   "Percentage of darkened pixels to set to black (0.0 - 1.0)" }
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
      //  Possibly retrieve data
      // TODO
      // gimp_get_data (PLUG_IN_PROCEDURE, &cvals);

      //  First acquire information with a dialog
      if (! pixel_art_scalers_dialog (drawable))
        return;
      break;

    case GIMP_RUN_NONINTERACTIVE:
      // TODO
      // cvals.mask_radius = param[3].data.d_float;
      // cvals.pct_black   = param[4].data.d_float;
      break;

    case GIMP_RUN_WITH_LAST_VALS:
      //  Possibly retrieve data
    // TODO
     // gimp_get_data (PLUG_IN_PROCEDURE, &cvals);
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

          //  Store data
          //if (run_mode == GIMP_RUN_INTERACTIVE)
            //gimp_set_data (PLUG_IN_PROC, &cvals, sizeof (CartoonVals));
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



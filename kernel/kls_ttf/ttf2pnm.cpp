/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-2000, 2003, 2004, 2005, 2006 by                          */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*                                                                          */
/*  FTView - a simple font viewer.                                          */
/*                                                                          */
/*  This is a new version using the MiGS graphics subsystem for             */
/*  blitting and display.                                                   */
/*                                                                          */
/*  Press F1 when running this program to have a list of key-bindings       */
/*                                                                          */
/****************************************************************************/

#include <freetype/config/ftheader.h>

#include "ftcommon.h"
#include <math.h>

  /* the following header shouldn't be used in normal programs */
#include FT_SYNTHESIS_H

#define MAXPTSIZE      500                 /* dtp */

#ifdef CEIL
#undef CEIL
#endif
#define CEIL( x )   ( ( (x) + 63 ) >> 6 )

#define INIT_SIZE( size, start_x, start_y, step_x, step_y, x, y )        \
          do {                                                           \
            start_x = 4;                                                 \
            start_y = CEIL( size->metrics.height );                      \
            step_x  = CEIL( size->metrics.max_advance );                 \
            step_y  = CEIL( size->metrics.height ) + 4;                  \
                                                                         \
            x = start_x;                                                 \
            y = start_y;                                                 \
          } while ( 0 )

#define X_TOO_LONG( x, size, bitmap) \
          ( ( x ) + ( ( size )->metrics.max_advance >> 6 ) > bitmap->width )

#define Y_TOO_LONG( y, size, bitmap) \
          ( ( y ) >= bitmap->rows )

grBitmap *bit;

  static struct  status_
  {
    FT_Encoding  encoding;
    int          res;
    int          ptsize;            /* current point size */

    int          font_index;
    int          Num;               /* current first index */
    int          Fail;

  } status = { FT_ENCODING_NONE, 72, 24, 0, 0, 0 };

  static FTDemo_Handle*   handle;

  static FT_Error
  Render_All( int  num_indices,
              int  first_index )
  {
    int         start_x, start_y, step_x, step_y, x, y;
    int         i;
    FT_Size     size;

    error = FTDemo_Get_Size(handle, &size);

    if ( error )
    {
      /* probably a non-existent bitmap font size */
      return error;
    }

    INIT_SIZE( size, start_x, start_y, step_x, step_y, x, y );

    i = first_index;

    while ( i < num_indices )
    {
      int  gindex;

      if ( handle->encoding == FT_ENCODING_NONE )
        gindex = i;
      else
        gindex = FTDemo_Get_Index( handle, i );

      error = FTDemo_Draw_Index( handle, bit, gindex, &x, &y );

      if ( error )
        status.Fail++;
      else if ( X_TOO_LONG( x, size, bit ) )
      {
        x  = start_x;
        y += step_y;

        if ( Y_TOO_LONG( y, size, bit ) )
          break;
      }

      i++;
    }

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                REST OF THE APPLICATION/PROGRAM                *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  event_font_change()
  {
    int      num_indices;

    status.font_index = 0;
    status.Num = 0;

    FTDemo_Set_Current_Font(handle, handle->fonts[status.font_index]);
    FTDemo_Set_Current_Pointsize(handle, status.ptsize, status.res);
    FTDemo_Update_Current_Flags(handle);

    num_indices = handle->current_font->num_indices;

    if(status.Num >= num_indices)
      status.Num = num_indices - 1;
  }


  int
  main( int    argc,
        char*  argv[] )
  {
    if(argc != 3)
        exit(1);

    /* Initialize engine */
    handle = FTDemo_New(status.encoding);

    FTDemo_Install_Font(handle, argv[1]);

    if(handle->num_fonts == 0)
      PanicZ( "could not find/open any font file" );

    bit = FTDemo_Display_New();

    if(!bit)
      PanicZ( "could not allocate display surface" );

    status.Fail = 0;

    event_font_change();

    FTDemo_Update_Current_Flags(handle);

    FTDemo_Display_Clear(bit);

    Render_All(handle->current_font->num_indices, status.Num);

    FILE *f = fopen(argv[2], "wb");

    fprintf(f, "P6\n%d %d\n255\n", bit->width, bit->rows);
    fwrite(bit->buffer, bit->width * bit->rows * 3, 1, f);

    fclose(f);

    FTDemo_Display_Done(bit);
    FTDemo_Done(handle);

    return 0;
  }

/*
 *  (C) Baryshev Dmitry, ksquirrel-libs project.
 */

/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-2000, 2003, 2004, 2005 by                                */
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


//#include "ftcommon.i"

#include <cmath>
#include <string>
#include <cstdio>
#include <cstdlib>

/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-2000, 2001, 2002, 2003, 2004, 2005 by                    */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*                                                                          */
/*  ftcommon.i - common routines for the FreeType demo programs.            */
/*                                                                          */
/****************************************************************************/

#include <ft2build.h>

#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <freetype/ftstroke.h>
#include <freetype/ftsynth.h>
#include <freetype/ftbitmap.h>

  /* forward declarations */
  void  PanicZ( const char*  message );

  FT_Error   ft_err;
  FT_Bitmap  ft_bitmap;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                 DISPLAY-SPECIFIC DEFINITIONS                  *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


#include "graph.h"
#include "grfont.h"


#define DIM_X      500
#define DIM_Y      400

#define CENTER_X   ( bit.width / 2 )
#define CENTER_Y   ( bit.rows / 2 )

#define MAXPTSIZE  500                 /* dtp */

  grBitmap    bit;          /* current display bitmap  */

  grColor  fore_color = { 0 };

  int  Fail;

  /* PanicZ */
  void
  PanicZ( const char*  message )
  {
    fprintf( stderr, "%s\n  ft_err = 0x%04x\n", message, ft_err );
    exit( 0 );
  }

  /* clear the `bit' bitmap/pixmap */
  void
  Clear_Display()
  {
    int image_size = bit.pitch * bit.rows;

    memset( bit.buffer, 255, image_size );
  }


  /* initialize the display bitmap `bit' */
  void
  Init_Display()
  {
/*    grInitDevices(); */

    bit.mode  = gr_pixel_mode_rgb24;
    bit.width = DIM_X;
    bit.rows  = DIM_Y;
    bit.grays = 256;
/*
    surface = grNewSurface( 0, &bit );
    if ( !surface )
      PanicZ( "could not allocate display surface\n" );

    grSetGlyphGamma( the_gamma );
*/
  }


#define MAX_BUFFER  300000

  FT_Library      ll_ftlib;       /* the FreeType ll_ftlib            */
  FTC_Manager     cache_manager; /* the cache manager               */
  FTC_ImageCache  image_cache;   /* the glyph image cache           */
  FTC_SBitCache   sbits_cache;   /* the glyph small bitmaps cache   */
  FTC_CMapCache   cmap_cache;    /* the charmap cache..             */

  FT_Face         face;          /* the font face                   */
  FT_Size         size;          /* the font size                   */
  FT_GlyphSlot    glyph;         /* the glyph slot                  */

  FTC_ImageTypeRec  current_font;

  int  use_sbits_cache  = 1;

  int  num_indices;           /* number of glyphs or characters */
  int  ptsize;                /* current point size             */

  FT_Encoding  encoding = FT_ENCODING_NONE;

  int  hinted    = 1;         /* is glyph hinting active?    */
  int  antialias = 1;         /* is anti-aliasing active?    */
  int  use_sbits = 1;         /* do we use embedded bitmaps? */
  int  low_prec  = 0;         /* force low precision         */
  int  autohint  = 0;         /* force auto-hinting          */
  int  lcd_mode  = 0;         /* 0 - 5                       */
  int  Num;                   /* current first index         */

  int  res = 72;


#define MAX_GLYPH_BYTES  150000   /* 150kB for the glyph image cache */


#define FLOOR( x )  (   (x)        & -64 )
#define CEIL( x )   ( ( (x) + 63 ) & -64 )
#define TRUNC( x )  (   (x) >> 6 )


  /* this simple record is used to model a given `installed' face */
  typedef struct  TFont_
  {
    const char   *filepathname;
    int          face_index;
    int          cmap_index;

    int          num_indices;

  } TFont, *PFont;

  PFont*  fonts = NULL;
  int     num_fonts = 0;
  int     max_fonts = 0;

  int
  install_font_file(const char *filename)
  {
    int           i, num_faces;

    ft_err = FT_New_Face( ll_ftlib, filename, 0, &face );

    if ( ft_err )
    {
      printf("*** 1\n");
      return 1;
    }

    /* allocate new font object */
    num_faces = face->num_faces;
    
//    printf("N: %d\n", num_faces);
    
    for ( i = 0; i < num_faces; i++ )
    {
      PFont  font;

      if ( i > 0 )
      {
        ft_err = FT_New_Face( ll_ftlib, filename, i, &face );
        if ( ft_err )
          continue;
      }

      if ( encoding != FT_ENCODING_NONE )
      {
        ft_err = FT_Select_Charmap( face, encoding );
        if ( ft_err )
        {
          FT_Done_Face( face );
          face = NULL;
          continue;
        }
      }

      font = (PFont)malloc( sizeof ( *font ) );
      font->filepathname = (char *)malloc(strlen(filename)+1);
      font->face_index   = i;
      font->cmap_index   = face->charmap ? FT_Get_Charmap_Index( face->charmap ) : 0;

      switch ( encoding )
      {
      case FT_ENCODING_NONE:
        font->num_indices = face->num_glyphs;
        break;

      case FT_ENCODING_UNICODE:
        font->num_indices = 0x110000L;
        break;

      case FT_ENCODING_MS_SYMBOL:
      case FT_ENCODING_ADOBE_LATIN_1:
      case FT_ENCODING_ADOBE_STANDARD:
      case FT_ENCODING_ADOBE_EXPERT:
      case FT_ENCODING_ADOBE_CUSTOM:
      case FT_ENCODING_APPLE_ROMAN:
        font->num_indices = 0x100L;
        break;

      default:
        font->num_indices = 0x10000L;
      }

      strcpy( (char*)font->filepathname, filename);

      FT_Done_Face( face );
      face = NULL;

      if ( max_fonts == 0 )
      {
        max_fonts = 16;
        fonts     = (PFont*)calloc( max_fonts, sizeof ( PFont ) );
      }
      else if ( num_fonts >= max_fonts )
      {
        max_fonts *= 2;
        fonts      = (PFont*)realloc( fonts, max_fonts * sizeof ( PFont ) );

        memset( &fonts[num_fonts], 0, ( max_fonts - num_fonts ) * sizeof ( PFont ) );
      }

      fonts[num_fonts++] = font;
    }

    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* The face requester is a function provided by the client application   */
  /* to the cache manager, whose role is to translate an `abstract' face   */
  /* ID into a real FT_Face object.                                        */
  /*                                                                       */
  /* In this program, the face IDs are simply pointers to TFont objects.   */
  /*                                                                       */
  FT_CALLBACK_DEF( FT_Error )
  my_face_requester( FTC_FaceID  face_id,
                     FT_Library  lib,
                     FT_Pointer  request_data,
                     FT_Face*    aface )
  {
    PFont  font = (PFont)face_id;

    FT_UNUSED( request_data );


    ft_err = FT_New_Face( lib,
                         font->filepathname,
                         font->face_index,
                         aface );
    if ( encoding == FT_ENCODING_NONE || ft_err )
      goto Fail;

    ft_err = FT_Select_Charmap( *aface, encoding );

  Fail:
    return ft_err;
  }

  void
  init_freetype()
  {

    ft_err = FT_Init_FreeType( &ll_ftlib );
    if ( ft_err )
      PanicZ( "could not initialize FreeType" );

    ft_err = FTC_Manager_New( ll_ftlib, 0, 0, 0,
                             my_face_requester, 0, &cache_manager );
    if ( ft_err )
      PanicZ( "could not initialize cache manager" );

    ft_err = FTC_SBitCache_New( cache_manager, &sbits_cache );
    if ( ft_err )
      PanicZ( "could not initialize small bitmaps cache" );

    ft_err = FTC_ImageCache_New( cache_manager, &image_cache );
    if ( ft_err )
      PanicZ( "could not initialize glyph image cache" );

    ft_err = FTC_CMapCache_New( cache_manager, &cmap_cache );
    if ( ft_err )
      PanicZ( "could not initialize charmap cache" );

    FT_Bitmap_New( &ft_bitmap );
  }


  void
  done_freetype()
  {
    int  i;
/*
    FT_Done_Face(face);
    face = NULL;
*/


    if(fonts)
    {

    for ( i = 0; i < max_fonts; i++ )
    {
        
      if ( fonts[i] )
      {
        if(fonts[i]->filepathname)
            free((void*)fonts[i]->filepathname);
            
        free( fonts[i] );
      }
    }

    free( fonts );

    fonts = NULL;

    }

    max_fonts = 0;
    num_fonts = 0;

    FTC_Manager_Done( cache_manager );
    FT_Bitmap_Done( ll_ftlib, &ft_bitmap );
    FT_Done_FreeType( ll_ftlib );
  }


  void
  set_current_face( PFont  font )
  {
    current_font.face_id = (FTC_FaceID)font;
  }


  void
  set_current_size( int  pixel_size )
  {
    if ( pixel_size > 0xFFFF )
      pixel_size = 0xFFFF;

    current_font.width  = (FT_UShort)pixel_size;
    current_font.height = (FT_UShort)pixel_size;
  }


  void
  set_current_pointsize( int  point_size )
  {
    set_current_size( ( point_size * res + 36 ) / 72 );
  }


  void
  set_current_image_type()
  {
    current_font.flags = antialias ? FT_LOAD_DEFAULT : FT_LOAD_TARGET_MONO;

    current_font.flags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

    if ( !hinted )
      current_font.flags |= FT_LOAD_NO_HINTING;

    if ( autohint )
      current_font.flags |= FT_LOAD_FORCE_AUTOHINT;

    if ( !use_sbits )
      current_font.flags |= FT_LOAD_NO_BITMAP;

    if ( antialias && lcd_mode > 0 )
    {
      if ( lcd_mode <= 1 )
        current_font.flags |= FT_LOAD_TARGET_LIGHT;
      else if ( lcd_mode <= 3 )
        current_font.flags |= FT_LOAD_TARGET_LCD;
      else
        current_font.flags |= FT_LOAD_TARGET_LCD_V;
    }
  }


  void
  done_glyph_bitmap( FT_Pointer  _glyf )
  {
    if ( _glyf )
    {
      FT_Glyph  glyf = (FT_Glyph)_glyf;

      FT_Done_Glyph( glyf );
    }
  }


  FT_UInt
  get_glyph_index( FT_UInt32  charcode )
  {
    FTC_FaceID  face_id = current_font.face_id;
    PFont       font    = (PFont)face_id;


    return FTC_CMapCache_Lookup( cmap_cache, face_id,
                                 font->cmap_index, charcode );
  }


  FT_Error
  glyph_to_bitmap( FT_Glyph    glyf,
                   grBitmap*   target,
                   int        *left,
                   int        *top,
                   int        *x_advance,
                   int        *y_advance,
                   FT_Pointer *aref )
  {
    FT_BitmapGlyph  bitmap;
    FT_Bitmap*      source;


    *aref = NULL;

    ft_err = FT_Err_Ok;

    if ( glyf->format == FT_GLYPH_FORMAT_OUTLINE )
    {
      /* render the glyph to a bitmap, don't destroy original */
      ft_err = FT_Glyph_To_Bitmap( &glyf,
                                  antialias ? FT_RENDER_MODE_NORMAL
                                            : FT_RENDER_MODE_MONO,
                                  NULL, 0 );
      if ( ft_err )
        goto Exit;

      *aref = glyf;
    }

    if ( glyf->format != FT_GLYPH_FORMAT_BITMAP )
      PanicZ( "invalid glyph format returned!" );

    bitmap = (FT_BitmapGlyph)glyf;
    source = &bitmap->bitmap;

    target->rows   = source->rows;
    target->width  = source->width;
    target->pitch  = source->pitch;
    target->buffer = source->buffer;

    switch ( source->pixel_mode )
    {
    case FT_PIXEL_MODE_MONO:
      target->mode  = gr_pixel_mode_mono;
      target->grays = 2;
      break;

    case FT_PIXEL_MODE_GRAY:
      target->mode  = gr_pixel_mode_gray;
      target->grays = source->num_grays;
      break;

    case FT_PIXEL_MODE_GRAY2:
    case FT_PIXEL_MODE_GRAY4:
      (void)FT_Bitmap_Convert( ll_ftlib, source, &ft_bitmap, 1 );
      target->pitch  = ft_bitmap.pitch;
      target->buffer = ft_bitmap.buffer;
      target->mode   = gr_pixel_mode_gray;
      target->grays  = ft_bitmap.num_grays;
      break;

    case FT_PIXEL_MODE_LCD:
      target->mode  = lcd_mode == 2 ? gr_pixel_mode_lcd
                                    : gr_pixel_mode_lcd2;
      target->grays = source->num_grays;
      break;

    case FT_PIXEL_MODE_LCD_V:
      target->mode  = lcd_mode == 4 ? gr_pixel_mode_lcdv
                                    : gr_pixel_mode_lcdv2;
      target->grays = source->num_grays;
      break;

    default:
      return FT_Err_Invalid_Glyph_Format;
    }

    *left = bitmap->left;
    *top  = bitmap->top;

    *x_advance = ( glyf->advance.x + 0x8000 ) >> 16;
    *y_advance = ( glyf->advance.y + 0x8000 ) >> 16;

  Exit:
    return ft_err;
  }


  FT_Error
  get_glyph_bitmap( FT_ULong     Index,
                    grBitmap*    target,
                    int         *left,
                    int         *top,
                    int         *x_advance,
                    int         *y_advance,
                    FT_Pointer  *aglyf )
  {
    *aglyf = NULL;

    if ( encoding != FT_ENCODING_NONE )
      Index = get_glyph_index( Index );

    /* use the SBits cache to store small glyph bitmaps; this is a lot */
    /* more memory-efficient                                           */
    /*                                                                 */
    if ( use_sbits_cache          &&
         current_font.width  < 48 &&
         current_font.height < 48 )
    {
      FTC_SBit   sbit;
      FT_Bitmap  source;


      ft_err = FTC_SBitCache_Lookup( sbits_cache,
                                    &current_font,
                                    Index,
                                    &sbit,
                                    NULL );
      if ( ft_err )
        goto Exit;

      if ( sbit->buffer )
      {
        target->rows   = sbit->height;
        target->width  = sbit->width;
        target->pitch  = sbit->pitch;
        target->buffer = sbit->buffer;

        switch ( sbit->format )
        {
        case FT_PIXEL_MODE_MONO:
          target->mode  = gr_pixel_mode_mono;
          target->grays = 2;
          break;

        case FT_PIXEL_MODE_GRAY:
          target->mode  = gr_pixel_mode_gray;
          target->grays = sbit->max_grays + 1;
          break;

        case FT_PIXEL_MODE_GRAY2:
        case FT_PIXEL_MODE_GRAY4:
          source.rows       = sbit->height;
          source.width      = sbit->width;
          source.pitch      = sbit->pitch;
          source.buffer     = sbit->buffer;
          source.pixel_mode = sbit->format;
          (void)FT_Bitmap_Convert( ll_ftlib, &source, &ft_bitmap, 1 );

          target->pitch  = ft_bitmap.pitch;
          target->buffer = ft_bitmap.buffer;
          target->mode   = gr_pixel_mode_gray;
          target->grays  = ft_bitmap.num_grays;
          break;

        case FT_PIXEL_MODE_LCD:
          target->mode  = lcd_mode == 2 ? gr_pixel_mode_lcd
                                        : gr_pixel_mode_lcd2;
          target->grays = sbit->max_grays + 1;
          break;

        case FT_PIXEL_MODE_LCD_V:
          target->mode  = lcd_mode == 4 ? gr_pixel_mode_lcdv
                                        : gr_pixel_mode_lcdv2;
          target->grays = sbit->max_grays + 1;
          break;

        default:
          return FT_Err_Invalid_Glyph_Format;
        }

        *left      = sbit->left;
        *top       = sbit->top;
        *x_advance = sbit->xadvance;
        *y_advance = sbit->yadvance;

        goto Exit;
      }
    }

    /* otherwise, use an image cache to store glyph outlines, and render */
    /* them on demand. we can thus support very large sizes easily..     */
    {
      FT_Glyph   glyf;

      ft_err = FTC_ImageCache_Lookup( image_cache,
                                     &current_font,
                                     Index,
                                     &glyf,
                                     NULL );

      if ( !ft_err )
        ft_err = glyph_to_bitmap( glyf, target, left, top, x_advance, y_advance, aglyf );
    }

  Exit:
    /* don't accept a `missing' character with zero or negative width */
    if ( Index == 0 && *x_advance <= 0 )
      *x_advance = 1;

    return ft_err;
  }


/* End */

/*****************************************************************************/

  static FT_Error
  Render_All( int  first_index , const char *raw)
  {
    FT_F26Dot6     start_x, start_y, step_x, step_y, x, y;
    FTC_ScalerRec  scaler;
    FT_Pointer     glyf;
    int            i;
    grBitmap       bit3;

    start_x = 4;
    start_y = 16 + current_font.height;

    scaler.face_id = current_font.face_id;
    scaler.width   = current_font.width;
    scaler.height  = current_font.height;
    scaler.pixel   = 1;

    ft_err = FTC_Manager_LookupSize( cache_manager, &scaler, &size );
    if ( ft_err )
    {
      /* probably a non-existent bitmap font size */
      return ft_err;
    }

    step_x = size->metrics.x_ppem + 4;
    step_y = ( size->metrics.height >> 6 ) + 4;

    x = start_x;
    y = start_y;

    i = first_index;

#if 0
    while ( i < first_index + 1 )
#else
    while ( i < num_indices )
#endif
    {
      int  x_top, y_top, left, top, x_advance, y_advance;

      ft_err = get_glyph_bitmap( i, &bit3, &left, &top,
                                &x_advance, &y_advance, &glyf );
      if ( !ft_err )
      {
        /* now render the bitmap into the display surface */
        x_top = x + left;
        y_top = y - top;

//        printf("grBlitGlyphToBitmap %d,%d\n", x_top, y_top);

        grBlitGlyphToBitmap( &bit, &bit3, x_top, y_top, fore_color );

//        printf("%dx%d\n", bit3.width, bit3.rows);

        if ( glyf )
          done_glyph_bitmap( glyf );

        x += x_advance + 1;

        if ( x + size->metrics.x_ppem > bit.width )
        {
          x  = start_x;
          y += step_y;

          if ( y >= bit.rows )
            goto gg;
        }
      }
      else
        Fail++;

      i++;
    }

gg:

    FILE *f = fopen(raw, "wb");

    if(!f)
        return 1;

    int bb = 24;

    fwrite(&bit.width, sizeof(int), 1, f);
    fwrite(&bit.rows, sizeof(int), 1, f);
    fwrite(&bb, sizeof(int), 1, f);
    fwrite(bit.buffer, bit.width * bit.rows * 3, 1, f);

    fclose(f);

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                REST OF THE APPLICATION/PROGRAM                *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  int call(const char *arg, const char *rawrgb)
  {
    int          old_ptsize, orig_ptsize, font_index;
    int          first_index = 0;

    orig_ptsize = 20;

    /* Initialize engine */
    init_freetype();

    if(install_font_file(arg) || !num_fonts)
    {
        done_freetype();
        return 1;
    }

    font_index = 0;
    ptsize     = orig_ptsize;

    set_current_face( fonts[font_index] );
    set_current_pointsize( ptsize );
    set_current_image_type();
    num_indices = fonts[font_index]->num_indices;

    ft_err = FTC_Manager_LookupFace(cache_manager, current_font.face_id, &face);

    if(ft_err)
    {
      // can't access the font file; do not render anything 
      fprintf(stderr, "can't access font file %p, %d\n", current_font.face_id, ft_err);
      return 1;
    }

    Init_Display();

    grNewBitmap(bit.mode,
                bit.grays,
                bit.width,
                bit.rows,
                &bit);

    old_ptsize = ptsize;

    if ( num_fonts >= 1 )
    {
      Fail = 0;
      Num  = first_index;

      if ( Num >= num_indices )
        Num = num_indices - 1;

      if ( Num < 0 )
        Num = 0;
    }

    Clear_Display();

    ft_err = Render_All( Num, rawrgb );

    done_freetype();

    free(bit.buffer);

    return 0;
  }

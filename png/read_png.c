/*  This file is part of SQuirrel (http://ksquirrel.sf.net) libraries

    Copyright (c) 2004 Dmitry Baryshev <ckult@yandex.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libiberty.h>

#include "read_png.h"

#include "png.h"

#ifndef png_jmpbuf
    #define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif


int fmt_readable()
{
    return SQERR_OK;
}

char* fmt_version()
{
    return "0.9.1";
}

char* fmt_quickinfo()
{
    return "Portable Network Graphics";
}

char* fmt_extension()
{
    return "png ";
}

int fmt_writeable()
{
    return SQERR_NOTOK;
}

/* inits decoding of 'file': opens it, fills struct fmt_info  */
int fmt_init(fmt_info **finfo, const char *file)
{
    *finfo = (fmt_info*)calloc(1, sizeof(fmt_info));
    
    if(!finfo)
	return SQERR_NOMEMORY;
	
    (*finfo)->w = 0;
    (*finfo)->h = 0;
    (*finfo)->bpp = 0;
    (*finfo)->hasalpha = FALSE;
    (*finfo)->needflip = FALSE;
    (*finfo)->images = 1;
    (*finfo)->animated = FALSE;

    (*finfo)->fptr = fopen(file, "rb");
    
    if(!((*finfo)->fptr))
    {
	free(*finfo);
	return SQERR_NOFILE;
    }
    
    return SQERR_OK;
}


#define PNG_BYTES_TO_CHECK 4

png_structp	png_ptr;
png_infop	info_ptr;
png_uint_32	width, height, number_passes;
int		color_type;
png_bytep	*row_pointers;
int row;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    char	buf[PNG_BYTES_TO_CHECK+1];
    int		bit_depth, interlace_type;

    if(fread(buf, 1, PNG_BYTES_TO_CHECK, finfo->fptr) != PNG_BYTES_TO_CHECK)
	return SQERR_BADFILE;

    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
    {
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
	fclose(finfo->fptr);
	png_destroy_read_struct(&png_ptr, 0, 0);
	return SQERR_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    png_init_io(png_ptr, finfo->fptr);
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)NULL, (int*)NULL);

    finfo->w = width;
    finfo->h = height;
    finfo->bpp = bit_depth;

    if(finfo->bpp == 16)
	png_set_strip_16(png_ptr);

    if(finfo->bpp < 8)
	png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && finfo->bpp < 8)
	png_set_gray_1_2_4_to_8(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
	png_set_palette_to_rgb(png_ptr);
	png_get_PLTE(png_ptr, info_ptr, (png_colorp*)&finfo->pal, (int*)&finfo->pal_entr);
    }
    else
	finfo->pal = 0;
	
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb(png_ptr);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(png_ptr);

    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    number_passes = png_set_interlace_handling(png_ptr);
    
    png_read_update_info(png_ptr, info_ptr);
/*    
    row_pointers = (png_bytep*)calloc(finfo->h, sizeof(png_bytep));

    for(row = 0; row < finfo->h; row++)
	row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

    row = 0;
	
    png_read_image(png_ptr, row_pointers);
*/
    asprintf(&finfo->dump, "%s\n%ldx%ld\n%d\n%s\nNO\n%d\n",
    fmt_quickinfo(),
    finfo->w,finfo->h,
    finfo->bpp,(finfo->pal_entr)?"Color indexed":"RGB",
    finfo->images);

    return SQERR_OK;
}


/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    png_bytep rows[1];

    memset(scan, 255, finfo->w * 4);

    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    rows[0] = (png_bytep)png_malloc(png_ptr, row_bytes);
    
    png_read_rows(png_ptr, &rows[0], png_bytepp_NULL, 1);

    memcpy(scan, rows[0], finfo->w * 4);

    return SQERR_OK;
}

void fmt_readimage(fmt_info *finfo, RGBA *image)
{
    unsigned int i = 0;

    for(;i < finfo->h;i++)
        fmt_read_scanline(finfo, image + i*finfo->w);
}

int fmt_close(fmt_info *finfo)
{
//    for(row = 0; row < finfo->h; row++)
//	png_free(png_ptr, row_pointers[row]);

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fclose(finfo->fptr);

    return SQERR_OK;
}

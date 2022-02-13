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
#include <string.h>
#include <libiberty.h>

#include "read_jpeg.h"


char* fmt_version()
{
    return "0.8.2";
}

char* fmt_quickinfo()
{
    return "JPEG compressed";
}

char* fmt_extension()
{
    return "jpg jpeg jpe ";
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


METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

struct jpeg_decompress_struct	cinfo;
struct my_error_mgr 		jerr;
int 				row_stride;
JSAMPARRAY 			buffer;

int fmt_read_info(fmt_info *finfo)
{
    int i;


    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    
    if(setjmp(jerr.setjmp_buffer)) 
    {
	jpeg_destroy_decompress(&cinfo);
	fclose(finfo->fptr);
	return SQERR_NOTOK;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, finfo->fptr);
    jpeg_read_header(&cinfo, TRUE);

    if(cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
	finfo->bpp = 8;
        cinfo.out_color_space = JCS_RGB;
	cinfo.desired_number_of_colors = 256;
	cinfo.quantize_colors = FALSE;
	cinfo.two_pass_quantize = FALSE;

	finfo->pal = (RGB*)calloc(256, sizeof(RGB));
	
	for(i = 0;i < 256;i++)
	    (finfo->pal)[i].r = (finfo->pal)[i].g = (finfo->pal)[i].b = i;
    }
    else
	finfo->bpp = 24;

    jpeg_start_decompress(&cinfo);

    finfo->w = cinfo.output_width;
    finfo->h = cinfo.output_height;
    finfo->pal_entr = 256;
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

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
    unsigned int i;

    memset(scan, 255, finfo->w * 4);

    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    for(i = 0;i < finfo->w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

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
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(finfo->fptr);

    return SQERR_OK;
}

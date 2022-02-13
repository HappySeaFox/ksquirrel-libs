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

#include "read_tiff.h"

#include <tiffio.h>


char* fmt_version()
{
    return "0.9";
}

char* fmt_quickinfo()
{
    return "Tagged Image File Format";
}

char* fmt_extension()
{
    return "*.tif *.tiff";
}


TIFF	*ftiff;
int	i;
TIFFRGBAImage img;

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
    
    if((ftiff = TIFFOpen(file,"r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

    return SQERR_OK;
}



/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    int bps, spp;

    TIFFGetField(ftiff, TIFFTAG_IMAGEWIDTH, &finfo->w);
    TIFFGetField(ftiff, TIFFTAG_IMAGELENGTH, &finfo->h);
    TIFFGetField(ftiff, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(ftiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
    
    finfo->bpp = (long)bps * spp;
    
    i = 1;

    memset(&img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&img, ftiff, 1, 0);

    asprintf(&finfo->dump, "Width: %ld\nHeight: %ld\nBits per pixel: %d\nNumber of images: %d\nAnimated: %s\nHas palette: %s\n",
    finfo->w,finfo->h,finfo->bpp,finfo->images,(finfo->animated)?"yes":"no",(finfo->pal_entr)?"yes":"no");

    return SQERR_OK;
}



/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
 
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    uint32 buf[finfo->w * 4];

    memset(scan, 255, finfo->w * 4);

    TIFFRGBAImageGet(&img, buf, finfo->w, 1);

    memcpy(scan, buf, finfo->w * 4);

    img.row_offset++;

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    TIFFRGBAImageEnd(&img);

    fclose(finfo->fptr);

    TIFFClose(ftiff);

    return SQERR_OK;
}

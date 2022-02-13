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

#define BE_SHORT

#include "read_pix.h"
#include "../endian.h"

char* fmt_version()
{
    return "1.0";
}

char* fmt_quickinfo()
{
    return "Irix PIX image";
}

char* fmt_extension()
{
    return "*.pix";
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


PIX_HEADER	pfh;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    pfh.width = BE_getshort(finfo->fptr);
    pfh.height = BE_getshort(finfo->fptr);
    pfh.x = BE_getshort(finfo->fptr);
    pfh.y = BE_getshort(finfo->fptr);
    pfh.bpp = BE_getshort(finfo->fptr);
    
    if(pfh.bpp != 24) return SQERR_BADFILE;

    finfo->w = pfh.width;
    finfo->h = pfh.height;
    finfo->bpp = pfh.bpp;
    finfo->pal = 0;
    finfo->pal_entr = 0;

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
    uint	len = 0, i, counter = 0;
    uchar	count;
    RGB		rgb;

    memset(scan, 255, finfo->w * 4);

    switch(finfo->bpp)
    {
	case 24:
	    do
	    {
		count = fgetc(finfo->fptr);
		len += count;
		
		fread(&rgb.b, 1, 1, finfo->fptr);
		fread(&rgb.g, 1, 1, finfo->fptr);
		fread(&rgb.r, 1, 1, finfo->fptr);
		
		for(i = 0;i < count;i++)
		    memcpy(scan+counter++, &rgb, 3);
		    
	    }while(len < finfo->w);
	break;

	default:
	{
		//@TODO:  free memory !!
		return SQERR_BADFILE;
	}
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

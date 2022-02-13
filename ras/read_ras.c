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

#define BE_LONG

#include "read_ras.h"
#include "../endian.h"

char* fmt_version()
{
    return "0.1";
}

char* fmt_quickinfo()
{
    return "SUN Raster file";
}

char* fmt_extension()
{
    return "*.ras";
}

#define RAS_MAGIC 0x59a66a95

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

RAS_HEADER rfh;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    rfh.ras_magic = BE_getlong(finfo->fptr);
    rfh.ras_width = BE_getlong(finfo->fptr);
    rfh.ras_height = BE_getlong(finfo->fptr);
    rfh.ras_depth = BE_getlong(finfo->fptr);
    rfh.ras_length = BE_getlong(finfo->fptr); 
    rfh.ras_type = BE_getlong(finfo->fptr);
    rfh.ras_maptype = BE_getlong(finfo->fptr);
    rfh.ras_maplength = BE_getlong(finfo->fptr);

    if(rfh.ras_magic != RAS_MAGIC) return SQERR_BADFILE;
    
    if(rfh.ras_type != ras_old && rfh.ras_type != ras_standard && rfh.ras_type != ras_byte_encoded && rfh.ras_type != ras_experimental)
	return SQERR_BADFILE;
    else if(rfh.ras_type == ras_experimental || rfh.ras_type == ras_byte_encoded)
	return SQERR_NOTSUPPORTED;
    

    finfo->w = rfh.ras_width;
    finfo->h = rfh.ras_height;
    finfo->bpp = rfh.ras_depth;

    asprintf(&finfo->dump, "Magic Number: 0x%x\nWidth: %ld\nHeight: %ld\nBits per pixel: %d\nNumber of images: %d\nAnimated: %s\nHas palette: %s\n",
    rfh.ras_magic, finfo->w,finfo->h,finfo->bpp,finfo->images,(finfo->animated)?"yes":"no",(finfo->pal_entr)?"yes":"no");

    return SQERR_OK;
}


/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int i;
    RGB rgb;

    memset(scan, 255, finfo->w * 4);

    switch(finfo->bpp)
    {
    	case 1:
	break;

	case 8:
	break;

	case 24:
	    if(rfh.ras_type != ras_byte_encoded)
		for(i = 0;i < finfo->w;i++)
		{
		    fread(&rgb, sizeof(RGB), 1, finfo->fptr);
		
		    scan[i].r = rgb.b;
		    scan[i].g = rgb.g;
		    scan[i].b = rgb.r;
		}
	    else
	    {
		
	    }
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

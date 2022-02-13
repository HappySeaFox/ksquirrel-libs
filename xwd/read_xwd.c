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

#include "read_xwd.h"
#include "../endian.h"

char* fmt_version()
{
    return "0.3";
}

char* fmt_quickinfo()
{
    return "X Window Dump";
}

char* fmt_extension()
{
    return "*.xwd";
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

XWDFileHeader	xfh;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    XWDColor	color;
    char 	str[256];
    int		i, ncolors;

    fread(&xfh, sizeof(XWDFileHeader), 1, finfo->fptr);
    
    fgets(str, 255, finfo->fptr); // get window name
    
    fseek(finfo->fptr, lLE2BE(xfh.header_size), SEEK_SET);

    finfo->pal_entr = ncolors = lLE2BE(xfh.ncolors);
    
    if((finfo->pal = (RGB*)calloc(ncolors, sizeof(RGB))) == NULL)
    {
	fclose(finfo->fptr);	
	return SQERR_NOMEMORY;
    }
    
    for(i = 0;i < ncolors;i++)
    {
	fread(&color, sizeof(XWDColor), 1, finfo->fptr);
	
	(finfo->pal)[i].r = (uchar)sLE2BE(color.red);
	(finfo->pal)[i].g = (uchar)sLE2BE(color.green);
	(finfo->pal)[i].b = (uchar)sLE2BE(color.blue);
    }

    finfo->w = lLE2BE(xfh.pixmap_width);
    finfo->h = lLE2BE(xfh.pixmap_height);
    finfo->bpp = lLE2BE(xfh.pixmap_depth);

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
    int 	i;
    RGBA	rgba;

    memset(scan, 255, finfo->w * 4);

    for(i = 0;i < finfo->w;i++)
    {
	fread(&rgba, sizeof(RGBA), 1, finfo->fptr);

	scan[i].r = rgba.b;
	scan[i].g = rgba.g;
	scan[i].b = rgba.r;
	scan[i].a = 255;
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

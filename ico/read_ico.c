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

#include "read_ico.h"


char* fmt_version()
{
    return "0.1";
}

char* fmt_quickinfo()
{
    return "Windows Icons";
}

char* fmt_extension()
{
    return "*.ico *.cur";
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
    (*finfo)->hasalpha = TRUE;
    (*finfo)->needflip = TRUE;
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


uchar			*bAND;
ICO_HEADER		ifh;
ICO_DIRENTRY		ide;
BITMAPINFO_HEADER	bih;


/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    RGBA		rgba;
    long		i, pos;

    fread(&ifh, sizeof(ICO_HEADER), 1, finfo->fptr);
    
    if(ifh.idType != 1 && ifh.idType != 2)
	return SQERR_BADFILE;

    fread(&ide, sizeof(ICO_DIRENTRY), 1, finfo->fptr);

    finfo->w = ide.bWidth;
    finfo->h = ide.bHeight;
    finfo->images = ifh.idCount;
    
/*
    if(finfo->w != 16 && finfo->w != 32 && finfo->w != 64)
	return SQERR_BADFILE;

    if(finfo->h != 16 && finfo->h != 32 && finfo->h != 64)
	return SQERR_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQERR_BADFILE;
*/

    fseek(finfo->fptr, ide.dwImageOffset, SEEK_SET);
    fread(&bih, sizeof(BITMAPINFO_HEADER), 1, finfo->fptr);

    pos = ftell(finfo->fptr);
    
    fseek(finfo->fptr, finfo->h * bih.Planes * bih.BitCount * finfo->w / 8, SEEK_CUR);
    
    int count = finfo->w * finfo->h / 8;
    
    bAND = (uchar*)calloc(count, sizeof(uchar));
    fread(bAND, count, 1, finfo->fptr);
    
//    for(i = 0;i < count;i++)
//	printf("%4d%s",bAND[i],((i+1)%8)?" ":"\n");
    
    fsetpos(finfo->fptr, (fpos_t*)&pos);

    finfo->bpp = bih.BitCount;
    finfo->pal_entr = 1 << finfo->bpp;

    if(finfo->bpp < 16)
    {
	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
		fclose(finfo->fptr);
		free(finfo);
		return SQERR_NOMEMORY;
	}

	/*  read palette  */
	for(i = 0;i < finfo->pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, finfo->fptr);
		(finfo->pal)[i].r = rgba.b;
		(finfo->pal)[i].g = rgba.g;
		(finfo->pal)[i].b = rgba.r;
		
	}
    }
    else
	finfo->pal = 0;
/*
    for(i = 0;i < finfo->pal_entr;i++)
	printf("%d %d %d\n",(finfo->pal)[i].r,(finfo->pal)[i].g,(finfo->pal)[i].b);
*/

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

    long i, j;
    unsigned char bt, ind;

    memset(scan, 255, finfo->w * 4);

    switch(finfo->bpp)
    {
    	case 1:
	break;

	case 4:
	    j = 0;
	    do
	    {
		bt = fgetc(finfo->fptr);
		ind = bt >> 4;
		memcpy(scan+j++, (finfo->pal)+ind, 3);
		ind = bt&15;
		memcpy(scan+j++, (finfo->pal)+ind, 3);
	    }while(j < finfo->w);

	break;

	case 8:
	    for(i = 0;i < finfo->w;i++)
	    {
		bt = fgetc(finfo->fptr);
		memcpy(scan+i, (finfo->pal)+bt, 3);
	    }
	break;

	case 16:
	break;

	case 24:
	break;

	case 32:
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

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
#define BE_LONG

#include "read_sgi.h"
#include "../endian.h"

char* fmt_version()
{
    return "0.8";
}

char* fmt_quickinfo()
{
    return "SGI format";
}

char* fmt_extension()
{
    return "*.sgi *.rgb *.rgba *.bw";
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

ulong		*starttab, *lengthtab;
SGI_HEADER	sfh;
int rle_row;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    sfh.Magik = BE_getshort(finfo->fptr);
    sfh.StorageFormat = fgetc(finfo->fptr);
    sfh.bpc = fgetc(finfo->fptr);
    sfh.Dimensions = BE_getshort(finfo->fptr);
    sfh.x = BE_getshort(finfo->fptr);
    sfh.y = BE_getshort(finfo->fptr);
    sfh.z = BE_getshort(finfo->fptr);
    sfh.pixmin = BE_getlong(finfo->fptr);
    sfh.pixmax = BE_getlong(finfo->fptr);
    sfh.dummy = BE_getlong(finfo->fptr);
    fread(sfh.name, sizeof(sfh.name), 1, finfo->fptr);
    sfh.ColormapID = BE_getlong(finfo->fptr);
    fread(sfh.dummy2, sizeof(sfh.dummy2), 1, finfo->fptr);

    finfo->w = sfh.x;
    finfo->h = sfh.y;
    finfo->bpp = sfh.bpc * sfh.z * 8;
    
    if(finfo->bpp == 32) finfo->hasalpha = TRUE;

    if(sfh.Magik != 474 || (sfh.StorageFormat != 0 && sfh.StorageFormat != 1) || (sfh.Dimensions != 1 && sfh.Dimensions != 2 && sfh.Dimensions != 3) || (sfh.bpc != 1 && sfh.bpc != 2))
	return SQERR_BADFILE;

    if(sfh.bpc == 2 || sfh.ColormapID > 0)
	return SQERR_NOTSUPPORTED;

    // SGI has no colormap
    finfo->pal_entr = 0;
    finfo->pal = 0;

    if(sfh.StorageFormat == 1)
    {
	long sz = sfh.y * sfh.z, i;
        lengthtab = (ulong*)calloc(sz, sizeof(ulong));
	starttab = (ulong*)calloc(sz, sizeof(ulong));
    
        if(lengthtab == NULL)
    	    return SQERR_NOMEMORY;

        if(starttab == NULL)
	{
	    free(lengthtab);
	    return SQERR_NOMEMORY;
	}

	fseek(finfo->fptr, 512, SEEK_SET);
    
	for(i = 0;i < sz;i++)
	    starttab[i] = BE_getlong(finfo->fptr);

	for(i = 0;i < sz;i++)
	    lengthtab[i] = BE_getlong(finfo->fptr);
    }

    rle_row = 0;

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
    const int sz = sfh.x;
    int i = 0, j = 0;
    long pos, len;

    memset(scan, 255, finfo->w * 4);

    // channel[0] == channel RED, channel[1] = channel GREEN...
    uchar	channel[4][sz];
    uchar	bt;

    // I think we don't need to memset r,g,b channels - only ALPHA channel
    memset(channel[3], 255, sz);

    switch(sfh.z)
    {
    	case 1:
	{
	    if(sfh.StorageFormat)
	    {
		    j = 0;

		    fseek(finfo->fptr, starttab[rle_row], SEEK_SET);
		    len = lengthtab[rle_row];

		    for(;;)
		    {
			char count;
		    
    			bt = fgetc(finfo->fptr);
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    channel[0][j++] = fgetc(finfo->fptr); 
				    if(!len--) goto ex1;
				}
			else
			{
			    bt = fgetc(finfo->fptr);
			    if(!len--) goto ex1;

			    while(count--)
				channel[0][j++] = bt;
			}
		    }
		    ex1:
		    len = len; // some stuff: get rid of compile warning

		rle_row++;
	    }
	    else
		fread(channel[0], sz, 1, finfo->fptr);

	    memcpy(channel[1], channel[0], sz);
	    memcpy(channel[2], channel[0], sz);
	}
	break;


	case 3:
	case 4:
	{
	    if(sfh.StorageFormat)
	    {
		for(i = 0;i < sfh.z;i++)
		{
		    j = 0;

		    fseek(finfo->fptr, starttab[rle_row + i*finfo->h], SEEK_SET);
		    len = lengthtab[rle_row + i*finfo->h];

		    for(;;)
		    {
			char count;
		    
    			bt = fgetc(finfo->fptr);
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    channel[i][j++] = fgetc(finfo->fptr); 
				    if(!len--) goto ex;
				}
			else
			{
			    bt = fgetc(finfo->fptr);
			    if(!len--) goto ex;

			    while(count--)
				channel[i][j++] = bt;
			}
		    }
		    ex:
		    len = len; // some stuff: get rid of compile warning
		}
		rle_row++;
	    }
	    else
	    {
		fread(channel[0], sz, 1, finfo->fptr);
		pos = ftell(finfo->fptr);
		fseek(finfo->fptr, finfo->w * (finfo->h - 1), SEEK_CUR);
		fread(channel[1], sz, 1, finfo->fptr);
		fseek(finfo->fptr, finfo->w * (finfo->h - 1), SEEK_CUR);
		fread(channel[2], sz, 1, finfo->fptr);
		fseek(finfo->fptr, finfo->w * (finfo->h - 1), SEEK_CUR);
		fread(channel[3], sz, 1, finfo->fptr);
		fsetpos(finfo->fptr, (fpos_t*)&pos);
		
	    }

	}
	break;

	default:
	{
		//@TODO:  free memory !!
		return SQERR_BADFILE;
	}
    }

    for(i = 0;i < sz;i++)
    {
        scan[i].r = channel[0][i];
        scan[i].g = channel[1][i];
        scan[i].b = channel[2][i];
        scan[i].a = channel[3][i];
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

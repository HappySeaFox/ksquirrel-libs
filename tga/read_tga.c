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

#include "read_tga.h"


char* fmt_version()
{
    return "0.6";
}

char* fmt_quickinfo()
{
    return "TarGA";
}

char* fmt_extension()
{
    return "*.tga";
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

TGA_FILEHEADER	tfh;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    fread(&tfh, sizeof(TGA_FILEHEADER), 1, finfo->fptr);

    finfo->w = tfh.ImageSpecW;
    finfo->h = tfh.ImageSpecH;
    finfo->bpp = tfh.ImageSpecDepth;
    finfo->pal_entr = 0;


    if(tfh.ColorMapType)
    {
	finfo->pal_entr = tfh.ColorMapSpecLength;

	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
		fclose(finfo->fptr);
		free(finfo);
		return SQERR_NOMEMORY;
	}

//	char sz = tfh.ColorMapSpecEntrySize;
	int  i;
//	unsigned short word;
  
	for(i = 0;i < finfo->pal_entr;i++)
	{
		/*if(sz==24)*/ fread(finfo->pal+i, sizeof(RGB), 1, finfo->fptr);
/* alpha ingored  *//*else if(sz==32) { fread(finfo->pal+i, sizeof(RGB), 1, finfo->fptr); fgetc(finfo->fptr); }
		else if(sz==16)
		{
		    fread(&word, 2, 1, finfo->fptr);
		    (finfo->pal)[i].b = (word&0x1f) << 3;
		    (finfo->pal)[i].g = ((word&0x3e0) >> 5) << 3;
		    (finfo->pal)[i].r = ((word&0x7c00)>>10) << 3;
		}*/
		
	}
    }
    else
	finfo->pal = 0;

    if(tfh.ImageType == 0)
	return SQERR_BADFILE;

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

    long j, counter = 0;

    memset(scan, 255, finfo->w * 4);

    switch(tfh.ImageType)
    {
    	case 0:
	break;

	case 1:
	{
	}
	break;

	case 2:
	{
	    if(tfh.ImageSpecDepth==24)
	    {
		for(j = 0;j < finfo->w;j++)
		    fread(scan+(counter++), sizeof(RGB), 1, finfo->fptr);
	    }
	    else if(tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < finfo->w;j++)
		    fread(scan+(counter++), sizeof(RGBA), 1, finfo->fptr);
	    }
	    else if(tfh.ImageSpecDepth==16)
	    {
		unsigned short word;

		for(j = 0;j < finfo->w;j++)
		{
		    fread(&word, 2, 1, finfo->fptr);
		    scan[counter].b = (word&0x1f) << 3;
		    scan[counter].g = ((word&0x3e0) >> 5) << 3;
		    scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}
	    }
	}
	break;

	case 3:
	{
	}
	break;

	// RLE + color mapped
	case 9:
	{
	}
	break;

	// RLE + true color
	case 10:
	{
	    uchar	bt, count;
	    ushort	counter = 0, word;
	    RGB		rgb;
	    RGBA	rgba;
	    
	    for(;;)
	    {
		bt = fgetc(finfo->fptr);
		count = (bt & 127) + 1;
		
    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(finfo->bpp)
		    {
			case 16:
    			    fread(&word, 2, 1, finfo->fptr);

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;

			case 24:
    			    fread(&rgb, sizeof(RGB), 1, finfo->fptr);

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;

			case 32:
    			    fread(&rgba, sizeof(RGBA), 1, finfo->fptr);

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgba, sizeof(RGBA));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(finfo->bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				fread(&word, 2, 1, finfo->fptr);

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				fread(&rgb, sizeof(RGB), 1, finfo->fptr);
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				fread(&rgba, sizeof(RGBA), 1, finfo->fptr);
				memcpy(scan+(counter++), &rgba, sizeof(RGBA));
				if(counter >= finfo->w-1) goto lts;
			    }
			break;
		    }
		}
	    }
	}
	lts:
	break;

	// RLE + B&W
	case 11:
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

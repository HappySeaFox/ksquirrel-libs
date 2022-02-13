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

#include "read_pcx.h"

#define PCX_COLORMAP_SIGNATURE		(0x0c)
#define PCX_COLORMAP_SIGNATURE_NEW	(0x0a)

void getrow(FILE*, unsigned char*, int);


char* fmt_version()
{
    return "0.4";
}

char* fmt_quickinfo()
{
    return "ZSoft PCX";
}

char* fmt_extension()
{
    return "*.pcx";
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

PCX_HEADER	pfh;
short		TotalBytesLine;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    fread(&pfh, sizeof(PCX_HEADER), 1, finfo->fptr);

    if(pfh.ID != 10 || pfh.Encoding != 1)
	return SQERR_BADFILE;

    finfo->w = pfh.Xmax - pfh.Xmin + 1;
    finfo->h = pfh.Ymax - pfh.Ymin + 1;
    finfo->bpp = pfh.bpp * pfh.NPlanes;
    finfo->pal_entr = 0;

    if(pfh.bpp == 1)
    {
	finfo->pal_entr = 2;

	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
	    fclose(finfo->fptr);
	    return SQERR_NOMEMORY;
	}

	memset(finfo->pal+0, (uchar)0, 3);
	memset(finfo->pal+1, (uchar)255, 3);
    
    }
    else if(pfh.bpp <= 4)
    {
	finfo->pal_entr = 16;

	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
	    fclose(finfo->fptr);
	    return SQERR_NOMEMORY;
	}

	memcpy(finfo->pal, pfh.Palette, 48);
    }
    else if(pfh.bpp == 8 && pfh.NPlanes == 1)
    {
	finfo->pal_entr = 256;

	fseek(finfo->fptr, -769, SEEK_END);
	
	uchar test = fgetc(finfo->fptr);
	
	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQERR_BADFILE;
	
	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
	    fclose(finfo->fptr);
	    return SQERR_NOMEMORY;
	}

	fread(finfo->pal, 768, 1, finfo->fptr);
	
//	int i;
//	for(i=0;i<256;i++)
//	printf("%d %d %d\n",(finfo->pal)[i].r,(finfo->pal)[i].g,(finfo->pal)[i].b);
    }
    else
	finfo->pal = 0;

    fseek(finfo->fptr, 128, SEEK_SET);
/*    
    printf("ID: %d\nVersion: %d\nEncoding: %d\nbpp: %d\nNPlanes: %d\nBytesPerLine: %d\nPaletteInfo: %d\n",
    pfh.ID, pfh.Version, pfh.Encoding, pfh.bpp, pfh.NPlanes, pfh.BytesPerLine, pfh.PaletteInfo);
*/
    TotalBytesLine = pfh.NPlanes * pfh.BytesPerLine;

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

    ushort  i, j;
    uchar channel[4][finfo->w];
    uchar indexes[finfo->w];

    memset(scan, 255, finfo->w * 4);
    
    for(i = 0;i < 4;i++)
	memset(channel[i], 255, finfo->w);

    switch(finfo->bpp)
    {
    	case 1:
	{
	}
	break;

	case 4:
	{
	}
	break;

	case 8:
	    getrow(finfo->fptr, indexes, pfh.BytesPerLine);

	    for(i = 0;i < finfo->w;i++)
		memcpy(scan+i, (finfo->pal)+indexes[i], 3);
	break;

	case 16:
	{
	}
	break;

	case 24:
	{
	    for(j = 0;j < pfh.NPlanes;j++)
		getrow(finfo->fptr, channel[j], pfh.BytesPerLine);
	    
	    for(i = 0;i < finfo->w;i++)
	    {
    		scan[i].r = channel[0][i];
    		scan[i].g = channel[1][i];
    		scan[i].b = channel[2][i];
	    }
	}
	break;

	case 32:
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

void getrow(FILE *f, unsigned char *pcxrow, int bytesperline)
{
    static int 	repetitionsLeft = 0;
    static int 	c;
    int 	bytesGenerated;

    bytesGenerated = 0;
    while(bytesGenerated < bytesperline)
    {
        if(repetitionsLeft > 0)
	{
            pcxrow[bytesGenerated++] = c;
            --repetitionsLeft;
        }
	else
	{
	    c = fgetc(f);

	    if(c <= 192)
                pcxrow[bytesGenerated++] = c;
            else
	    {
                repetitionsLeft = c&63;
		c = fgetc(f);
            }
        }
    }
}

/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>

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

#include "read_pcx.h"

#define PCX_COLORMAP_SIGNATURE		(0x0c)
#define PCX_COLORMAP_SIGNATURE_NEW	(0x0a)

void getrow(FILE*, unsigned char*, int);

FILE 		*fptr;
int 		currentImage, bytes;
PCX_HEADER	pfh;
short		TotalBytesLine;
RGB		*pal;
int		pal_entr;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.7.1";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"ZSoft PCX";
}
	
const char* fmt_filter()
{
    return (const char*)"*.pcx ";
}
	    
const char* fmt_mime()
{
    return (const char*)"\x000A[\x0002\x0003\x0004\x0005]";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,128,128,128,4,4,4,171,39,77,152,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,78,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,139,147,138,138,19,144,161,162,162,2,100,130,69,160,12,21,4,3,170,6,166,11,110,14,196,100,37,81,168,165,96,91,193,206,8,0,0,88,48,23,89,192,219,238,61,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;
	
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    finfo->passes = 1;
    pal = 0;
    pal_entr = 0;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;
	    
    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    fread(&pfh, sizeof(PCX_HEADER), 1, fptr);

    if(pfh.ID != 10 || pfh.Encoding != 1)
	return SQERR_BADFILE;

    finfo->image[currentImage].w = pfh.Xmax - pfh.Xmin + 1;
    finfo->image[currentImage].h = pfh.Ymax - pfh.Ymin + 1;
    finfo->image[currentImage].bpp = pfh.bpp * pfh.NPlanes;
    pal_entr = 0;

    if(pfh.bpp == 1)
    {
	pal_entr = 2;

	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	memset(pal, 0, sizeof(RGB));
	memset(pal+1, 255, sizeof(RGB));

    }
    else if(pfh.bpp <= 4)
    {
	pal_entr = 16;

	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	memcpy(pal, pfh.Palette, 48);
    }
    else if(pfh.bpp == 8 && pfh.NPlanes == 1)
    {
	pal_entr = 256;

	fseek(fptr, -769, SEEK_END);
	
	uchar test = fgetc(fptr);
	
	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQERR_BADFILE;

	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	fread(pal, 768, 1, fptr);
	
//	int i;
//	for(i=0;i<256;i++)
//	printf("%d %d %d\n",(finfo->image[currentImage].pal)[i].r,(finfo->image[currentImage].pal)[i].g,(finfo->image[currentImage].pal)[i].b);
    }
    else
	pal = 0;

    fseek(fptr, 128, SEEK_SET);
/*    
    printf("ID: %d\nVersion: %d\nEncoding: %d\nbpp: %d\nNPlanes: %d\nBytesPerLine: %d\nPaletteInfo: %d\n",
    pfh.ID, pfh.Version, pfh.Encoding, pfh.bpp, pfh.NPlanes, pfh.BytesPerLine, pfh.PaletteInfo);
*/
    TotalBytesLine = pfh.NPlanes * pfh.BytesPerLine;

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->images++;
	
    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\nRLE\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGB",
	bytes);

    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    ushort  i, j;
    uchar channel[4][finfo->image[currentImage].w];
    uchar indexes[finfo->image[currentImage].w];

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
    
    for(i = 0;i < 4;i++)
	memset(channel[i], 255, finfo->image[currentImage].w);

    switch(finfo->image[currentImage].bpp)
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
	    getrow(fptr, indexes, pfh.BytesPerLine);

	    for(i = 0;i < finfo->image[currentImage].w;i++)
		memcpy(scan+i, pal+indexes[i], sizeof(RGB));
	break;

	case 16:
	{
	}
	break;

	case 24:
	{
	    for(j = 0;j < pfh.NPlanes;j++)
		getrow(fptr, channel[j], pfh.BytesPerLine);

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
    		scan[i].r = channel[0][i];
    		scan[i].g = channel[1][i];
    		scan[i].b = channel[2][i];
	    }
	}
	break;

	default:;
    }

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    FILE *m_fptr;
    int w, h, bpp;
    PCX_HEADER	m_pfh;
    short	m_TotalBytesLine;
    RGB		*m_pal;
    int		m_pal_entr;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    fread(&m_pfh, sizeof(PCX_HEADER), 1, m_fptr);

    if(m_pfh.ID != 10 || m_pfh.Encoding != 1)
	return SQERR_BADFILE;

    w = m_pfh.Xmax - m_pfh.Xmin + 1;
    h = m_pfh.Ymax - m_pfh.Ymin + 1;
    bpp = m_pfh.bpp * m_pfh.NPlanes;
    m_pal_entr = 0;

    if(m_pfh.bpp == 1)
    {
	m_pal_entr = 2;

	if((m_pal = (RGB*)calloc(m_pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	memset(m_pal, 0, sizeof(RGB));
	memset(m_pal+1, 255, sizeof(RGB));

    }
    else if(m_pfh.bpp <= 4)
    {
	m_pal_entr = 16;

	if((m_pal = (RGB*)calloc(m_pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	memcpy(m_pal, m_pfh.Palette, 48);
    }
    else if(m_pfh.bpp == 8 && m_pfh.NPlanes == 1)
    {
	m_pal_entr = 256;

	fseek(m_fptr, -769, SEEK_END);
	
	uchar test = fgetc(m_fptr);
	
	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQERR_BADFILE;

	if((m_pal = (RGB*)calloc(m_pal_entr, sizeof(RGB))) == 0)
	{
	    return SQERR_NOMEMORY;
	}

	fread(m_pal, 768, 1, m_fptr);
	
//	int i;
//	for(i=0;i<256;i++)
//	printf("%d %d %d\n",(finfo->image[currentImage].m_pal)[i].r,(finfo->image[currentImage].m_pal)[i].g,(finfo->image[currentImage].m_pal)[i].b);
    }
    else
	m_pal = 0;

    fseek(m_fptr, 128, SEEK_SET);
/*    
    printf("ID: %d\nVersion: %d\nEncoding: %d\nbpp: %d\nNPlanes: %d\nBytesPerLine: %d\nPaletteInfo: %d\n",
    m_pfh.ID, m_pfh.Version, m_pfh.Encoding, m_pfh.bpp, m_pfh.NPlanes, m_pfh.BytesPerLine, m_pfh.PaletteInfo);
*/
    m_TotalBytesLine = m_pfh.NPlanes * m_pfh.BytesPerLine;

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\nRLE\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_pcx: Image is null!\n");
        fclose(m_fptr);
	
	if(m_pal)
	    free(m_pal);

        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    int W = w * sizeof(RGBA);
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	memset(scan, 255, W);

    ushort  i, j;
    uchar channel[4][w];
    uchar indexes[w];

    for(i = 0;i < 4;i++)
	memset(channel[i], 255, w);

    switch(bpp)
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
	    getrow(m_fptr, indexes, m_pfh.BytesPerLine);

	    for(i = 0;i < w;i++)
		memcpy(scan+i, m_pal+indexes[i], sizeof(RGB));
	break;

	case 16:
	{
	}
	break;

	case 24:
	{
	    for(j = 0;j < m_pfh.NPlanes;j++)
		getrow(m_fptr, channel[j], m_pfh.BytesPerLine);
	    
	    for(i = 0;i < w;i++)
	    {
    		scan[i].r = channel[0][i];
    		scan[i].g = channel[1][i];
    		scan[i].b = channel[2][i];
	    }
	}
	break;

	default:;
    }

    }

    fclose(m_fptr);
    
    if(m_pal)
	free(m_pal);

    return SQERR_OK;
}

    
int fmt_close()
{
    fclose(fptr);
    
    if(pal)
	free(pal);

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

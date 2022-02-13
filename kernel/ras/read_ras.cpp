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

#include "read_ras.h"
#include "endian.h"

FILE *fptr;
int currentImage, bytes;
int pal_entr;
RGB *pal;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.2";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"SUN Raster";
}
	
const char* fmt_filter()
{
    return (const char*)"*.ras ";
}
	    
const char* fmt_mime()
{
    return (const char*)"\x0059\x00A6\x006A\x0095";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,102,255,204,4,4,4,193,122,37,156,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,73,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,177,139,177,137,177,9,144,1,6,24,34,46,48,6,88,4,166,11,110,14,196,100,37,81,168,165,96,91,193,206,8,0,0,119,14,23,196,139,226,159,35,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

RAS_HEADER rfh;
bool rle, isRGB;
unsigned short fill;
unsigned char fillchar;
unsigned short linelength;
unsigned char *buf;

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;
	
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    pal = 0;
    pal_entr = 0;
    rle = false;
    isRGB = false;

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

    finfo->image[currentImage].passes = 1;

    rfh.ras_magic = BE_getlong(fptr);
    rfh.ras_width = BE_getlong(fptr);
    rfh.ras_height = BE_getlong(fptr);
    rfh.ras_depth = BE_getlong(fptr);
    rfh.ras_length = BE_getlong(fptr);
    rfh.ras_type = BE_getlong(fptr);
    rfh.ras_maptype = BE_getlong(fptr);
    rfh.ras_maplength = BE_getlong(fptr);

    if(rfh.ras_magic != RAS_MAGIC) return SQERR_BADFILE;

    if(rfh.ras_type != RAS_OLD && rfh.ras_type != RAS_STANDARD && rfh.ras_type != RAS_BYTE_ENCODED && rfh.ras_type != RAS_RGB &&
	    rfh.ras_type != RAS_TIFF && rfh.ras_type != RAS_IFF &&  rfh.ras_type != RAS_EXPERIMENTAL)
	return SQERR_BADFILE;
    else if(rfh.ras_type == RAS_EXPERIMENTAL)
	return SQERR_NOTSUPPORTED;

    finfo->image[currentImage].w = rfh.ras_width;
    finfo->image[currentImage].h = rfh.ras_height;
    finfo->image[currentImage].bpp = rfh.ras_depth;

    switch(rfh.ras_maptype)
    {
	case RMT_NONE :
	{
		if (rfh.ras_depth < 24)
		{
		    pal = (RGB*)calloc(256, sizeof(RGB));

		    int numcolors = 1 << rfh.ras_depth, i;

		    for (i = 0; i < numcolors; i++)
		    {
			pal[i].r = (255 * i) / (numcolors - 1);
			pal[i].g = (255 * i) / (numcolors - 1);
			pal[i].b = (255 * i) / (numcolors - 1);
		    }
		}

	break;
	}

	case RMT_EQUAL_RGB:
	{
		char *r, *g, *b;

		int numcolors = 1 << rfh.ras_depth;

		r = (char*)malloc(3 * numcolors * 1);
		g = r + numcolors;
		b = g + numcolors;

		pal = (RGB*)calloc(256, sizeof(RGB));

		fread(r, 3 * numcolors, 1, fptr);

		for(int i = 0; i < numcolors; i++)
		{
			pal[i].r = r[i];
			pal[i].g = g[i];
			pal[i].b = b[i];
		}

		free(r);
	break;
	}

	case RMT_RAW:
	{
		char *colormap = (char*)malloc(rfh.ras_maplength * 1);

		fread(colormap, rfh.ras_maplength, 1, fptr);

		free(colormap);
	break;
	}
    }

    switch(rfh.ras_type)
    {
	case RAS_OLD:
	case RAS_STANDARD:
	case RAS_TIFF:
	case RAS_IFF:
	break;

	case RAS_BYTE_ENCODED:
	    rle = true;
	break;

	case RAS_RGB:
	    isRGB = true;
	break;
    }
    
    if(rfh.ras_depth == 1)
	linelength = (short)((rfh.ras_width / 8) + (rfh.ras_width % 8 ? 1 : 0));
    else
	linelength = (short)rfh.ras_width;

    fill = (linelength % 2) ? 1 : 0;
    buf = (unsigned char*)calloc(rfh.ras_width, sizeof(RGB));
							
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->images++;
	
    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n%s\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGB",
	(isRGB) ? "-":"RLE",
	bytes);
	
//    printf("ras_depath: %d\n", rfh.ras_depth);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}
    
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    unsigned int i;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
    	case 1:
	break;

	case 8:
		fmt_readdata(fptr, buf, linelength, rle);

		for(i = 0;i < rfh.ras_width;i++)
		    memcpy(scan+i, &pal[i], sizeof(RGB));

		if (fill)
		    fmt_readdata(fptr, &fillchar, fill, rle);
    	break;

	case 24:
	{
	    unsigned char *b = buf;
	    
	    fmt_readdata(fptr, buf, rfh.ras_width * 3, rle);

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].r = *b;
    	    	    scan[i].g = *(b+1);
		    scan[i].b = *(b+2);
		    b += 3;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 2);
		    scan[i].g = *(b + 1);
		    scan[i].b = *b;
		    b += 3;
		}
		
	    if(fill)
	        fmt_readdata(fptr, &fillchar, fill, rle);
	}	
	break;

	case 32:
	{
	    unsigned char *b = buf;
	    
	    fmt_readdata(fptr, buf, rfh.ras_width * 4, rle);

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].a = *b;
	    	    scan[i].r = *(b+1);
    	    	    scan[i].g = *(b+2);
		    scan[i].b = *(b+3);
		    b += 4;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 3);
		    scan[i].g = *(b + 2);
		    scan[i].b = *(b + 1);
		    scan[i].a = *b;
		    b += 4;
		}
		
	    if(fill)
	        fmt_readdata(fptr, &fillchar, fill, rle);

	}
	break;

    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    RAS_HEADER m_rfh;
    bool m_rle = false, m_isRGB = false;
    unsigned short m_fill;
    unsigned char m_fillchar;
    unsigned short m_linelength;
    unsigned char *m_buf;
    RGB *m_pal = 0;

    FILE *m_fptr;
    int w, h, bpp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    m_rfh.ras_magic = BE_getlong(m_fptr);
    m_rfh.ras_width = BE_getlong(m_fptr);
    m_rfh.ras_height = BE_getlong(m_fptr);
    m_rfh.ras_depth = BE_getlong(m_fptr);
    m_rfh.ras_length = BE_getlong(m_fptr);
    m_rfh.ras_type = BE_getlong(m_fptr);
    m_rfh.ras_maptype = BE_getlong(m_fptr);
    m_rfh.ras_maplength = BE_getlong(m_fptr);

    if(m_rfh.ras_magic != RAS_MAGIC) return SQERR_BADFILE;

    if(m_rfh.ras_type != RAS_OLD && m_rfh.ras_type != RAS_STANDARD && m_rfh.ras_type != RAS_BYTE_ENCODED && m_rfh.ras_type != RAS_RGB &&
	m_rfh.ras_type != RAS_TIFF && m_rfh.ras_type != RAS_IFF &&  m_rfh.ras_type != RAS_EXPERIMENTAL)
	return SQERR_BADFILE;
    else if(m_rfh.ras_type == RAS_EXPERIMENTAL)
	return SQERR_NOTSUPPORTED;

    w = m_rfh.ras_width;
    h = m_rfh.ras_height;
    bpp = m_rfh.ras_depth;

    switch(m_rfh.ras_maptype)
    {
	case RMT_NONE :
	{
		if (m_rfh.ras_depth < 24)
		{
		    m_pal = (RGB*)calloc(256, sizeof(RGB));

		    int numcolors = 1 << m_rfh.ras_depth, i;

		    for (i = 0; i < numcolors; i++)
		    {
			m_pal[i].r = (255 * i) / (numcolors - 1);
			m_pal[i].g = (255 * i) / (numcolors - 1);
			m_pal[i].b = (255 * i) / (numcolors - 1);
		    }
		}

	break;
	}

	case RMT_EQUAL_RGB:
	{
		char *r, *g, *b;

		int numcolors = 1 << m_rfh.ras_depth;

		r = (char*)malloc(3 * numcolors * 1);
		g = r + numcolors;
		b = g + numcolors;

		m_pal = (RGB*)calloc(256, sizeof(RGB));

		fread(r, 3 * numcolors, 1, m_fptr);

		for(int i = 0; i < numcolors; i++)
		{
			m_pal[i].r = r[i];
			m_pal[i].g = g[i];
			m_pal[i].b = b[i];
		}

		free(r);
	break;
	}

	case RMT_RAW:
	{
		char *colormap = (char*)malloc(m_rfh.ras_maplength * 1);

		fread(colormap, m_rfh.ras_maplength, 1, m_fptr);

		free(colormap);
	break;
	}
    }

    switch(m_rfh.ras_type)
    {
	case RAS_OLD:
	case RAS_STANDARD:
	case RAS_TIFF:
	case RAS_IFF:
	break;

	case RAS_BYTE_ENCODED:
	    m_rle = true;
	break;

	case RAS_RGB:
	    m_isRGB = true;
	break;
    }
    
    if(m_rfh.ras_depth == 1)
	m_linelength = (short)((m_rfh.ras_width / 8) + (m_rfh.ras_width % 8 ? 1 : 0));
    else
	m_linelength = (short)m_rfh.ras_width;
							
    m_fill = (m_linelength % 2) ? 1 : 0;
    m_buf = (unsigned char*)malloc(m_rfh.ras_width * 3);
    memset(m_buf, 0, sizeof(m_buf));

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n%s\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	(m_isRGB) ? "-":"RLE",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_ras: Image is null!\n");
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    int W = w * sizeof(RGBA);
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	unsigned int i;

	memset(scan, 255, W);

    switch(bpp)
    {
    	case 1:
	break;

	case 8:
		fmt_readdata(m_fptr, m_buf, m_linelength, m_rle);
		
		for(i = 0;i < m_rfh.ras_width;i++)
		    memcpy(scan+i, &m_pal[i], sizeof(RGB));
			
		if (m_fill)
		    fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle);
    	break;

	case 24:
	{
	    unsigned char *b = m_buf;
	    
	    fmt_readdata(m_fptr, m_buf, m_rfh.ras_width * 3, m_rle);

	    if(m_isRGB)
	        for (i = 0; i < m_rfh.ras_width; i++)
	        {
	    	    scan[i].r = *b;
    	    	    scan[i].g = *(b+1);
		    scan[i].b = *(b+2);
		    b += 3;
		}
	    else
	        for (i = 0; i < m_rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 2);
		    scan[i].g = *(b + 1);
		    scan[i].b = *b;
		    b += 3;
		}
		
	    if(m_fill)
	        fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle);
	}	
	break;

	case 32:
	{
	    unsigned char *b = m_buf;
	    
	    fmt_readdata(m_fptr, m_buf, m_rfh.ras_width * 4, m_rle);

	    if(m_isRGB)
	        for (i = 0; i < m_rfh.ras_width; i++)
	        {
	    	    scan[i].a = *b;
	    	    scan[i].r = *(b+1);
    	    	    scan[i].g = *(b+2);
		    scan[i].b = *(b+3);
		    b += 4;
		}
	    else
	        for (i = 0; i < m_rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 3);
		    scan[i].g = *(b + 2);
		    scan[i].b = *(b + 1);
		    scan[i].a = *b;
		    b += 4;
		}

	    if(m_fill)
	        fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle);

	}
	break;

    }

    }

    fclose(m_fptr);
    free(m_buf);

    return SQERR_OK;
}

int fmt_close()
{
    fclose(fptr);
    free(buf);
    return SQERR_OK;
}

void fmt_readdata(FILE *handle, unsigned char *buf, unsigned long length, bool rle)
{
    unsigned char repchar, remaining = 0;

    if(rle)
    {
	while(length--)
	{
		if (remaining)
		{
    		    remaining--;
		    *(buf++)= repchar;
		}
		else
		{
			fread(&repchar, 1, 1, handle);

			if(repchar == RESC)
			{
				fread(&remaining, 1, 1, handle);

				if (remaining == 0)
					*(buf++)= RESC;
				else
				{
					fread(&repchar, 1, 1, handle);
					*(buf++)= repchar;
				}
			}
			else
				*(buf++)= repchar;
		}
	}
    }
    else
	fread(buf, length, 1, handle);
}

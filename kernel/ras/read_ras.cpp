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
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "read_ras.h"
#include "endian.h"
#include "utils.h"

FILE 	*fptr;
int 	currentImage, bytes;
RGB 	pal[256];

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.3";
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

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    rle = false;
    isRGB = false;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;
	    
    if(!finfo)
        return SQERR_NOMEMORY;

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

    if(sq_ferror(fptr)) return SQERR_BADFILE;

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
		char *g, *b;

		int numcolors = 1 << rfh.ras_depth;

		char r[3 * numcolors];

		g = r + numcolors;
		b = g + numcolors;

		if(!sq_fread(r, 3 * numcolors, 1, fptr)) return SQERR_BADFILE;

		for(int i = 0; i < numcolors; i++)
		{
			pal[i].r = r[i];
			pal[i].g = g[i];
			pal[i].b = b[i];
		}
	break;
	}

	case RMT_RAW:
	{
		char colormap[rfh.ras_maplength];
		if(!sq_fread(colormap, rfh.ras_maplength, 1, fptr)) return SQERR_BADFILE;
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

    if(!buf)
	return SQERR_NOMEMORY;

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->images++;
	
    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n%s\n%d\n",
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
		if(!fmt_readdata(fptr, buf, linelength, rle))
		    return SQERR_BADFILE;

		for(i = 0;i < rfh.ras_width;i++)
		    memcpy(scan+i, &pal[i], sizeof(RGB));

		if(fill)
		{
		    if(!fmt_readdata(fptr, &fillchar, fill, rle))
			return SQERR_BADFILE;
		}
    	break;

	case 24:
	{
	    unsigned char *b = buf;
	    
	    if(!fmt_readdata(fptr, buf, rfh.ras_width * 3, rle))
		return SQERR_BADFILE;

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
	    {
	        if(!fmt_readdata(fptr, &fillchar, fill, rle))
		    return SQERR_BADFILE;
	    }
	}	
	break;

	case 32:
	{
	    unsigned char *b = buf;
	    
	    if(!fmt_readdata(fptr, buf, rfh.ras_width * 4, rle))
		return SQERR_BADFILE;

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
	    {
	        if(!fmt_readdata(fptr, &fillchar, fill, rle))
		    return SQERR_BADFILE;
	    }

	}
	break;
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    jmp_buf		jmp;
    FILE 		*m_fptr;
    int 		w, h, bpp, m_bytes;
    RAS_HEADER 		m_rfh;
    bool 		m_rle, m_isRGB;
    unsigned short 	m_fill;
    unsigned char 	m_fillchar;
    unsigned short 	m_linelen;
    RGB			m_pal[256];

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	fclose(m_fptr);
	
	return SQERR_BADFILE;
    }

    m_rfh.ras_magic = BE_getlong(m_fptr);
    m_rfh.ras_width = BE_getlong(m_fptr);
    m_rfh.ras_height = BE_getlong(m_fptr);
    m_rfh.ras_depth = BE_getlong(m_fptr);
    m_rfh.ras_length = BE_getlong(m_fptr);
    m_rfh.ras_type = BE_getlong(m_fptr);
    m_rfh.ras_maptype = BE_getlong(m_fptr);
    m_rfh.ras_maplength = BE_getlong(m_fptr);
    
    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

    if(m_rfh.ras_magic != RAS_MAGIC) longjmp(jmp, 1);

    if(m_rfh.ras_type != RAS_OLD && m_rfh.ras_type != RAS_STANDARD && m_rfh.ras_type != RAS_BYTE_ENCODED && m_rfh.ras_type != RAS_RGB &&
	    m_rfh.ras_type != RAS_TIFF && m_rfh.ras_type != RAS_IFF &&  m_rfh.ras_type != RAS_EXPERIMENTAL)
	longjmp(jmp, 1);
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
		char *g, *b;

		int numcolors = 1 << m_rfh.ras_depth;

		char r[3 * numcolors];

		g = r + numcolors;
		b = g + numcolors;

		if(!sq_fread(r, 3 * numcolors, 1, m_fptr)) longjmp(jmp, 1);

		for(int i = 0; i < numcolors; i++)
		{
			m_pal[i].r = r[i];
			m_pal[i].g = g[i];
			m_pal[i].b = b[i];
		}
	break;
	}

	case RMT_RAW:
	{
		char colormap[m_rfh.ras_maplength];
		if(!sq_fread(colormap, m_rfh.ras_maplength, 1, m_fptr)) longjmp(jmp, 1);
	break;
	}
    }

    m_rle = false;
    m_isRGB = false;

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
	m_linelen = (short)((m_rfh.ras_width / 8) + (m_rfh.ras_width % 8 ? 1 : 0));
    else
	m_linelen = (short)m_rfh.ras_width;

    m_fill = (m_linelen % 2) ? 1 : 0;

    unsigned char m_buf[m_rfh.ras_width * sizeof(RGB)];

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n%s\n%d\n%d\n",
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
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;
	
        unsigned int i;

	switch(bpp)
	{
    	    case 1:
	    break;

	    case 8:
		if(!fmt_readdata(m_fptr, m_buf, m_linelen, m_rle))
		    longjmp(jmp, 1);

		for(i = 0;i < m_rfh.ras_width;i++)
		    memcpy(scan+i, &m_pal[i], sizeof(RGB));

		if(m_fill)
		{
		    if(!fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
    	    break;

	    case 24:
	    {
		unsigned char *b = m_buf;
	    
		if(!fmt_readdata(m_fptr, m_buf, m_rfh.ras_width * 3, m_rle))
		    longjmp(jmp, 1);

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
		{
	    	    if(!fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
	    }	
	    break;

	    case 32:
	    {
		unsigned char *b = m_buf;
	    
		if(!fmt_readdata(m_fptr, m_buf, m_rfh.ras_width * 4, m_rle))
		    longjmp(jmp, 1);

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
		{
	    	    if(!fmt_readdata(m_fptr, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
	    }
	    break;
	}
    }

    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);

    if(buf)
	free(buf);
}

bool fmt_readdata(FILE *handle, unsigned char *_buf, unsigned long length, bool rle)
{
    unsigned char repchar, remaining = 0;

    if(rle)
    {
	while(length--)
	{
		if (remaining)
		{
    		    remaining--;
		    *(_buf++)= repchar;
		}
		else
		{
			if(!sq_fread(&repchar, 1, 1, handle)) return false;

			if(repchar == RESC)
			{
				if(!sq_fread(&remaining, 1, 1, handle)) return false;

				if (remaining == 0)
					*(_buf++) = RESC;
				else
				{
					if(!sq_fread(&repchar, 1, 1, handle)) return false;
					*(_buf++) = repchar;
				}
			}
			else
				*(_buf++) = repchar;
		}
	}
    }
    else
    {
	if(!sq_fread(_buf, length, 1, handle)) return false;
	
    }
    
    return true;
}

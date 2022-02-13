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

#include "read_pix.h"
#include "utils.h"

FILE *fptr;
int currentImage, bytes;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"1.0.0";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"Irix PIX image";
}
	
const char* fmt_filter()
{
    return (const char*)"*.pix ";
}
	    
const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,204,51,4,4,4,195,151,166,176,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,76,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,144,97,236,98,98,98,2,98,152,152,64,25,64,17,103,176,148,9,92,10,198,128,234,130,155,3,49,89,73,20,106,41,216,86,176,51,2,0,148,190,24,56,148,160,248,187,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    unsigned short tmp;
    PIX_HEADER	pfh;

    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;
	    
    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;

    pfh.width = BE_getshort(fptr);
    pfh.height = BE_getshort(fptr);

    if(!sq_fread(&tmp, sizeof(unsigned short), 1, fptr)) return SQERR_BADFILE;
    if(!sq_fread(&tmp, sizeof(unsigned short), 1, fptr)) return SQERR_BADFILE;

    pfh.bpp = BE_getshort(fptr);

    if(pfh.bpp != 24)	
	return SQERR_BADFILE;

    finfo->image[currentImage].w = pfh.width;
    finfo->image[currentImage].h = pfh.height;
    finfo->image[currentImage].bpp = pfh.bpp;

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    finfo->images++;

    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\nRLE\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGB",
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int	len = 0, i, counter = 0;
    unsigned char count;
    RGB	rgb;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
	case 24:
	    do
	    {
		if(!sq_fgetc(fptr, &count)) return SQERR_BADFILE;
		len += count;
		
		if(!sq_fread(&rgb.b, 1, 1, fptr)) return SQERR_BADFILE;
		if(!sq_fread(&rgb.g, 1, 1, fptr)) return SQERR_BADFILE;
		if(!sq_fread(&rgb.r, 1, 1, fptr)) return SQERR_BADFILE;

		for(i = 0;i < count;i++)
		    memcpy(scan+counter++, &rgb, 3);
		    
	    }while(len < finfo->image[currentImage].w);
	break;

    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    PIX_HEADER		pfh;
    FILE 		*m_fptr;
    int 		w, h, bpp;
    unsigned short 	tmp;
    int 		m_bytes;
    jmp_buf		jmp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;
	
    if(setjmp(jmp))
    {
	fclose(m_fptr);
	return SQERR_BADFILE;
    }

    pfh.width = BE_getshort(m_fptr);
    pfh.height = BE_getshort(m_fptr);

    if(!sq_fread(&tmp, sizeof(unsigned short), 1, m_fptr)) longjmp(jmp, 1);
    if(!sq_fread(&tmp, sizeof(unsigned short), 1, m_fptr)) longjmp(jmp, 1);

    pfh.bpp = BE_getshort(m_fptr);

    if(pfh.bpp != 24)	
	longjmp(jmp, 1);

    w = pfh.width;
    h = pfh.height;
    bpp = pfh.bpp;

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\nRLE\n%d\n%d\n",
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
        fprintf(stderr, "libSQ_read_pix: Image is null!\n");
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    for(int h2 = 0;h2 < h;h2++)
    {
	int len = 0, i, counter = 0;
        unsigned char	count;
	RGB		rgb;
        RGBA 	*scan = *image + h2 * w;

        switch(bpp)
	{
	    case 24:
		do
    		{
		    if(!sq_fgetc(m_fptr, &count)) longjmp(jmp, 1);
		    len += count;

		    if(!sq_fread(&rgb.b, 1, 1, m_fptr)) longjmp(jmp, 1);
		    if(!sq_fread(&rgb.g, 1, 1, m_fptr)) longjmp(jmp, 1);
		    if(!sq_fread(&rgb.r, 1, 1, m_fptr)) longjmp(jmp, 1);

		    for(i = 0;i < count;i++)
			memcpy(scan+counter++, &rgb, sizeof(RGB));

		}while(len < w);
	    break;
	}
    }

    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
}

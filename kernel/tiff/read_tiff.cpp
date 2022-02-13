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

#include "read_tiff.h"

#include <tiffio.h>

TIFF	*ftiff;
TIFFRGBAImage img;
int currentImage, bytes;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.9.1";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"Tagged Image File Format";
}
	
const char* fmt_filter()
{
    return (const char*)"*.tiff *tif ";
}
	    
const char* fmt_mime()
{
    return (const char*)"II|MM";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,0,153,4,4,4,41,96,241,199,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,79,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,10,16,42,1,25,78,46,78,46,42,46,80,134,146,10,170,136,146,10,152,1,211,5,55,7,98,178,146,40,212,82,176,173,96,103,4,0,0,107,22,23,177,172,1,179,111,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;
	
    currentImage = -1;

    if((ftiff = TIFFOpen(file, "r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

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

//    int bps, spp;

    TIFFGetField(ftiff, TIFFTAG_IMAGEWIDTH, &finfo->image[currentImage].w);
    TIFFGetField(ftiff, TIFFTAG_IMAGELENGTH, &finfo->image[currentImage].h);
//    TIFFGetField(ftiff, TIFFTAG_BITSPERSAMPLE, &bps);
//    TIFFGetField(ftiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
    
//    printf("bps: %d, spp: %d\n", bps, spp);

    finfo->image[currentImage].bpp = 32;//bps * spp;

    memset(&img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&img, ftiff, 1, 0);

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->image[currentImage].hasalpha = true;

    finfo->images++;

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGBA",
	bytes);
					
    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}
    
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    const int W = finfo->image[currentImage].w * sizeof(RGBA);

    uint32 buf[W];
    
    memset(scan, 255, W);

    TIFFRGBAImageGet(&img, buf, finfo->image[currentImage].w, 1);

    memcpy(scan, buf, W);

    img.row_offset++;

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    TIFF	*m_ftiff;
    TIFFRGBAImage m_img;

    int w, h, bpp;

    if((m_ftiff = TIFFOpen(file, "r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

//    int bps, spp;

    TIFFGetField(m_ftiff, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(m_ftiff, TIFFTAG_IMAGELENGTH, &h);
//    TIFFGetField(m_ftiff, TIFFTAG_BITSPERSAMPLE, &bps);
//    TIFFGetField(m_ftiff, TIFFTAG_SAMPLESPERPIXEL, &spp);

    bpp = 32;//bps * spp;

    memset(&m_img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&m_img, m_ftiff, 1, 0);

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGBA",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_tiff: Image is null!\n");
	TIFFRGBAImageEnd(&m_img);
	TIFFClose(m_ftiff);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    const int W = w * sizeof(RGBA);
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	uint32 buf[W];

	memset(scan, 255, W);
	TIFFRGBAImageGet(&m_img, buf, w, 1);

	memcpy(scan, buf, W);

	m_img.row_offset++;
    }

    TIFFRGBAImageEnd(&m_img);
    TIFFClose(m_ftiff);

    return SQERR_OK;
}

int fmt_close()
{
    TIFFRGBAImageEnd(&img);
    TIFFClose(ftiff);

    return SQERR_OK;
}

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

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "read_xcur.h"
#include "utils.h"

FILE *fptr;
int bytes, currentImage;
int currentToc;
bool lastToc;

XCUR_HEADER xcur_h;
XCUR_CHUNK_DESC *tocs;
XCUR_CHUNK_HEADER xcur_chunk;
XCUR_CHUNK_IMAGE xcur_im;

const char* fmt_version()
{
    return (const char*)"0.3.0";
}

const char* fmt_quickinfo()
{
    return (const char*)"X Cursors";
}

const char* fmt_filter()
{
    return (const char*)0;
}

const char* fmt_mime()
{
    return (const char*)"Xcur";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,0,0,0,192,192,192,255,255,255,255,255,0,4,4,4,37,60,155,71,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,99,96,96,8,5,1,6,32,16,82,82,82,82,5,49,130,140,141,141,85,67,161,12,160,144,32,20,48,136,184,128,129,35,131,160,35,58,67,68,196,81,208,209,209,81,16,40,226,232,2,100,128,164,28,5,93,128,4,146,46,152,57,16,147,141,65,150,5,41,65,109,5,59,35,0,0,30,33,22,117,242,237,176,248,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");

    if(!fptr)
        return SQERR_NOFILE;

    currentImage = -1;
    currentToc = -1;
    
    if(!sq_fread(&xcur_h, sizeof(XCUR_HEADER), 1, fptr)) return SQERR_BADFILE;

    tocs = (XCUR_CHUNK_DESC*)calloc(xcur_h.ntoc, sizeof(XCUR_CHUNK_DESC));

    if(!sq_fread(tocs, sizeof(XCUR_CHUNK_DESC), xcur_h.ntoc, fptr)) return SQERR_BADFILE;
    
    lastToc = false;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;

    if(!finfo)
	return SQERR_NOMEMORY;
    
    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
	return SQERR_NOMEMORY;

    if(lastToc)
    {
	finfo->animated = (currentToc > 0);
	return SQERR_NOTOK;
    }

    do
    {
	 currentToc++;
    }
    while(tocs[currentToc].type != XCUR_CHUNK_TYPE_IMAGE && currentToc < (int)xcur_h.ntoc);

    if(currentToc == (int)xcur_h.ntoc-1)
	lastToc = true;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;    

    /* Move to the next image here */
    
    fseek(fptr, tocs[currentToc].pos, SEEK_SET);
    
    if(!sq_fread(&xcur_chunk, sizeof(XCUR_CHUNK_HEADER), 1, fptr)) return SQERR_BADFILE;
    if(!sq_fread(&xcur_im, sizeof(XCUR_CHUNK_IMAGE), 1, fptr)) return SQERR_BADFILE;
    
    finfo->image[currentImage].w = xcur_im.width;
    finfo->image[currentImage].h = xcur_im.height;
    finfo->image[currentImage].bpp = 32;
    finfo->image[currentImage].delay = xcur_im.delay;
    finfo->image[currentImage].hasalpha = true;

    /* Compute total bytes needed for the image */
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    /* Determine image type (RGB, RGBA, YUV, etc.) */
    char type[25];
    strcpy(type, "ARGB"); /* Usually image is in RGB format */

    finfo->images++;

    /* Write dump */
    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	type,
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *finfo)
{
    if(!finfo)
	return SQERR_NOTOK;

    if(!finfo->image)
	return SQERR_NOTOK;

    finfo = finfo;

    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    RGBA rgba;

    for(int i = 0;i < finfo->image[currentImage].w;i++)
    {
	if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

	(scan+i)->r = rgba.b;
	(scan+i)->g = rgba.g;
	(scan+i)->b = rgba.r;
	(scan+i)->a = rgba.a;
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    FILE 		*m_fptr;
    int 		w, h, bpp;
    XCUR_HEADER 	m_xcur_h;
    XCUR_CHUNK_HEADER 	m_xcur_chunk;
    XCUR_CHUNK_IMAGE 	m_xcur_im;
    int 		m_currentToc;
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

    m_currentToc = -1;
    
    if(!sq_fread(&m_xcur_h, sizeof(XCUR_HEADER), 1, m_fptr)) longjmp(jmp, 1);

    XCUR_CHUNK_DESC m_tocs[m_xcur_h.ntoc];

    if(!sq_fread(m_tocs, sizeof(XCUR_CHUNK_DESC), m_xcur_h.ntoc, m_fptr)) longjmp(jmp, 1);

    do
    {
	 m_currentToc++;
    }
    while(m_tocs[m_currentToc].type != XCUR_CHUNK_TYPE_IMAGE && m_currentToc < (int)m_xcur_h.ntoc);

    fseek(m_fptr, m_tocs[m_currentToc].pos, SEEK_SET);
    
    if(!sq_fread(&m_xcur_chunk, sizeof(XCUR_CHUNK_HEADER), 1, m_fptr)) longjmp(jmp, 1);
    if(!sq_fread(&m_xcur_im, sizeof(XCUR_CHUNK_IMAGE), 1, m_fptr)) longjmp(jmp, 1);
    
    w = m_xcur_im.width;
    h = m_xcur_im.height;
    bpp = 32;

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"ARGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
        fprintf(stderr, "libSQ_read_xcur: Image is null!\n");
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	RGBA rgba;

	for(int i = 0;i < w;i++)
	{
	    if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

	    (scan+i)->r = rgba.b;
	    (scan+i)->g = rgba.g;
	    (scan+i)->b = rgba.r;
	    (scan+i)->a = rgba.a;
	}
    }
    
    fclose(m_fptr);
    
    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);

    if(tocs)
	free(tocs);
}

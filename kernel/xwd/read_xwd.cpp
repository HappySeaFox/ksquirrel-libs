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

#include "read_xwd.h"
#include "endian.h"

FILE *fptr;
int currentImage, bytes, pal_entr, filler;
RGB *pal;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.3.3";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"X Window Dump";
}
	
const char* fmt_filter()
{
    return (const char*)"*.xwd ";
}
	    
const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,137,12,83,4,4,4,204,223,87,180,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,1,98,75,71,68,0,136,5,29,72,0,0,0,9,112,72,89,115,0,0,11,17,0,0,11,17,1,127,100,95,145,0,0,0,7,116,73,77,69,7,213,1,30,19,36,17,134,185,242,93,0,0,0,81,73,68,65,84,120,156,69,142,65,17,192,48,8,4,207,2,15,12,80,20,164,81,64,17,208,62,226,223,74,40,97,146,125,237,48,112,7,128,241,131,224,35,34,78,17,17,30,37,49,186,11,116,79,30,52,51,55,147,148,192,47,116,45,105,230,170,71,98,103,95,237,156,149,44,92,165,217,154,111,188,19,140,238,23,253,181,135,193,82,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;

    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;

    currentImage = -1;
    pal = 0;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    XWDFileHeader xfh;

    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    XWDColor	color;
    char 	str[256];
    int		i, ncolors;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;

    fread(&xfh, sizeof(XWDFileHeader), 1, fptr);

    fgets(str, 255, fptr);

    fseek(fptr, lLE2BE(xfh.header_size), SEEK_SET);

    pal_entr = ncolors = lLE2BE(xfh.ncolors);

    if((pal = (RGB*)calloc(ncolors, sizeof(RGB))) == NULL)
    {
//	fclose(fptr);	
	return SQERR_NOMEMORY;
    }

    for(i = 0;i < ncolors;i++)
    {
	fread(&color, sizeof(XWDColor), 1, fptr);

	pal[i].r = (uchar)sLE2BE(color.red);
	pal[i].g = (uchar)sLE2BE(color.green);
	pal[i].b = (uchar)sLE2BE(color.blue);
    }

    finfo->image[currentImage].w = lLE2BE(xfh.pixmap_width);
    finfo->image[currentImage].h = lLE2BE(xfh.pixmap_height);
    finfo->image[currentImage].bpp = lLE2BE(xfh.bits_per_pixel);//lLE2BE(xfh.pixmap_depth);

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
            
    finfo->images++;

    finfo->image[currentImage].meta = (fmt_metainfo *)calloc(1, sizeof(fmt_metainfo));

    if(finfo->image[currentImage].meta)
    {
	finfo->image[currentImage].meta->entries++;
	finfo->image[currentImage].meta->m = (fmt_meta_entry *)calloc(1, sizeof(fmt_meta_entry));
	fmt_meta_entry *entry = finfo->image[currentImage].meta->m;

	if(entry)
	{
	    entry[currentImage].datalen = strlen(str) + 1;
	    strcpy(entry[currentImage].group, "XWD Window Name");
	    entry[currentImage].data = (char *)malloc(entry[currentImage].datalen);

	    if(entry->data)
		strcpy(entry[currentImage].data, str);
	}
    }

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGB",
	bytes);

    filler = lLE2BE(xfh.bytes_per_line) - finfo->image[currentImage].w * finfo->image[currentImage].bpp / 8;
    
    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int 	i;
    RGBA	rgba;
    RGB		rgb;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
        case 24:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
    	    {
    		fread(&rgb, sizeof(RGB), 1, fptr);
		memcpy(scan+i, &rgb, sizeof(RGB));
	    }
	    
	    for(int s = 0;s < filler;s++) fgetc(fptr);
	break;

        case 32:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
    	    {
    		fread(&rgba, sizeof(RGBA), 1, fptr);

		scan[i].r = rgba.b;
		scan[i].g = rgba.g;
		scan[i].b = rgba.r;
	    }
	    
	    for(int s = 0;s < filler;s++) fgetc(fptr);
	break;
    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    XWDFileHeader m_xfh;

    FILE *m_fptr;
    int w, h, bpp, m_pal_entr, m_ncolors;
    RGB *m_pal = 0;
	    
    m_fptr = fopen(file, "rb");
		
    if(!m_fptr)
        return SQERR_NOFILE;

    XWDColor	color;
    char 	str[256];
    int		i;

    fread(&m_xfh, sizeof(XWDFileHeader), 1, m_fptr);

    fgets(str, 255, m_fptr);

    fseek(m_fptr, lLE2BE(m_xfh.header_size), SEEK_SET);

    m_pal_entr = m_ncolors = lLE2BE(m_xfh.ncolors);

    if((m_pal = (RGB*)calloc(m_ncolors, sizeof(RGB))) == NULL)
    {
	fclose(m_fptr);	
	return SQERR_NOMEMORY;
    }
    
    for(i = 0;i < m_ncolors;i++)
    {
	fread(&color, sizeof(XWDColor), 1, m_fptr);

	m_pal[i].r = (uchar)sLE2BE(color.red);
	m_pal[i].g = (uchar)sLE2BE(color.green);
	m_pal[i].b = (uchar)sLE2BE(color.blue);
    }

    w = lLE2BE(m_xfh.pixmap_width);
    h = lLE2BE(m_xfh.pixmap_height);
    bpp = lLE2BE(m_xfh.bits_per_pixel);

    int m_bytes = w * h * sizeof(RGBA);
    
    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	1,
	m_bytes);

    int m_filler = lLE2BE(m_xfh.bytes_per_line) - w * bpp / 8;
				    
    *image = (RGBA*)realloc(*image, m_bytes);
					
    if(!*image)
    {
        fprintf(stderr, "libSQ_read_pix: Image is null!\n");
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA	rgba;
	RGB	rgb;
        RGBA 	*scan = *image + h2 * w;

        memset(scan, 255, w * sizeof(RGBA));
/*
	for(int s = 0;s < w;s++)
	{
	    fread(&rgba, sizeof(RGBA), 1, fptr);

	    scan[s].r = rgba.b;
	    scan[s].g = rgba.g;
	    scan[s].b = rgba.r;
	}*/
        switch(bpp)
	{
    	    case 24:
		for(int s = 0;s < w;s++)
    		{
    		    fread(&rgb, sizeof(RGB), 1, m_fptr);
		    memcpy(scan+s, &rgb, sizeof(RGB));
		}

		for(int s = 0;s < m_filler;s++) fgetc(m_fptr);
	    break;

    	    case 32:
		for(int s = 0;s < w;s++)
    		{
    		    fread(&rgba, sizeof(RGBA), 1, m_fptr);

		    scan[s].r = rgba.b;
		    scan[s].g = rgba.g;
		    scan[s].b = rgba.r;
		}

		for(int s = 0;s < m_filler;s++) fgetc(m_fptr);
	    break;
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

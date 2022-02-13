/*  This file is part of ksquirrel (http://ksquirrel.sf.net)

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

#include "read_ico.h"
#include "utils.h"

typedef unsigned char uchar;

uchar			*bAND;
ICO_HEADER		ifh;
ICO_DIRENTRY		ide;
BITMAPINFO_HEADER	bih;
int			pixel;

FILE *fptr;
int currentImage, bytes;
RGB *pal;
int pal_entr;

const char* fmt_version()
{
    return (const char*)"0.3.0";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"Windows icons";
}
	
const char* fmt_filter()
{
    return (const char*)"*.ico *.cur ";
}
	    
const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,0,0,128,0,255,0,4,4,4,78,45,246,135,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,74,73,68,65,84,120,218,99,96,96,72,3,1,6,32,72,20,20,20,20,3,51,148,148,148,196,210,160,12,160,144,49,20,48,152,184,128,129,51,131,73,136,107,168,75,104,8,136,17,226,226,234,226,138,157,1,83,3,211,5,55,7,98,178,146,24,212,82,176,173,96,103,36,0,0,118,239,27,119,25,223,110,163,0,0,0,0,73,69,78,68,174,66,96,130,130";
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

    memset(&ifh, 0, sizeof(ICO_HEADER));
    
    pal = 0;
    pal_entr = 0;

    fread(&ifh, sizeof(ICO_HEADER), 1, fptr);

    if(ifh.idType != 1 && ifh.idType != 2)
	return SQERR_BADFILE;
	
//    printf("ICO: count = %d\n", ifh.idCount);

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;

    if(currentImage == ifh.idCount)
	return SQERR_NOTOK;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    RGBA	rgba;
    int		i, pos;

    fread(&ide, sizeof(ICO_DIRENTRY), 1, fptr);
    
//    printf("ressize: %d\n", ide.dwBytes);

    finfo->image[currentImage].w = ide.bWidth;
    finfo->image[currentImage].h = ide.bHeight;
//    finfo->images = ifh.idCount;
    
/*
    if(finfo->image[currentImage].w != 16 && finfo->image[currentImage].w != 32 && finfo->image[currentImage].w != 64)
	return SQERR_BADFILE;

    if(finfo->image[currentImage].h != 16 && finfo->image[currentImage].h != 32 && finfo->image[currentImage].h != 64)
	return SQERR_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQERR_BADFILE;
*/

    fseek(fptr, ide.dwImageOffset, SEEK_SET);
    fread(&bih, sizeof(BITMAPINFO_HEADER), 1, fptr);

    pos = ftell(fptr);

//    fseek(fptr, finfo->image[currentImage].h * bih.Planes * bih.BitCount * finfo->image[currentImage].w / 8, SEEK_CUR);

    int count = finfo->image[currentImage].w * finfo->image[currentImage].h;
    int count2 = count / 8;

    fseek(fptr, ide.dwBytes - sizeof(BITMAPINFO_HEADER) - count2, SEEK_CUR);

    bAND = (uchar *)calloc(count, sizeof(uchar));

    if(!bAND)
        return SQERR_NOMEMORY;
	
    uchar realAND[count2];

    fread(realAND, count2, 1, fptr);
    
    int r = 0;

    for(i = 0;i < count2;i++)
    {
	for(int z = 0,f = 128,u = 7;z < 8;z++,f >>= 1,u--)
	{
	    bAND[r] = (realAND[i] & f) >> u;
	    r++;
	}
    }
/*
    for(int i = 0;i < finfo->image[currentImage].h;i++)
    {
	for(int j = 0;j < finfo->image[currentImage].w;j++)
	{
	    printf("%2d", bAND[i * finfo->image[currentImage].w + j]);
	}
	printf("\n");
    }
*/
    fsetpos(fptr, (fpos_t*)&pos);

    finfo->image[currentImage].bpp = bih.BitCount;
    pal_entr = 1 << finfo->image[currentImage].bpp;

    if(finfo->image[currentImage].bpp < 16)
    {
	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
		return SQERR_NOMEMORY;

	for(i = 0;i < pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, fptr);
		pal[i].r = rgba.b;
		pal[i].g = rgba.g;
		pal[i].b = rgba.r;
	}
    }
    else
	pal = 0;
/*
    for(i = 0;i < pal_entr;i++)
	printf("%d %d %d\n",(pal)[i].r,(pal)[i].g,(pal)[i].b);
*/

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    finfo->image[currentImage].needflip = true;
    finfo->image[currentImage].hasalpha = true;
    finfo->images++;
	
    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGB",
	bytes);
	
    pixel = 0;

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}
    
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int i, j;
    unsigned char bt, ind;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
    	case 1:
	break;

	case 4:
	    j = 0;
	    do
	    {
		bt = fgetc(fptr);
		ind = bt >> 4;
		memcpy(scan+j, pal+ind, sizeof(RGB));
		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
		ind = bt&15;
		memcpy(scan+j, pal+ind, sizeof(RGB));
		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
	    }while(j < finfo->image[currentImage].w);

	break;

	case 8:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		bt = fgetc(fptr);
		memcpy(scan+i, pal+bt, sizeof(RGB));
		(scan+i)->a = (bAND[pixel]) ? 0 : 255;
		pixel++;
	    }
	break;

	case 16:
	break;

	case 24:
	break;

	case 32:
	break;
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    uchar		*m_bAND;
    ICO_HEADER		m_ifh;
    ICO_DIRENTRY	m_ide;
    BITMAPINFO_HEADER	m_bih;
    int			m_pixel = 0;
    RGB 		*m_pal;
    int 		m_pal_entr;

    FILE *m_fptr;
    int w, h, bpp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    m_pal = 0;
    m_pal_entr = 0;

    fread(&m_ifh, sizeof(ICO_HEADER), 1, m_fptr);

    if(m_ifh.idType != 1 && m_ifh.idType != 2)
	return SQERR_BADFILE;

    RGBA	rgba;
    int		i, pos;

    fread(&m_ide, sizeof(ICO_DIRENTRY), 1, m_fptr);
    
//    printf("ressize: %d\n", m_ide.dwBytes);

    w = m_ide.bWidth;
    h = m_ide.bHeight;
//    finfo->images = ifh.idCount;
    
/*
    if(finfo->image[currentImage].w != 16 && finfo->image[currentImage].w != 32 && finfo->image[currentImage].w != 64)
	return SQERR_BADFILE;

    if(finfo->image[currentImage].h != 16 && finfo->image[currentImage].h != 32 && finfo->image[currentImage].h != 64)
	return SQERR_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQERR_BADFILE;
*/

    fseek(m_fptr, m_ide.dwImageOffset, SEEK_SET);
    fread(&m_bih, sizeof(BITMAPINFO_HEADER), 1, m_fptr);

    pos = ftell(m_fptr);

//    fseek(fptr, finfo->image[currentImage].h * bih.Planes * bih.BitCount * finfo->image[currentImage].w / 8, SEEK_CUR);

    int count = w * h;
    int count2 = count / 8;

    fseek(m_fptr, m_ide.dwBytes - sizeof(BITMAPINFO_HEADER) - count2, SEEK_CUR);

    m_bAND = (uchar *)calloc(count, sizeof(uchar));

    if(!m_bAND)
        return SQERR_NOMEMORY;
	
    uchar realAND[count2];

    fread(realAND, count2, 1, m_fptr);

    int r = 0;

    for(i = 0;i < count2;i++)
    {
	for(int z = 0,f = 128,u = 7;z < 8;z++,f >>= 1,u--)
	{
	    m_bAND[r] = (realAND[i] & f) >> u;
	    r++;
	}
    }
/*
    for(int i = 0;i < finfo->image[currentImage].h;i++)
    {
	for(int j = 0;j < finfo->image[currentImage].w;j++)
	{
	    printf("%2d", bAND[i * finfo->image[currentImage].w + j]);
	}
	printf("\n");
    }
*/
    fsetpos(m_fptr, (fpos_t*)&pos);

    bpp = m_bih.BitCount;
    m_pal_entr = 1 << bpp;

    if(bpp < 16)
    {
	if((m_pal = (RGB *)calloc(m_pal_entr, sizeof(RGB))) == 0)
	{
	    if(m_bAND)
		free(m_bAND);
	    fclose(m_fptr);

	    return SQERR_NOMEMORY;
	}

	for(i = 0;i < m_pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, m_fptr);
		m_pal[i].r = rgba.b;
		m_pal[i].g = rgba.g;
		m_pal[i].b = rgba.r;
	}
    }
    else
	m_pal = 0;
/*
    for(i = 0;i < pal_entr;i++)
	printf("%d %d %d\n",(pal)[i].r,(pal)[i].g,(pal)[i].b);
*/

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	m_ifh.idCount,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_ico: Image is null!\n");
        
	fclose(m_fptr);
        
	if(m_bAND)
	    free(m_bAND);

	return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    int W = w * sizeof(RGBA);
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	memset(scan, 255, W);

    int i, j;
    unsigned char bt, ind;

    memset(scan, 255, W);

    switch(bpp)
    {
    	case 1:
	break;

	case 4:
	    j = 0;
	    do
	    {
		bt = fgetc(m_fptr);
		ind = bt >> 4;
		memcpy(scan+j, m_pal+ind, sizeof(RGB));
		(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		j++;
		m_pixel++;
		ind = bt&15;
		memcpy(scan+j, m_pal+ind, sizeof(RGB));
		(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		j++;
		m_pixel++;
	    }while(j < w);
	break;

	case 8:
	    for(i = 0;i < w;i++)
	    {
		bt = fgetc(m_fptr);
		memcpy(scan+i, m_pal+bt, sizeof(RGB));
		(scan+i)->a = (m_bAND[m_pixel]) ? 0 : 255;
		m_pixel++;
	    }
	break;

	case 16:
	break;

	case 24:
	break;

	case 32:
	break;
    }

    }

    fclose(m_fptr);

    if(m_pal)
	free(m_pal);
	
    if(m_bAND)
	free(m_bAND);
	
    flip((char*)*image, w * sizeof(RGBA), h);

    return SQERR_OK;
}

int fmt_close()
{
    fclose(fptr);
    
    if(pal)
	free(pal);
	
    if(bAND)
	free(bAND);

    return SQERR_OK;
}

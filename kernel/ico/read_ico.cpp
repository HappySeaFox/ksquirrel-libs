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
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "read_ico.h"

#define SQ_NEED_FLIP
#include "utils.h"

typedef unsigned char uchar;

uchar			*bAND;
ICO_HEADER		ifh;
ICO_DIRENTRY		*ide;
BITMAPINFO_HEADER	bih;
int			pixel;

FILE *fptr;
int currentImage, bytes;
RGB pal[256];
int pal_entr;

const char* fmt_version()
{
    return (const char*)"0.3.2";
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

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;

    pal_entr = 0;
    bAND = 0;

    if(!sq_fread(&ifh, sizeof(ICO_HEADER), 1, fptr)) return SQERR_BADFILE;

    if(ifh.idType != 1 && ifh.idType != 2)
	return SQERR_BADFILE;
	
//    printf("ICO: count = %d\n", ifh.idCount);

    ide = (ICO_DIRENTRY*)calloc(ifh.idCount, sizeof(ICO_DIRENTRY));

    if(!ide)
	return SQERR_NOMEMORY;

    if(!sq_fread(ide, sizeof(ICO_DIRENTRY), ifh.idCount, fptr)) return SQERR_BADFILE;

//    for(int i = 0;i < ifh.idCount;i++)
//    printf("colorcount: %d, bitcount: %d, bytes: %d\n", ide[i].bColorCount, ide[i].wBitCount, ide[i].dwBytes);

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;

    if(currentImage == ifh.idCount)
	return SQERR_NOTOK;

    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;

    RGBA	rgba;
    int		i;
    long	pos;


//    printf("ressize: %d\n", ide.dwBytes);

    finfo->image[currentImage].w = ide[currentImage].bWidth;
    finfo->image[currentImage].h = ide[currentImage].bHeight;
//    finfo->images = ifh.idCount;
    
/*
    if(finfo->image[currentImage].w != 16 && finfo->image[currentImage].w != 32 && finfo->image[currentImage].w != 64)
	return SQERR_BADFILE;

    if(finfo->image[currentImage].h != 16 && finfo->image[currentImage].h != 32 && finfo->image[currentImage].h != 64)
	return SQERR_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQERR_BADFILE;
*/

    fseek(fptr, ide[currentImage].dwImageOffset, SEEK_SET);
    if(!sq_fread(&bih, sizeof(BITMAPINFO_HEADER), 1, fptr)) return SQERR_BADFILE;

    finfo->image[currentImage].bpp = bih.BitCount;
    printf("bitcount #2: %d\n", bih.BitCount);
//    printf("pal_entr: %d\n", pal_entr);

    if(finfo->image[currentImage].bpp < 16)
    {
	pal_entr = 1 << finfo->image[currentImage].bpp;
	
	for(i = 0;i < pal_entr;i++)
	{
		if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

		pal[i].r = rgba.b;
		pal[i].g = rgba.g;
		pal[i].b = rgba.r;
	}
    }
    else
    {
	pal_entr = 0;
    }

    pos = ftell(fptr);

    printf("Calculating ...\n");
    int count = finfo->image[currentImage].w * finfo->image[currentImage].h;
    printf("count: %d\n", count);
    int count2 = (finfo->image[currentImage].bpp < 16) ? (count / (8 / finfo->image[currentImage].bpp)) : (count * (finfo->image[currentImage].bpp / 8));
    printf("count2: %d\n", count2);
    int count3 = count / 8;
    printf("count3: %d\n", count3);

    fseek(fptr, /*ide[currentImage].dwBytes - sizeof(BITMAPINFO_HEADER) - */count2, SEEK_CUR);

    bAND = (uchar *)realloc(bAND, count * sizeof(uchar));

    if(!bAND)
        return SQERR_NOMEMORY;

    uchar realAND[count3];

    if(!sq_fread(realAND, 1, count3, fptr)) return SQERR_BADFILE;

/*    
    for(i = 0;i < count3;i++)
	printf("REALAND %d\n", realAND[i]);
*/
    int r = 0;

    for(i = 0;i < count3;i++)
    {
	for(int z = 0,f = 128;z < 8;z++,f >>= 1)
	{
	    bAND[r] = (realAND[i] & f) ? 1 : 0;
	//    printf("%d,", bAND[r]);
	    r++;
	}
	//printf("\n");
    }printf("r: %d\n", r);
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

/*
    for(i = 0;i < pal_entr;i++)
	printf("%d %d %d\n",(pal)[i].r,(pal)[i].g,(pal)[i].b);
*/

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    finfo->image[currentImage].needflip = true;
    finfo->image[currentImage].hasalpha = true;
    finfo->images++;
	
    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
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
    int i, j, count;
    unsigned char bt, ind;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
    	case 1:
	    j = finfo->image[currentImage].w / 8;
	    count = 0;

	    for(i = 0;i < j;i++)
	    {
		if(!sq_fread(&bt, 1, 1, fptr)) return SQERR_BADFILE;

//		printf("*** Read byte %d\n", bt);

		for(int z = 0, f = 128;z < 8;z++,f >>= 1)
		{
		    ind = (bt & f) ? 1 : 0;
//		    printf("ind: %d, %d\n", ind, (bt & f));

		    memcpy(scan+count, pal+ind, sizeof(RGB));

//		    (scan+count)->a = (bAND[pixel]) ? 0 : 255;

//		    printf("pixel: %d\n", pixel);
		    count++;
		    pixel++;
		}
	    }
	break;

	case 4:
	    j = 0;
	    do
	    {
		if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

		ind = bt >> 4;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
		ind = bt&15;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
	    }while(j < finfo->image[currentImage].w);

	break;

	case 8:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

		memcpy(scan+i, pal+bt, sizeof(RGB));
//		(scan+i)->a = (bAND[pixel]) ? 0 : 255;
		pixel++;
	    }
	break;
	
	case 24:
	{
	    RGB rgb;
	    
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		if(!sq_fread(&rgb, 1, sizeof(RGB), fptr)) return SQERR_BADFILE;

		(scan+i)->r = rgb.b;
		(scan+i)->g = rgb.g;
		(scan+i)->b = rgb.r;
		pixel++;
	    }
	}
	break;
	
	case 32:
	{
	    RGBA rgba;
	    
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		if(!sq_fread(&rgba, 1, sizeof(RGBA), fptr)) return SQERR_BADFILE;

		(scan+i)->r = rgba.b;
		(scan+i)->g = rgba.g;
		(scan+i)->b = rgba.r;
		(scan+i)->a = rgba.a;
		pixel++;
	    }
	}
	break;
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    ICO_HEADER		m_ifh;
    ICO_DIRENTRY	m_ide;
    BITMAPINFO_HEADER	m_bih;
    int			m_pixel = 0;
    RGB 		m_pal[256];
    int 		m_pal_entr;
    long		pos;
    RGBA		rgba;
    int 		m_bytes, r, count, count2, count3;
    jmp_buf		jmp;
    FILE 		*m_fptr;
    int 		w, h, bpp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	fclose(m_fptr);

	return SQERR_BADFILE;
    }

    if(!sq_fread(&m_ifh, sizeof(ICO_HEADER), 1, m_fptr)) longjmp(jmp, 1);

    if(m_ifh.idType != 1 && m_ifh.idType != 2)
	longjmp(jmp, 1);
	
    if(!sq_fread(&m_ide, sizeof(ICO_DIRENTRY), 1, m_fptr)) longjmp(jmp, 1);

    w = m_ide.bWidth;
    h = m_ide.bHeight;

    fseek(m_fptr, m_ide.dwImageOffset, SEEK_SET);
    if(!sq_fread(&m_bih, sizeof(BITMAPINFO_HEADER), 1, m_fptr)) longjmp(jmp, 1);

    bpp = m_bih.BitCount;

    if(bpp < 16)
    {
	m_pal_entr = 1 << bpp;

	for(int i = 0;i < m_pal_entr;i++)
	{
		if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

		m_pal[i].r = rgba.b;
		m_pal[i].g = rgba.g;
		m_pal[i].b = rgba.r;
	}
    }
    else
    {
	m_pal_entr = 0;
    }

    pos = ftell(m_fptr);

    count = w * h;
//    int count2 = count / (8 / bpp);
    count2 = (bpp < 16) ? (count / (8 / bpp)) : (count * (bpp / 8));

    count3 = count / 8;

    fseek(m_fptr, count2, SEEK_CUR);

    uchar m_bAND[count];

    if(!m_bAND)
        longjmp(jmp, 1);

    uchar realAND[count3];

    if(!sq_fread(realAND, 1, count3, m_fptr)) longjmp(jmp, 1);

    r = 0;
    for(int i = 0;i < count3;i++)
    {
	for(int z = 0,f = 128;z < 8;z++,f >>= 1)
	{
	    m_bAND[r] = (realAND[i] & f) ? 1 : 0;
	    r++;
	}
    }

    fsetpos(m_fptr, (fpos_t*)&pos);

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
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
    	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */


    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

        int i, j, count;
	unsigned char bt, ind;

	switch(bpp)
	{
    	    case 1:
		j = w / 8;
		count = 0;

		for(i = 0;i < j;i++)
		{
		    if(!sq_fread(&bt, 1, 1, m_fptr)) longjmp(jmp, 1);

//			printf("*** Read byte %d\n", bt);

		    for(int z = 0, f = 128;z < 8;z++,f >>= 1)
		    {
			ind = (bt & f) ? 1 : 0;
//		    	printf("ind: %d, %d\n", ind, (bt & f));

			memcpy(scan+count, m_pal+ind, sizeof(RGB));

//		    	(scan+count)->a = (m_bAND[m_pixel]) ? 0 : 255;
			count++;
			m_pixel++;
		    }
		}
	    break;

	    case 4:
		j = 0;
		do
		{
		    if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);

		    ind = bt >> 4;
		    memcpy(scan+j, m_pal+ind, sizeof(RGB));
//			(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    j++;
		    m_pixel++;
		    ind = bt&15;
		    memcpy(scan+j, m_pal+ind, sizeof(RGB));
//			(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    j++;
		    m_pixel++;
		}while(j < w);
	    break;

	    case 8:
		for(i = 0;i < w;i++)
		{
		    if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);

		    memcpy(scan+i, m_pal+bt, sizeof(RGB));
//			(scan+i)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    m_pixel++;
		}
	    break;

	    case 24:
	    {
		RGB rgb;

		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&rgb, 1, sizeof(RGB), m_fptr)) longjmp(jmp, 1);

		    (scan+i)->r = rgb.b;
		    (scan+i)->g = rgb.g;
		    (scan+i)->b = rgb.r;
		    m_pixel++;
		}
	    }
	    break;

	    case 32:
	    {
		RGBA rgba;

		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&rgba, 1, sizeof(RGBA), m_fptr)) longjmp(jmp, 1);

		    (scan+i)->r = rgba.b;
		    (scan+i)->g = rgba.g;
		    (scan+i)->b = rgba.r;
		    (scan+i)->a = rgba.a;
		    m_pixel++;
		}
	    }
	    break;
	}
    }

    fclose(m_fptr);

    flip((char*)*image, w * sizeof(RGBA), h);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
    
    if(bAND)
	free(bAND);
}

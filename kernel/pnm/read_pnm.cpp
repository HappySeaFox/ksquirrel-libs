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

#include "read_pnm.h"

PPM_HEADER	pfh;
int		pnm;
FILE 		*fptr;
int 		currentImage, bytes;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.0";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"Portable aNy Map";
}
	
const char* fmt_filter()
{
    return (const char*)"*.ppm *.pgm *.pbm *.pnm ";
}
	    
const char* fmt_mime()
{
    return (const char*)"P[123456]";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,80,80,4,4,4,254,242,52,76,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,189,2,15,22,176,216,160,153,128,102,128,246,145,253,87,41,1,154,123,157,44,48,0,88,27,4,175,136,104,10,73,93,45,17,93,13,198,76,110,12,154,185,123,136,25,221,153,9,89,201,180,35,53,243,111,157,158,106,166,246,209,188,154,111,60,31,79,50,23,69,141,112,85,89,0,0,0,0,73,69,78,68,174,66,96,130,130";
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

    char		str[256];
    int			w, h, maxcolor;

    fread(&pfh, 2, 1, fptr);
    pfh.ID[2] = '\0';

    fgetc(fptr);

    for(;;)
    {
	fgets(str, 255, fptr);
        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);
    if(pfh.ID[1] != '4')
	fscanf(fptr, "%d", &maxcolor);

    finfo->image[currentImage].w = w;
    finfo->image[currentImage].h = h;

    pnm = pfh.ID[1] - 48;

    if(pnm != 4 && pnm != 1)
	fgetc(fptr);
	
    switch(pnm)
    {
	case 1:
	case 4:
	    finfo->image[currentImage].bpp = 1;
	break;

	case 2:
	case 5:
	    finfo->image[currentImage].bpp = 8;
	break;

	case 3:
	case 6:
	    finfo->image[currentImage].bpp = 8;
	break;
    }

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->images++;
	
    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	(pnm == 1 || pnm == 4) ? "Monochrome":"Color indexed",
	bytes);

    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    RGB		rgb;
    uchar	bt;
    int		i;


    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(pnm)
    {
	case 3:
    	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr ,"%d%d%d", (int*)&rgb.r, (int*)&rgb.g, (int*)&rgb.b);
		memcpy(scan+i, &rgb, 3);
	    }
	break;

	case 6:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fread(&rgb,sizeof(RGB),1,fptr);
		memcpy(scan+i, &rgb, 3);
    	    }
	break;
	
	case 2:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr ,"%d", (int*)&bt);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
    
	case 5:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fread(&bt,1,1,fptr);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
	
	case 1:
        {
    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr ,"%d", (int*)&bt);
		memcpy(scan+i, palmono+(int)bt, 3);
    	    }
	}
	break;

	case 4:
	{
	    int index;//, remain = finfo->image[currentImage].w % 8;

    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;;)
	    {
		fread(&bt,1,1,fptr);
		index = (bt&128)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&64)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&32)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&16)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&8)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&4)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&2)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
		index = (bt&1);
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->image[currentImage].w) break;
	    }
	}
	break;
    }

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    PPM_HEADER	m_pfh;
    int		m_pnm;
    FILE *m_fptr;
    int w, h, bpp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    char		str[256];
    int			maxcolor;

    fread(&m_pfh, 2, 1, m_fptr);
    m_pfh.ID[2] = '\0';

    fgetc(m_fptr);

    for(;;)
    {
	fgets(str, 255, m_fptr);
        if(*str != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);
    if(m_pfh.ID[1] != '4')
	fscanf(m_fptr, "%d", &maxcolor);

    m_pnm = m_pfh.ID[1] - 48;

    if(m_pnm != 4 && m_pnm != 1)
	fgetc(m_fptr);
	
    switch(m_pnm)
    {
	case 1:
	case 4:
	    bpp = 1;
	break;

	case 2:
	case 5:
	    bpp = 8;
	break;

	case 3:
	case 6:
	    bpp = 8;
	break;
	
	default:
	    bpp = 8;
    }

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	(m_pnm == 1 || m_pnm == 4) ? "Monochrome":"Color indexed",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_pix: Image is null!\n");
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    int W = w * sizeof(RGBA);
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	memset(scan, 255, W);

    RGB		rgb;
    uchar	bt;
    int 	i;

    memset(scan, 255, W);

    switch(m_pnm)
    {
	case 3:
    	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr ,"%d%d%d", (int*)&rgb.r, (int*)&rgb.g, (int*)&rgb.b);
		memcpy(scan+i, &rgb, 3);
	    }
	break;

	case 6:
	    for(i = 0;i < w;i++)
	    {
		fread(&rgb,sizeof(RGB),1,m_fptr);
		memcpy(scan+i, &rgb, 3);
    	    }
	break;
	
	case 2:
	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr ,"%d", (int*)&bt);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
    
	case 5:
	    for(i = 0;i < w;i++)
	    {
		fread(&bt,1,1,m_fptr);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
	
	case 1:
        {
    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr ,"%d", (int*)&bt);
		memcpy(scan+i, palmono+(int)bt, 3);
    	    }
	}
	break;

	case 4:
	{
	    int index;//, remain = finfo->image[currentImage].w % 8;

    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;;)
	    {
		fread(&bt,1,1,m_fptr);
		index = (bt&128)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&64)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&32)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&16)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&8)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&4)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&2)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		index = (bt&1);
		memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
	    }
	}
	break;
    }
    }

    fclose(m_fptr);

    return SQERR_OK;
}

int fmt_close()
{
    fclose(fptr);

    return SQERR_OK;
}

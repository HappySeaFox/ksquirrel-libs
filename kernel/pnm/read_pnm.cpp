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
#include <ctype.h>

#include "read_pnm.h"

PPM_HEADER	pfh;
int		pnm;
FILE 		*fptr;
int 		currentImage, bytes;
char		format[10];
double		koeff;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.2";
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

    char		str[256];
    int			w, h;
    unsigned int	 maxcolor;

    fread(&pfh, 2, 1, fptr);
    pfh.ID[2] = '\0';

    fgetc(fptr);

    while(true)
    {
	fgets(str, 255, fptr);
        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    finfo->image[currentImage].w = w;
    finfo->image[currentImage].h = h;

    printf("%dx%d\n", w, h);

    pnm = pfh.ID[1] - 48;
    
    printf("PNM: %d\n", pnm);

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

    if(pnm != 4 && pnm != 1)
    {
	fscanf(fptr, "%d", &maxcolor);
	
	if((pnm == 5 || pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(pnm == 2 || pnm == 3)
	    skip_flood(fptr);
	else
	    fgetc(fptr);

	if(maxcolor <= 9)
	    strcpy(format, "%1d");
	else if(maxcolor >= 9 && maxcolor <= 99)
	    strcpy(format, "%2d");
	else if(maxcolor > 99 && maxcolor <= 999)
	    strcpy(format, "%3d");
	else if(maxcolor > 999 && maxcolor <= 9999)
	    strcpy(format, "%4d");

	koeff = 255.0 / maxcolor;
    }
    else if(pnm == 1)
    {
	strcpy(format, "%1d");
	koeff = 1.0;
    }
    
    printf("maxcolor: %d, format: %s, koeff: %.1f\n\n", maxcolor, format, koeff);

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
	case 1:
        {
    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};
	    int d;

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);

		d = (int)(d * koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }
	    
	    skip_flood(fptr);
	}
	break;

	case 2:
	{
	    int d;

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);

		d = (int)(d * koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    skip_flood(fptr);
	}
	break;

	case 3:
    	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr, format, (int*)&rgb.r);
		fscanf(fptr, format, (int*)&rgb.g);
		fscanf(fptr, format, (int*)&rgb.b);
		memcpy(scan+i, &rgb, sizeof(RGB));
	    }

	    skip_flood(fptr);
	break;

	case 6:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fread(&rgb, sizeof(RGB), 1, fptr);
		memcpy(scan+i, &rgb, sizeof(RGB));
    	    }
	break;

	case 5:
	{
	    long pos;
	    pos = ftell(fptr);
	    printf("POS1: %d, ", pos);

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fread(&bt,1,1,fptr);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
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

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    int w, h, bpp = 0;
    PPM_HEADER	m_pfh;
    int		m_pnm;
    FILE 	*m_fptr;
    char	m_format[10];
    double	m_koeff = 1.0;

    m_fptr = fopen(file, "rb");
				        
    if(!m_fptr)
        return SQERR_NOFILE;

    char		str[256];
    unsigned int	 maxcolor;

    fread(&m_pfh, 2, 1, m_fptr);
    m_pfh.ID[2] = '\0';

    fgetc(m_fptr);

    while(true)
    {
	fgets(str, 255, m_fptr);
        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    w = w;
    h = h;

    printf("%dx%d\n", w, h);

    m_pnm = m_pfh.ID[1] - 48;
    
    printf("PNM: %d\n", m_pnm);

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
    }

    if(m_pnm != 4 && m_pnm != 1)
    {
	fscanf(m_fptr, "%d", &maxcolor);
	
	if((m_pnm == 5 || m_pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(m_pnm == 2 || m_pnm == 3)
	    skip_flood(m_fptr);
	else
	    fgetc(m_fptr);
	
	if(maxcolor <= 9)
	    strcpy(m_format, "%1d");
	else if(maxcolor >= 9 && maxcolor <= 99)
	    strcpy(m_format, "%2d");
	else if(maxcolor > 99 && maxcolor <= 999)
	    strcpy(m_format, "%3d");
	else if(maxcolor > 999 && maxcolor <= 9999)
	    strcpy(m_format, "%4d");

	m_koeff = 255.0 / maxcolor;
    }
    else if(m_pnm == 1)
    {
	strcpy(m_format, "%1d");
	m_koeff = 1.0;
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
        fprintf(stderr, "libSQ_read_pnm: Image is null!\n");
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

    RGB		rgb;
    uchar	bt;
    int		i;

    switch(m_pnm)
    {
	case 1:
        {
    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};
	    int d;

	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr, m_format, &d);

		d = (int)(d * m_koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }
	    
	    skip_flood(m_fptr);
	}
	break;

	case 2:
	{
	    int d;

	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr, m_format, &d);

		d = (int)(d * m_koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    skip_flood(m_fptr);
	}
	break;

	case 3:
    	    for(i = 0;i < w;i++)
	    {
		fscanf(m_fptr, m_format, (int*)&rgb.r);
		fscanf(m_fptr, m_format, (int*)&rgb.g);
		fscanf(m_fptr, m_format, (int*)&rgb.b);
		memcpy(scan+i, &rgb, 3);
	    }

	    skip_flood(m_fptr);
	break;

	case 6:
	    for(i = 0;i < w;i++)
	    {
		fread(&rgb,sizeof(RGB),1,m_fptr);
		memcpy(scan+i, &rgb, sizeof(RGB));
    	    }
	break;

	case 5:
	{
	    long pos;
	    pos = ftell(m_fptr);
	    printf("POS1: %d, ", pos);

	    for(i = 0;i < w;i++)
	    {
		fread(&bt,1,1,m_fptr);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
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

void skip_flood(FILE *f)
{
    long pos;
    unsigned char b;

    while(true)
    {
	pos = ftell(f);
	fread(&b, 1, 1, f);
	
	if(feof(f) || ferror(f))
	    break;

	if(!isspace(b))
	{
	    if(b == '#')
	    {
		while(true)
		{
		    b = fgetc(f);

		    if(b == '\n')
			break;
		}
	    }

	    break;
	}
    }

    fsetpos(f, (fpos_t*)&pos);
}

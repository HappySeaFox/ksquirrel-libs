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
#include <ctype.h>
#include <setjmp.h>

#include "read_pnm.h"
#include "utils.h"

int		pnm;
FILE 		*fptr;
int 		currentImage, bytes;
char		format[10];
double		koeff;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.3";
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
    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;

    char		str[256];
    int			w, h;
    unsigned int	maxcolor;

    if(!sq_fgets(str, 255, fptr)) return SQERR_BADFILE;

    pnm = str[1] - 48;

    if(pnm < 1 || pnm > 6)
	return SQERR_BADFILE;

    while(true)
    {
	if(!sq_fgets(str, 255, fptr)) return SQERR_BADFILE;

        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    finfo->image[currentImage].w = w;
    finfo->image[currentImage].h = h;

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
	if(sq_ferror(fptr)) return SQERR_BADFILE;

	if((pnm == 5 || pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(pnm == 2 || pnm == 3)
	{
	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	else
	{
	    fgetc(fptr);
	    if(sq_ferror(fptr)) return SQERR_BADFILE;
	}

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
    
//    printf("maxcolor: %d, format: %s, koeff: %.1f\n\n", maxcolor, format, koeff);

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->images++;
	
    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
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
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		d = (int)(d * koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }
	    
	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	break;

	case 2:
	{
	    int d;

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		d = (int)(d * koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	break;

	case 3:
    	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		fscanf(fptr, format, (int*)&rgb.r);
		fscanf(fptr, format, (int*)&rgb.g);
		fscanf(fptr, format, (int*)&rgb.b);
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }

	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	break;

	case 6:
	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
//		(scan+i)->r = rgb.
    	    }
	break;

	case 5:
	{
	    long pos;
	    pos = ftell(fptr);
//	    printf("POS1: %d, ", pos);

	    for(i = 0;i < finfo->image[currentImage].w;i++)
	    {
		if(!sq_fread(&bt,1,1,fptr)) return SQERR_BADFILE;

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
		if(!sq_fread(&bt,1,1,fptr)) return SQERR_BADFILE;

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

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    int 	w, h, bpp;
    int		m_pnm;
    FILE 	*m_fptr;
    char	m_format[10];
    double	m_koeff;
    int 	m_bytes;
    char	str[256];
    unsigned int maxcolor;
    jmp_buf	jmp;

    m_fptr = fopen(file, "rb");
				        
    if(!m_fptr)
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	fclose(m_fptr);
	return SQERR_BADFILE;
    }


    if(!sq_fgets(str, 255, m_fptr)) longjmp(jmp, 1);

    m_pnm = str[1] - 48;

    if(m_pnm < 1 || m_pnm > 6)
	longjmp(jmp, 1);

    while(true)
    {
	if(!sq_fgets(str, 255, m_fptr)) longjmp(jmp, 1);

        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    w = w;
    h = h;

    bpp = 0;
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
	if(sq_ferror(m_fptr)) longjmp(jmp, 1);

	if((m_pnm == 5 || m_pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(m_pnm == 2 || m_pnm == 3)
	{
	    if(!skip_flood(m_fptr))
		longjmp(jmp, 1);
	}
	else
	{
	    fgetc(m_fptr);
	    if(sq_ferror(m_fptr)) longjmp(jmp, 1);
	}
	
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
    else m_koeff = 1.0;

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
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
	longjmp(jmp, 1);
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
	    
		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    }
	    break;

	    case 2:
	    {
		int d;

		for(i = 0;i < w;i++)
		{
		    fscanf(m_fptr, m_format, &d);
		    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

		    d = (int)(d * m_koeff);

		    memset(scan+i, d, sizeof(RGB));
		}
	    
		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    }
	    break;

	    case 3:
    		for(i = 0;i < w;i++)
		{
		    fscanf(m_fptr, m_format, (int*)&rgb.r);
		    fscanf(m_fptr, m_format, (int*)&rgb.g);
		    fscanf(m_fptr, m_format, (int*)&rgb.b);
		    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

		    memcpy(scan+i, &rgb, 3);
		}

		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    break;

	    case 6:
		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&rgb,sizeof(RGB),1,m_fptr)) longjmp(jmp, 1);

		    memcpy(scan+i, &rgb, sizeof(RGB));
    		}
	    break;

	    case 5:
	    {
		long pos;
		pos = ftell(m_fptr);
//		printf("POS1: %d, ", pos);

		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&bt,1,1,m_fptr)) longjmp(jmp, 1);

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
		    if(!sq_fread(&bt,1,1,m_fptr)) longjmp(jmp, 1);

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

void fmt_close()
{
    fclose(fptr);
}

bool skip_flood(FILE *f)
{
    long pos;
    unsigned char b;

    while(true)
    {
	pos = ftell(f);
	if(!sq_fread(&b, 1, 1, f)) return false;

	if(!isspace(b))
	{
	    if(b == '#')
	    {
		while(true)
		{
		    if(!sq_fgetc(f, &b)) return false;

		    if(b == '\n')
			break;
		}
	    }

	    break;
	}
    }

    fsetpos(f, (fpos_t*)&pos);
    
    return true;
}

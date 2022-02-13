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

#include "read_bmp.h"
#include "utils.h"

const char* fmt_version()
{
    return (const char*)"0.9.1";
}

const char* fmt_quickinfo()
{
    return (const char*)"Windows Bitmap";
}

const char* fmt_filter()
{
    return (const char*)"*.bmp *.dib ";
}

const char* fmt_mime()
{
    return (const char*)"BM";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,0,255,4,4,4,55,45,89,24,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,99,96,96,8,5,1,6,32,48,20,20,20,20,5,49,2,149,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,139,138,147,147,10,144,161,162,162,164,228,228,132,42,2,100,192,213,128,24,48,93,112,115,32,38,43,129,44,11,20,132,218,10,118,70,0,0,73,168,23,77,189,109,216,200,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

RGB		*pal;
int		pal_entr;
unsigned short	FILLER;
FILE 		*fptr;
int		currentImage;
int		bytes;
BITMAPFILE_HEADER	bfh;
BITMAPINFO_HEADER	bih;


int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;

    fptr = fopen(file, "rb");
    
    if(!fptr)
	return SQERR_NOFILE;

    pal_entr = 0;    
    pal = 0;

    currentImage = -1;

    fread(&bfh, sizeof(BITMAPFILE_HEADER), 1, fptr);
    fread(&bih, sizeof(BITMAPINFO_HEADER), 1, fptr);

    if(bih.Size != 40)
    	return SQERR_BADFILE;

    if(bih.Compression != BI_RGB)
	return SQERR_NOTSUPPORTED;

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

    RGBA		rgba;
    long		i, j, scanShouldBe;

    if(bih.BitCount < 16)
    	pal_entr = 1 << bih.BitCount;
    else
	pal_entr = 0;

    finfo->image[currentImage].w = bih.Width;
    finfo->image[currentImage].h = bih.Height;
    finfo->image[currentImage].bpp = bih.BitCount;
    scanShouldBe = bih.Width;

    switch(finfo->image[currentImage].bpp)
    {
	case 1:
	{
	    long tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((finfo->image[currentImage].w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;
	
	default:
	    return SQERR_BADFILE;
    }

    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    FILLER = j;
	    break;
	}

    if(finfo->image[currentImage].bpp < 16)
    {
	if((pal = (RGB *)calloc(pal_entr, sizeof(RGB))) == 0)
	{
		fclose(fptr);
		return SQERR_NOMEMORY;
	}

	/*  read palette  */
	for(i = 0;i < pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, fptr);
		(pal)[i].r = rgba.b;
		(pal)[i].g = rgba.g;
		(pal)[i].b = rgba.r;
		
	}
    }
    else
	pal = 0;

    /*  fseek to image bits  */
    fseek(fptr, bfh.OffBits, SEEK_SET);
    
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->image[currentImage].needflip = true;
    finfo->images++;

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	(pal_entr) ? "Color indexed":"RGB",
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    unsigned short remain, scanShouldBe, j, counter = 0;
    unsigned char bt;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    switch(finfo->image[currentImage].bpp)
    {
    	case 1:
	{
		unsigned char	index;
		remain=((finfo->image[currentImage].w)<=8)?(0):((finfo->image[currentImage].w)%8);
		scanShouldBe = finfo->image[currentImage].w;;

		long tmp = scanShouldBe;
		scanShouldBe /= 8;
		scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
 
		// @todo get rid of miltiple 'if'
		for(j = 0;j < scanShouldBe;j++)
		{
			fread(&bt, 1, 1, fptr);
			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (pal)+index, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(fptr);
	}
	break;

	case 4:
	{
		unsigned char	index;
		remain = (finfo->image[currentImage].w)%2;

		int ck = (finfo->image[currentImage].w%2)?(finfo->image[currentImage].w + 1):(finfo->image[currentImage].w);
		ck /= 2;

		for(j = 0;j < ck-1;j++)
		{
			fread(&bt, 1, 1, fptr);
			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		fread(&bt, 1, 1, fptr);
		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(fptr);
	}
	break;

	case 8:
	{
		
		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			fread(&bt, 1, 1, fptr);
			memcpy(scan+(counter++), (pal)+bt, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(fptr);
	}
	break;

	case 16:
	{
		unsigned short word;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			fread(&word, 2, 1, fptr);
			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < FILLER;j++) fgetc(fptr);
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			fread(&rgb, sizeof(RGB), 1, fptr);
			scan[counter].r = rgb.b;
			scan[counter].g = rgb.g;
			scan[counter].b = rgb.r;
			counter++;
		}

		for(j = 0;j < FILLER;j++) fgetc(fptr);
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			fread(&rgba, sizeof(RGBA), 1, fptr);
			scan[j].r = rgba.b;
			scan[j].g = rgba.g;
			scan[j].b = rgba.r;
		}
	}
	break;

    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    int 		w, h, bpp;
    RGB			*m_pal;
    int			m_pal_entr;
    unsigned short	m_FILLER = 0;
    FILE 		*m_fptr;
    BITMAPFILE_HEADER	m_bfh;
    BITMAPINFO_HEADER	m_bih;

    m_fptr = fopen(file, "rb");
				        
    if(!m_fptr)
        return SQERR_NOFILE;

    m_pal_entr = 0;    
    m_pal = 0;

    fread(&m_bfh, sizeof(BITMAPFILE_HEADER), 1, m_fptr);
    fread(&m_bih, sizeof(BITMAPINFO_HEADER), 1, m_fptr);

    if(m_bih.Size != 40)
    	return SQERR_BADFILE;

    if(m_bih.Compression != BI_RGB)
	return SQERR_NOTSUPPORTED;

    RGBA		rgba;
    long		i, j, scanShouldBe;

    if(m_bih.BitCount < 16)
    	m_pal_entr = 1 << m_bih.BitCount;
    else
	m_pal_entr = 0;

    w = m_bih.Width;
    h = m_bih.Height;
    bpp = m_bih.BitCount;
    scanShouldBe = m_bih.Width;

    switch(bpp)
    {
	case 1:
	{
	    long tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;

	default:
	    return SQERR_BADFILE;
    }

    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    m_FILLER = j;
	    break;
	}

    if(bpp < 16)
    {
	if((m_pal = (RGB *)calloc(m_pal_entr, sizeof(RGB))) == 0)
	{
		fclose(m_fptr);
		return SQERR_NOMEMORY;
	}

	/*  read palette  */
	for(i = 0;i < m_pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, m_fptr);
		(m_pal)[i].r = rgba.b;
		(m_pal)[i].g = rgba.g;
		(m_pal)[i].b = rgba.r;
		
	}
    }
    else
	m_pal = 0;

    /*  fseek to image bits  */
    fseek(m_fptr, m_bfh.OffBits, SEEK_SET);

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	(m_pal_entr)?"Color indexed":"RGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_bmp: Image is null!\n");
        fclose(m_fptr);
	
	free(m_pal);

        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

    unsigned short remain, scanShouldBe, j, counter = 0;
    unsigned char bt;

    switch(bpp)
    {
    	case 1:
	{
		unsigned char	index;
		remain=((w)<=8)?(0):((w)%8);
		scanShouldBe = w;;

		long tmp = scanShouldBe;
		scanShouldBe /= 8;
		scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
 
		// @todo get rid of miltiple 'if'
		for(j = 0;j < scanShouldBe;j++)
		{
			fread(&bt, 1, 1, m_fptr);
			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		for(j = 0;j < m_FILLER;j++) fgetc(m_fptr);
	}
	break;

	case 4:
	{
		unsigned char	index;
		remain = (w)%2;

		int ck = (w%2)?(w + 1):(w);
		ck /= 2;

		for(j = 0;j < ck-1;j++)
		{
			fread(&bt, 1, 1, m_fptr);
			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (m_pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		fread(&bt, 1, 1, m_fptr);
		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (m_pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		for(j = 0;j < m_FILLER;j++) fgetc(m_fptr);
	}
	break;

	case 8:
	{
		
		for(j = 0;j < w;j++)
		{
			fread(&bt, 1, 1, m_fptr);
			memcpy(scan+(counter++), (m_pal)+bt, 3);
		}

		for(j = 0;j < m_FILLER;j++) fgetc(m_fptr);
	}
	break;

	case 16:
	{
		unsigned short word;

		for(j = 0;j < w;j++)
		{
			fread(&word, 2, 1, m_fptr);
			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < m_FILLER;j++) fgetc(m_fptr);
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < w;j++)
		{
			fread(&rgb, sizeof(RGB), 1, m_fptr);
			scan[counter].r = rgb.b;
			scan[counter].g = rgb.g;
			scan[counter].b = rgb.r;
			counter++;
		}

		for(j = 0;j < m_FILLER;j++) fgetc(m_fptr);
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < w;j++)
		{
			fread(&rgba, sizeof(RGBA), 1, m_fptr);
			scan[j].r = rgba.b;
			scan[j].g = rgba.g;
			scan[j].b = rgba.r;
		}
	}
	break;

    }

    }
    
    fclose(m_fptr);
    
    if(m_pal)
	free(m_pal);

    flip((char*)*image, w * sizeof(RGBA), h);

    return SQERR_OK;
}

int fmt_close()
{
    if(pal)
	free(pal);

    fclose(fptr);

    return SQERR_OK;
}

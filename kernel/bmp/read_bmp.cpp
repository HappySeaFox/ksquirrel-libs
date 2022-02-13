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

#include "read_bmp.h"

#define SQ_NEED_FLIP
#include "utils.h"

const char* fmt_version()
{
    return (const char*)"1.0.2";
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
    return (const char*)0; // "BM" is too common to be a regexp :)
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,0,255,4,4,4,55,45,89,24,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,99,96,96,8,5,1,6,32,48,20,20,20,20,5,49,2,149,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,139,138,147,147,10,144,161,162,162,164,228,228,132,42,2,100,192,213,128,24,48,93,112,115,32,38,43,129,44,11,20,132,218,10,118,70,0,0,73,168,23,77,189,109,216,200,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

RGB		pal[256];
int		pal_entr;
unsigned short	FILLER;
FILE 		*fptr;
int		currentImage;
int		bytes;
BITMAPFILE_HEADER	bfh;
BITMAPINFO_HEADER	bih;


int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");

    if(!fptr)
	return SQERR_NOFILE;

    pal_entr = 0;    

    currentImage = -1;

    if(!sq_fread(&bfh, sizeof(BITMAPFILE_HEADER), 1, fptr))  return SQERR_BADFILE;
    if(!sq_fread(&bih, sizeof(BITMAPINFO_HEADER), 1, fptr))  return SQERR_BADFILE;

    if(bfh.Type != 0x4D42)
	return SQERR_BADFILE;

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

    if(!finfo)
	return SQERR_NOMEMORY;

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
	/*  read palette  */
	for(i = 0;i < pal_entr;i++)
	{
		if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

		(pal)[i].r = rgba.b;
		(pal)[i].g = rgba.g;
		(pal)[i].b = rgba.r;
		
	}
    }
    
    /*  fseek to image bits  */
    fseek(fptr, bfh.OffBits, SEEK_SET);
    
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    finfo->image[currentImage].needflip = true;
    finfo->images++;

    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
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
    unsigned char bt, dummy;

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
			if(!sq_fread(&bt, 1, 1, fptr)) return SQERR_BADFILE;
			
			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
		}

		for(j = 0;j < FILLER;j++)
		    if(!sq_fgetc(fptr, &dummy))
			return SQERR_BADFILE;
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
			if(!sq_fread(&bt, 1, 1, fptr)) return SQERR_BADFILE;

			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		if(!sq_fread(&bt, 1, 1, fptr)) return SQERR_BADFILE;

		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		for(j = 0;j < FILLER;j++)
		    if(!sq_fgetc(fptr, &dummy))
			return SQERR_BADFILE;
	}
	break;

	case 8:
	{
		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			if(!sq_fread(&bt, 1, 1, fptr)) return SQERR_BADFILE;

			memcpy(scan+(counter++), (pal)+bt, 3);
		}

		for(j = 0;j < FILLER;j++)
		    if(!sq_fgetc(fptr, &dummy))
			return SQERR_BADFILE;
	}
	break;

	case 16:
	{
		unsigned short word;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			if(!sq_fread(&word, 2, 1, fptr)) return SQERR_BADFILE;
			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < FILLER;j++)
		    if(!sq_fgetc(fptr, &dummy))
			return SQERR_BADFILE;
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

			scan[counter].r = rgb.b;
			scan[counter].g = rgb.g;
			scan[counter].b = rgb.r;
			counter++;
		}

		for(j = 0;j < FILLER;j++)
		    if(!sq_fgetc(fptr, &dummy))
			return SQERR_BADFILE;
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
			if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

			scan[j].r = rgba.b;
			scan[j].g = rgba.g;
			scan[j].b = rgba.r;
		}
	}
	break;

    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    int 		w, h, bpp;
    RGB			m_pal[256];
    int			m_pal_entr;
    unsigned short	m_FILLER;
    FILE 		*m_fptr;
    BITMAPFILE_HEADER	m_bfh;
    BITMAPINFO_HEADER	m_bih;
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

    m_pal_entr = 0;    

    if(!sq_fread(&m_bfh, sizeof(BITMAPFILE_HEADER), 1, m_fptr)) longjmp(jmp, 1);
    if(!sq_fread(&m_bih, sizeof(BITMAPINFO_HEADER), 1, m_fptr)) longjmp(jmp, 1);

    if(m_bih.Size != 40)
    	longjmp(jmp, 1);

    if(m_bih.Compression != BI_RGB)
	longjmp(jmp, 1);

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
	    longjmp(jmp, 1);
    }

    m_FILLER = 0;
    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    m_FILLER = j;
	    break;
	}

    if(bpp < 16)
    {
	/*  read palette  */
	for(i = 0;i < m_pal_entr;i++)
	{
		if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

		(m_pal)[i].r = rgba.b;
		(m_pal)[i].g = rgba.g;
		(m_pal)[i].b = rgba.r;
		
	}
    }

    /*  fseek to image bits  */
    fseek(m_fptr, m_bfh.OffBits, SEEK_SET);

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
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
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	unsigned short remain, scanShouldBe, j, counter = 0;
	unsigned char bt, dummy;

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
			if(!sq_fread(&bt, 1, 1, m_fptr)) longjmp(jmp, 1);

			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (m_pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		for(j = 0;j < m_FILLER;j++)
		    if(!sq_fgetc(m_fptr, &dummy))
			longjmp(jmp, 1);
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
			if(!sq_fread(&bt, 1, 1, m_fptr)) longjmp(jmp, 1);

			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (m_pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		if(!sq_fread(&bt, 1, 1, m_fptr)) longjmp(jmp, 1);

		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (m_pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		for(j = 0;j < m_FILLER;j++)
		    if(!sq_fgetc(m_fptr, &dummy))
			longjmp(jmp, 1);
	    }
	    break;

	    case 8:
	    {
		for(j = 0;j < w;j++)
		{
			if(!sq_fread(&bt, 1, 1, m_fptr)) longjmp(jmp, 1);

			memcpy(scan+(counter++), (m_pal)+bt, 3);
		}

		for(j = 0;j < m_FILLER;j++)
		    if(!sq_fgetc(m_fptr, &dummy))
			longjmp(jmp, 1);
	    }
	    break;

	    case 16:
	    {
		unsigned short word;

		for(j = 0;j < w;j++)
		{
			if(!sq_fread(&word, 2, 1, m_fptr)) longjmp(jmp, 1);

			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < m_FILLER;j++)
		    if(!sq_fgetc(m_fptr, &dummy))
			longjmp(jmp, 1);
	    }
	    break;

	    case 24:
	    {
		RGB rgb;

		for(j = 0;j < w;j++)
		{
			if(!sq_fread(&rgb, sizeof(RGB), 1, m_fptr)) longjmp(jmp, 1);

			scan[counter].r = rgb.b;
			scan[counter].g = rgb.g;
			scan[counter].b = rgb.r;
			counter++;
		}

		for(j = 0;j < m_FILLER;j++)
		    if(!sq_fgetc(m_fptr, &dummy))
			longjmp(jmp, 1);
	    }
	    break;

	    case 32:
	    {
		RGBA rgba;

		for(j = 0;j < w;j++)
		{
			if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

			scan[j].r = rgba.b;
			scan[j].g = rgba.g;
			scan[j].b = rgba.r;
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
}

void fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

/*                                                              politely ignore all options */
int fmt_writeimage(const char *file, RGBA *image, int w, int h, const fmt_writeoptions &)
{
    FILE		*m_fptr;
    int			m_FILLER;
    RGBA		*scan;
    BITMAPFILE_HEADER	bfh;
    BITMAPINFO_HEADER	bih;
    
    if(!image || !file || !w || !h)
	return SQERR_NOTOK;

    m_fptr = fopen(file, "wb");

    if(!m_fptr)
	return SQERR_NOFILE;

    flip((char*)image, w * sizeof(RGBA), h);

    bfh.Type = 0x4D42;
    bfh.Size = 0;
    bfh.Reserved1 = 0;
    bfh.OffBits = sizeof(BITMAPFILE_HEADER) + sizeof(BITMAPINFO_HEADER);

    fwrite(&bfh, sizeof(BITMAPFILE_HEADER), 1, m_fptr);
    
    bih.Size = 40;
    bih.Width = w;
    bih.Height = h;
    bih.Planes = 1;
    bih.BitCount = 24;
    bih.Compression = BI_RGB;
    bih.SizeImage = 0;
    bih.XPelsPerMeter = 0;
    bih.YPelsPerMeter = 0;
    bih.ClrUsed = 0;
    bih.ClrImportant = 0;

    fwrite(&bih, sizeof(BITMAPINFO_HEADER), 1, m_fptr);

    m_FILLER = (w < 4) ? (4-w) : w%4;
    char f[m_FILLER];
    
    for(int y = 0;y < h;y++)
    {
	scan = image + w * y;

	for(int x = 0;x < w;x++)
	{
	    fwrite(&scan[x].b, 1, 1, m_fptr);
	    fwrite(&scan[x].g, 1, 1, m_fptr);
	    fwrite(&scan[x].r, 1, 1, m_fptr);
	}
	
	if(m_FILLER)
	    fwrite(f, sizeof(f), 1, m_fptr);
    }

    fclose(m_fptr);
    
    return SQERR_OK;
}

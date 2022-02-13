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

#include "read_tga.h"

#define SQ_NEED_FLIP
#include "utils.h"

typedef unsigned char uchar;

FILE *fptr;
int currentImage, bytes, pal_entr;
RGB pal[256];

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.6.1";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"TarGA";
}
	
const char* fmt_filter()
{
    return (const char*)"*.tga ";
}

const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,128,128,4,4,4,181,151,89,64,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,89,129,7,11,88,108,208,76,224,50,64,251,200,254,171,52,1,210,123,157,44,97,35,34,115,35,139,87,85,45,5,128,205,150,21,93,141,140,72,110,25,112,122,248,18,134,7,89,226,71,72,164,176,146,115,245,247,84,51,172,71,115,53,223,120,62,131,188,24,11,124,78,54,7,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    pal_entr = 0;

    return SQERR_OK;
}

TGA_FILEHEADER	tfh;

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

    if(!sq_fread(&tfh, sizeof(TGA_FILEHEADER), 1, fptr)) return SQERR_BADFILE;

    finfo->image[currentImage].w = tfh.ImageSpecW;
    finfo->image[currentImage].h = tfh.ImageSpecH;
    finfo->image[currentImage].bpp = tfh.ImageSpecDepth;
    pal_entr = 0;

    if(tfh.IDlength)
    {
	finfo->meta = (fmt_metainfo *)calloc(1, sizeof(fmt_metainfo));

	if(finfo->meta)
	{
		finfo->meta->entries++;
		finfo->meta->m = (fmt_meta_entry *)calloc(1, sizeof(fmt_meta_entry));
		fmt_meta_entry *entry = finfo->meta->m;

		if(entry)
		{
		    entry[0].datalen = tfh.IDlength+1;
		    strcpy(entry[0].group, "TGA image identification field");
		    entry[0].data = (char *)malloc(entry[0].datalen);

		    if(entry[0].data)
		    {
			if(!sq_fread(entry[0].data, tfh.IDlength, 1, fptr)) return SQERR_BADFILE;

			entry[0].data[tfh.IDlength] = 0;
		    }
		}
	}
    }

    if(tfh.ColorMapType)
    {
	pal_entr = tfh.ColorMapSpecLength;

//	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
//		return SQERR_NOMEMORY;

//	char sz = tfh.ColorMapSpecEntrySize;
	int  i;
//	unsigned short word;
  
	for(i = 0;i < pal_entr;i++)
	{
		/*if(sz==24)*/ if(!sq_fread(pal+i, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;
/* alpha ingored  *//*else if(sz==32) { fread(finfo->pal+i, sizeof(RGB), 1, fptr); fgetc(fptr); }
		else if(sz==16)
		{
		    fread(&word, 2, 1, fptr);
		    (finfo->pal)[i].b = (word&0x1f) << 3;
		    (finfo->pal)[i].g = ((word&0x3e0) >> 5) << 3;
		    (finfo->pal)[i].r = ((word&0x7c00)>>10) << 3;
		}*/
		
	}
    }
//    else
//	pal = 0;

    if(tfh.ImageType == 0)
	return SQERR_BADFILE;

    char comp[25], type[25];

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    finfo->image[currentImage].needflip = true;
    finfo->images++;
    finfo->image[currentImage].hasalpha = (finfo->image[currentImage].bpp == 32);

    switch(tfh.ImageType)
    {
	case 1:
	    strcpy(comp, "-");
	    strcpy(type, "Color indexed");
	break;

	case 2:
	    strcpy(comp, "-");
	    strcpy(type, (finfo->image[currentImage].bpp == 32) ? "RGBA":"RGB");
	break;

	case 3:
	    strcpy(comp, "-");
	    strcpy(type, "Monochrome");
	break;

	case 9:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, "Color indexed");
	break;

	case 10:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, (finfo->image[currentImage].bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, "Monochrome");
	break;
    }

    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n%s\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	type,
	comp,
	bytes);

//    printf("tfh.ImageType: %d, pal_len: %d\n", tfh.ImageType, tfh.ColorMapSpecLength);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int j, counter = 0;
    RGB rgb;
    RGBA rgba;

    memset(scan, 255, finfo->image[currentImage].w * 4);

    switch(tfh.ImageType)
    {
    	case 0:
	break;

	case 1:
	{
	}
	break;

	case 2:
	{
	    if(tfh.ImageSpecDepth==24)
	    {
		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
		    if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

		    (scan+counter)->r = rgb.b;
		    (scan+counter)->g = rgb.g;
		    (scan+counter)->b = rgb.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
		    if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

		    (scan+counter)->r = rgba.b;
		    (scan+counter)->g = rgba.g;
		    (scan+counter)->b = rgba.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==16)
	    {
		unsigned short word;

		for(j = 0;j < finfo->image[currentImage].w;j++)
		{
		    if(!sq_fread(&word, 2, 1, fptr)) return SQERR_BADFILE;

		    scan[counter].b = (word&0x1f) << 3;
		    scan[counter].g = ((word&0x3e0) >> 5) << 3;
		    scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}
	    }
	}
	break;

	case 3:
	break;

	// RLE + color mapped
	case 9:
	break;

	// RLE + true color
	case 10:
	{
	    uchar	bt, count;
	    ushort	counter = 0, word;
	    RGBA	rgba;
	    
	    for(;;)
	    {
		if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

		count = (bt&127) + 1;

    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(finfo->image[currentImage].bpp)
		    {
			case 16:
    			    if(!sq_fread(&word, 2, 1, fptr)) return SQERR_BADFILE;

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;

			case 24:
    			    if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;

			case 32:
    			    if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(finfo->image[currentImage].bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				if(!sq_fread(&word, 2, 1, fptr)) return SQERR_BADFILE;

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				if(!sq_fread(&rgba, sizeof(RGBA), 1, fptr)) return SQERR_BADFILE;

				(scan+counter)->r = rgba.b;
    				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;
				if(counter >= finfo->image[currentImage].w-1) goto lts;
			    }
			break;
		    }
		}
	    }
	}
	lts:
	break;

	// RLE + B&W
	case 11:
	break;
    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    FILE 		*m_fptr;
    int 		w, h, bpp;
    TGA_FILEHEADER 	m_tfh;
    RGB 		m_pal[256];
    int 		m_pal_entr;
    RGB 		rgb;
    RGBA 		rgba;
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
			    
    if(!sq_fread(&m_tfh, sizeof(TGA_FILEHEADER), 1, m_fptr)) longjmp(jmp, 1);

    w = m_tfh.ImageSpecW;
    h = m_tfh.ImageSpecH;
    bpp = m_tfh.ImageSpecDepth;
    m_pal_entr = 0;

    if(m_tfh.ColorMapType)
    {
	m_pal_entr = m_tfh.ColorMapSpecLength;

//	if((m_pal = (RGB*)calloc(m_pal_entr, sizeof(RGB))) == 0)
//		return SQERR_NOMEMORY;

//	char sz = m_tfh.ColorMapSpecEntrySize;
	int  i;
//	unsigned short word;
  
	for(i = 0;i < m_pal_entr;i++)
	{
		/*if(sz==24)*/ if(!sq_fread(m_pal+i, sizeof(RGB), 1, m_fptr)) longjmp(jmp, 1);
/* alpha ingored  *//*else if(sz==32) { fread(finfo->m_pal+i, sizeof(RGB), 1, m_fptr); fgetc(m_fptr); }
		else if(sz==16)
		{
		    fread(&word, 2, 1, m_fptr);
		    (finfo->m_pal)[i].b = (word&0x1f) << 3;
		    (finfo->m_pal)[i].g = ((word&0x3e0) >> 5) << 3;
		    (finfo->m_pal)[i].r = ((word&0x7c00)>>10) << 3;
		}*/
		
	}
    }
//    else
//	m_pal = 0;

    if(m_tfh.ImageType == 0)
	return SQERR_BADFILE;

    m_bytes = w * h * sizeof(RGBA);

    char type[25], comp[25];

    switch(m_tfh.ImageType)
    {
	case 1:
	    strcpy(comp, "-");
	    strcpy(type, "Color indexed");
	break;

	case 2:
	    strcpy(comp, "-");
	    strcpy(type, (bpp == 32) ? "RGBA":"RGB");
	break;

	case 3:
	    strcpy(comp, "-");
	    strcpy(type, "Monochrome");
	break;

	case 9:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, "Color indexed");
	break;

	case 10:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, (bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    strcpy(comp, "RLE with two types of data packets");
	    strcpy(type, "Monochrome");
	break;
    }

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n%s\n%d\n%d",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	type,
	comp,
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

    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

    int j, counter = 0;

    switch(m_tfh.ImageType)
    {
    	case 0:
	break;

	case 1:
	break;

	case 2:
	{
	    if(m_tfh.ImageSpecDepth==24)
	    {
		for(j = 0;j < w;j++)
		{
		    if(!sq_fread(&rgb, sizeof(RGB), 1, m_fptr)) longjmp(jmp, 1);

		    (scan+counter)->r = rgb.b;
		    (scan+counter)->g = rgb.g;
		    (scan+counter)->b = rgb.r;
		    counter++;
		}
	    }
	    else if(m_tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < w;j++)
		{
		    if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

		    (scan+counter)->r = rgba.b;
		    (scan+counter)->g = rgba.g;
		    (scan+counter)->b = rgba.r;
		    counter++;
		}
	    }
	    else if(m_tfh.ImageSpecDepth==16)
	    {
		unsigned short word;

		for(j = 0;j < w;j++)
		{
		    if(!sq_fread(&word, sizeof(unsigned short), 1, m_fptr)) longjmp(jmp, 1);

		    scan[counter].b = (word&0x1f) << 3;
		    scan[counter].g = ((word&0x3e0) >> 5) << 3;
		    scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}
	    }
	}
	break;

	case 3:
	break;

	// RLE + color mapped
	case 9:
	break;

	// RLE + true color
	case 10:
	{
	    uchar	bt, count;
	    ushort	counter = 0, word;
	    
	    for(;;)
	    {
		if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);
		count = (bt & 127) + 1;
		
    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(bpp)
		    {
			case 16:
    			    if(!sq_fread(&word, 2, 1, m_fptr)) longjmp(jmp, 1);

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= w-1) goto lts;
			    }
			break;

			case 24:
    			    if(!sq_fread(&rgb, sizeof(RGB), 1, m_fptr)) longjmp(jmp, 1);

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgb.b;
				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;

			case 32:
    			    if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				if(!sq_fread(&word, 2, 1, m_fptr)) longjmp(jmp, 1);

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				if(!sq_fread(&rgb, sizeof(RGB), 1, m_fptr)) longjmp(jmp, 1);

				(scan+counter)->r = rgb.b;
				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				if(!sq_fread(&rgba, sizeof(RGBA), 1, m_fptr)) longjmp(jmp, 1);

				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;
		    }
		}
	    }
	}
	lts:
	break;

	// RLE + B&W
	case 11:
	break;

    }

    }

    flip((char*)*image, w * sizeof(RGBA), h);

    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
}

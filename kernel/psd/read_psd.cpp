/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "read_psd.h"

FILE *fptr;
int bytes, currentImage;
int layer, compression, width, height, channels, depth, mode, line;
RGBA **last;
unsigned char *L;
RGB *pal;

const char* fmt_version()
{
    return (const char*)"0.7.0";
}

const char* fmt_quickinfo()
{
    return (const char*)"Adobe Photoshop PSD";
}

const char* fmt_filter()
{
    return (const char*)"*.psd ";
}

const char* fmt_mime()
{
/*  QRegExp pattern  */
    return (const char*)"\x0038\x0042\x0050\x0053\x0001";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,255,255,0,128,128,0,4,4,4,204,13,117,30,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,65,17,192,48,8,4,177,192,7,1,88,200,85,1,56,104,5,180,159,248,151,80,114,161,221,215,206,13,28,136,200,92,72,241,168,170,81,220,221,102,75,69,163,17,36,9,193,184,78,4,74,138,40,86,18,160,32,59,65,198,193,153,111,235,239,217,205,110,125,148,87,249,198,253,2,11,129,25,221,25,73,23,33,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
        return SQERR_NOMEMORY;
	
    fptr = fopen(file, "rb");

    if(!fptr)
        return SQERR_NOFILE;
	
    currentImage = -1;
    layer = -1;

    if(BE_getlong(fptr) != 0x38425053)
	return SQERR_NOTSUPPORTED;

    if(BE_getshort(fptr) != 1)
	return SQERR_NOTSUPPORTED;
	
    last = 0;
    L = 0;
    pal = 0;

    char dummy[6];
    fread(dummy, 6, 1, fptr);

    channels = BE_getshort(fptr);

//    printf("channels: %d\n", channels);

    if(channels != 3 && channels != 4 && channels != 1)
	return SQERR_NOTSUPPORTED;
	
    height = BE_getlong(fptr);
    width = BE_getlong(fptr);
    depth = BE_getshort(fptr);
    mode = BE_getshort(fptr);
    
//    printf("mode: %d, depth: %d\n", mode, depth);

    if(depth != 8)
	return SQERR_NOTSUPPORTED;

    if(mode != PSD_RGB && mode != PSD_CMYK && mode != PSD_INDEXED && mode != PSD_GRAYSCALE)
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_RGB && channels != 4)
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_CMYK && channels != 4 && channels != 5)
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_INDEXED && channels != 1)
	return SQERR_NOTSUPPORTED;

    int	data_count = BE_getlong(fptr);
    
//    printf("mode_data: %d\n", data_count);

    if(data_count)
    {
	pal = (RGB*)calloc(256, sizeof(RGB));
	
	if(!pal)
	    return SQERR_NOMEMORY;

//	fseek(fptr, data_count, SEEK_CUR);
	fread(pal, 256, sizeof(RGB), fptr);
    }

    // skip the image resources.  (resolution, pen tool paths, alpha channel names, etc)

    data_count = BE_getlong(fptr);
//    printf("data_count: %d\n", data_count);

    if(data_count)
	fseek(fptr, data_count, SEEK_CUR);

    // skip the reserved data

    data_count = BE_getlong(fptr);
//    printf("data_count: %d\n", data_count);

    if(data_count)
	fseek(fptr, data_count, SEEK_CUR);

    // find out if the data is compressed
    //   0: no compressiod
    //   1: RLE compressed

    compression = BE_getshort(fptr);

    if(compression != 1 && compression != 0)
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

    finfo->image[currentImage].hasalpha = (mode == PSD_RGB) ? true : ((channels == 5) ? true : false);
    finfo->image[currentImage].passes = (channels == 5) ? 4 : channels;
    finfo->image[currentImage].h = height;
    finfo->image[currentImage].w = width;
    
//    printf("passes: %d\n", finfo->image[currentImage].passes);

//    printf("compression: %d\n", compression);
//    printf("channels: %d\n", channels);

    if(compression)
    {
	unsigned short b[height * channels];

//	for(unsigned i = 0;i < height * channel_count;i++)
//	{
	    //fseek(fptr,  * 2, SEEK_CUR);
	    fread(b, 2, height * channels, fptr);
//	    b = BE_getshort(fptr);
//	    printf("%u,", b);
//	}
    }//printf("\n");
/*    
    printf("POS: %d\n", ftell(fptr));
    
    unsigned char c;
    fread(&c, 1, 1, fptr);
    printf("Byte: %d\n", c);
*/	
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
    
    char type[25];
    
    switch(mode)
    {
	case PSD_RGB:
	    strcpy(type, "RGB");
	    finfo->image[currentImage].bpp = 24;
	break;

	case PSD_CMYK:
	    strcpy(type, "CMYK");
	    finfo->image[currentImage].bpp = (channels == 5) ? 32 : 24;
	break;
	
	case PSD_INDEXED:
	    strcpy(type, "Color indexed");
	    finfo->image[currentImage].bpp = 8;
	break;

	case PSD_GRAYSCALE:
	    strcpy(type, "Grayscale");
	    finfo->image[currentImage].bpp = 8;
	break;
    }

    last = (RGBA**)calloc(height, sizeof(RGBA*));

    if(!last)
        return SQERR_NOMEMORY;

    const int S = width * sizeof(RGBA);

    for(int i = 0;i < height;i++)
    {
	last[i] = (RGBA*)malloc(S);

	if(!last[i])
	    return SQERR_NOMEMORY;
	    
	memset(last[i], 255, S);
    }
    
    line = -1;

    L = (unsigned char*)calloc(width, 1);
    
    if(!L)
	return SQERR_NOMEMORY;


    finfo->images++;

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n%s\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	type,
	((compression) ? "RLE" : "-"),
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    layer++;

    line = -1;

    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    unsigned char c, value, *p;
    int count = 0;
    
//    for(int i = 0;i < finfo->image[currentImage].w;i++)
//	scan[i].a = 255;

    line++;

    memcpy(scan, last[line], sizeof(RGBA) * finfo->image[currentImage].w);

    if(compression)
    {
	while(count < finfo->image[currentImage].w)
	{
	    fread(&c, 1, 1, fptr);

	    if(c == 128)
	    {} // do nothing
	    else if(c > 128)
	    {
		c ^= 0xff;
		c += 2;

		fread(&value, 1, 1, fptr);

		for(int i = count; i < count+c;i++)
		{
		    p = (unsigned char*)(scan+i);
		    *(p+layer) = value;
		}

		count += c;
	    }
	    else if(c < 128)
	    {
		c++;

		for(int i = count; i < count+c;i++)
		{
		    fread(&value, 1, 1, fptr);
		    p = (unsigned char*)(scan+i);
		    *(p+layer) = value;
		}

		count += c;
	    }
	}
    }
    else
    {
	fread(L, 1, width, fptr);

	for(int i = 0;i < width;i++)
	{
	    p = (unsigned char*)(scan+i);
	    *(p+layer) = L[i];
	}
    }

    memcpy(last[line], scan, sizeof(RGBA) * finfo->image[currentImage].w);

    if(layer == finfo->image[currentImage].passes-1)
    {
	if(mode == PSD_CMYK)
	{
	    for(int i = 0;i < finfo->image[currentImage].w;i++)
	    {
		scan[i].r = (scan[i].r * scan[i].a) >> 8;
		scan[i].g = (scan[i].g * scan[i].a) >> 8;
		scan[i].b = (scan[i].b * scan[i].a) >> 8;
	    
		if(channels == 4)
		    scan[i].a = 255;
	    }
	}
	else if(mode == PSD_INDEXED)
	{
	    unsigned char r;
	    const int z1 = 768/3;
	    const int z2 = z1 << 1;

	    for(int i = 0;i < finfo->image[currentImage].w;i++)
	    {
		unsigned char *p = (unsigned char*)pal;
		r = scan[i].r;

		(scan+i)->r = *(p+r);
		(scan+i)->g = *(p+r+z1);
		(scan+i)->b = *(p+r+z2);
		scan[i].a = 255;
	    }	    
	}
	else if(mode == PSD_GRAYSCALE)
	{
	    unsigned char v;

	    for(int i = 0;i < finfo->image[currentImage].w;i++)
	    {
		v = scan[i].r;

		(scan+i)->r = v;
		(scan+i)->g = v;
		(scan+i)->b = v;
		scan[i].a = 255;
	    }	    
	}
    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    FILE *m_fptr;
    int w, h, bpp;
    int m_layer, m_compression, m_channels, m_depth, m_mode, m_line, passes;
    RGBA **m_last;
    unsigned char *m_L;
    RGB *m_pal;

    m_fptr = fopen(file, "rb");
				        
    if(!m_fptr)
        return SQERR_NOFILE;

    m_layer = -1;

    if(BE_getlong(m_fptr) != 0x38425053)
    {
	fclose(m_fptr);
	return SQERR_NOTSUPPORTED;
    }

    if(BE_getshort(m_fptr) != 1)
    {
	fclose(m_fptr);
	return SQERR_NOTSUPPORTED;
    }
	
    m_last = 0;
    m_L = 0;
    m_pal = 0;

    char dummy[6];
    fread(dummy, 6, 1, m_fptr);

    m_channels = BE_getshort(m_fptr);

    if(m_channels != 3 && m_channels != 4 && m_channels != 1)
    {
	fclose(m_fptr);
	return SQERR_NOTSUPPORTED;
    }
	
    h = BE_getlong(m_fptr);
    w = BE_getlong(m_fptr);
    m_depth = BE_getshort(m_fptr);
    m_mode = BE_getshort(m_fptr);
    
    passes = (m_channels == 5) ? 4 : m_channels;
    
    if(m_depth != 8)
	return SQERR_NOTSUPPORTED;

    if(m_mode != PSD_RGB && m_mode != PSD_CMYK && m_mode != PSD_INDEXED && m_mode != PSD_GRAYSCALE)
	return SQERR_NOTSUPPORTED;

    if(m_mode == PSD_RGB && m_channels != 4)
	return SQERR_NOTSUPPORTED;

    if(m_mode == PSD_CMYK && m_channels != 4 && m_channels != 5)
	return SQERR_NOTSUPPORTED;

    if(m_mode == PSD_INDEXED && m_channels != 1)
	return SQERR_NOTSUPPORTED;

    int	data_count = BE_getlong(m_fptr);
    
//    printf("mode_data: %d\n", data_count);

    if(data_count)
    {
	m_pal = (RGB*)calloc(256, sizeof(RGB));
	
	if(!m_pal)
	    return SQERR_NOMEMORY;

//	fseek(m_fptr, data_count, SEEK_CUR);
	fread(m_pal, 256, sizeof(RGB), m_fptr);
    }

    // skip the image resources.  (resolution, pen tool paths, alpha channel names, etc)

    data_count = BE_getlong(m_fptr);
//    printf("data_count: %d\n", data_count);

    if(data_count)
	fseek(m_fptr, data_count, SEEK_CUR);

    // skip the reserved data

    data_count = BE_getlong(m_fptr);
//    printf("data_count: %d\n", data_count);

    if(data_count)
	fseek(m_fptr, data_count, SEEK_CUR);

    // find out if the data is compressed
    //   0: no compressiod
    //   1: RLE compressed

    m_compression = BE_getshort(m_fptr);

    if(m_compression != 1 && m_compression != 0)
	return SQERR_NOTSUPPORTED;



    /* Do something like fmt_next() here */


//    printf("m_compression: %d\n", m_compression);
//    printf("m_channels: %d\n", m_channels);

    if(m_compression)
    {
	unsigned short b[h * m_channels];

//	for(unsigned i = 0;i < h * channel_count;i++)
//	{
	    //fseek(m_fptr,  * 2, SEEK_CUR);
	    fread(b, 2, h * m_channels, m_fptr);
//	    b = BE_getshort(m_fptr);
//	    printf("%u,", b);
//	}
    }//printf("\n");
/*    
    printf("POS: %d\n", ftell(m_fptr));
    
    unsigned char c;
    fread(&c, 1, 1, m_fptr);
    printf("Byte: %d\n", c);
*/	
    
    char type[25];

    switch(m_mode)
    {
	case PSD_RGB:
	    strcpy(type, "RGB");
	    bpp = 24;
	break;

	case PSD_CMYK:
	    strcpy(type, "CMYK");
	    bpp = (channels == 5) ? 32 : 24;
	break;
	
	case PSD_INDEXED:
	    strcpy(type, "Color indexed");
	    bpp = 8;
	break;

	case PSD_GRAYSCALE:
	    strcpy(type, "Grayscale");
	    bpp = 8;
	break;
	
	default: bpp = 0;
    }
    
    m_last = (RGBA**)calloc(h, sizeof(RGBA*));

    if(!m_last)
        return SQERR_NOMEMORY;

    const int S = w * sizeof(RGBA);

    for(int i = 0;i < h;i++)
    {
	m_last[i] = (RGBA*)malloc(S);

	if(!m_last[i])
	    return SQERR_NOMEMORY;
	    
	memset(m_last[i], 255, S);
    }
    
    m_line = -1;

    m_L = (unsigned char*)calloc(w, 1);
    
    if(!m_L)
	return SQERR_NOMEMORY;

    int m_bytes = w * h * sizeof(RGBA);

    /*
	Dump has the following format: "%QUICK_INFO\n%WIDTH\n%HEIGHT\n
        %BPP\n%COLOR_TYPE\n%COMPRESSION\n%NUMBER_OF_IMAGES\n%TOTAL_BYTES_NEEDED"
    */
    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n%s\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	type,
	(m_compression) ? "RLE" : "-",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
        fprintf(stderr, "libSQ_read_psd: Image is null!\n");
        fclose(m_fptr);

    if(m_last)
    {
	for(int i = 0;i < h;i++)
	{
	    if(m_last[i])
		free(m_last[i]);
	}

	free(m_last);
    }
    
    if(m_L)
	free(m_L);

    if(m_pal)
	free(m_pal);

        return SQERR_NOMEMORY;
    }

    memset(*image, 0, m_bytes);

    for(m_layer = 0;m_layer < passes;m_layer++)    
    {m_line = -1;
    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

//	memset(scan, 255, w * sizeof(RGBA));

    unsigned char c, value, *p;
    int count = 0;
    
//    for(int i = 0;i < finfo->image[currentImage].w;i++)
//	scan[i].a = 255;

    m_line++;

    memcpy(scan, m_last[m_line], sizeof(RGBA) * w);

    if(m_compression)
    {
	while(count < w)
	{
	    fread(&c, 1, 1, m_fptr);

	    if(c == 128)
	    {} // do nothing
	    else if(c > 128)
	    {
		c ^= 0xff;
		c += 2;

		fread(&value, 1, 1, m_fptr);

		for(int i = count; i < count+c;i++)
		{
		    p = (unsigned char*)(scan+i);
		    *(p+m_layer) = value;
		}

		count += c;
	    }
	    else if(c < 128)
	    {
		c++;

		for(int i = count; i < count+c;i++)
		{
		    fread(&value, 1, 1, m_fptr);
		    p = (unsigned char*)(scan+i);
		    *(p+m_layer) = value;
		}

		count += c;
	    }
	}
    }
    else
    {
	fread(m_L, 1, w, m_fptr);

	for(int i = 0;i < w;i++)
	{
	    p = (unsigned char*)(scan+i);
	    *(p+m_layer) = m_L[i];
	}
    }

    memcpy(m_last[m_line], scan, sizeof(RGBA) * w);

    if(m_layer == passes-1)
    {
	if(m_mode == PSD_CMYK)
	{
	    for(int i = 0;i < w;i++)
	    {
		scan[i].r = (scan[i].r * scan[i].a) >> 8;
		scan[i].g = (scan[i].g * scan[i].a) >> 8;
		scan[i].b = (scan[i].b * scan[i].a) >> 8;
	    
		if(m_channels == 4)
		    scan[i].a = 255;
	    }
	}
	else if(m_mode == PSD_INDEXED)
	{
	    unsigned char r;
	    const int z1 = 768/3;
	    const int z2 = z1 << 1;

	    for(int i = 0;i < w;i++)
	    {
		unsigned char *p = (unsigned char*)m_pal;
		r = scan[i].r;

		(scan+i)->r = *(p+r);
		(scan+i)->g = *(p+r+z1);
		(scan+i)->b = *(p+r+z2);
		scan[i].a = 255;
	    }	    
	}
	else if(m_mode == PSD_GRAYSCALE)
	{
	    unsigned char v;

	    for(int i = 0;i < w;i++)
	    {
		v = scan[i].r;

		(scan+i)->r = v;
		(scan+i)->g = v;
		(scan+i)->b = v;
		scan[i].a = 255;
	    }	    
	}
    }

    }
    }

    /* Do something like fmt_close() here */
    fclose(m_fptr);

    if(m_last)
    {
	for(int i = 0;i < h;i++)
	{
	    if(m_last[i])
		free(m_last[i]);
	}

	free(m_last);
    }
    
    if(m_L)
	free(m_L);
	
    if(m_pal)
	free(m_pal);

    /* Everything OK */
    return SQERR_OK;
}

int fmt_close()
{
    fclose(fptr);

    if(last)
    {
	for(int i = 0;i < height;i++)
	{
	    if(last[i])
		free(last[i]);
	}

	free(last);
    }
    
    if(L)
	free(L);
	
    if(pal)
	free(pal);

    return SQERR_OK;
}

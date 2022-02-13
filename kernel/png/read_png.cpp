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

#include "read_png.h"

#include <png.h>

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

png_structp	png_ptr;
png_infop	info_ptr;
png_uint_32	width, height, number_passes;
int		color_type;
png_bytep	*row_pointers;
png_bytep	rows;
int		currentImage, bytes;

void my_error_exit(png_struct *, const char *mes)
{
    printf("libSQ_read_png: %s\n", mes);
}

const char* fmt_version()
{
    return (const char *)"0.9.1";
}

const char* fmt_quickinfo()
{
    return (const char *)"Portable Network Graphics";
}

const char* fmt_filter()
{
    return (const char *)"*.png ";
}

const char* fmt_mime()
{
    return (const char *)"\x0089\x0050\x004E\x0047\x000D\x000A\x001A\x000A";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,128,0,0,4,4,4,247,152,75,49,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,61,142,193,21,128,48,8,67,179,2,7,22,136,108,96,39,64,6,208,67,247,95,69,74,209,156,254,11,144,0,96,46,33,245,136,136,22,144,212,217,144,214,217,194,136,210,133,193,48,15,38,152,209,189,157,164,229,228,200,227,216,80,59,223,213,159,179,147,169,93,90,173,245,198,253,2,94,166,23,111,8,115,27,215,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

FILE *fptr;
    
/* inits decoding of 'file': opens it, fills struct fmt_info  */
int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;

    if(!finfo->image)
	return SQERR_NOMEMORY;

    fptr = fopen(file, "rb");

    if(!fptr)
	return SQERR_NOFILE;
	
    currentImage = -1;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    char	color_[30];
    int		bit_depth, interlace_type;
    
    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    if(!finfo->image)
	return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));
	
    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, my_error_exit, 0)) == NULL)
    {
	fclose(fptr);
	return SQERR_NOMEMORY;
    }

    if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
	fclose(fptr);
	png_destroy_read_struct(&png_ptr, 0, 0);
	return SQERR_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
//	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
//	fclose(fptr);
	return SQERR_BADFILE;
    }

    png_init_io(png_ptr, fptr);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)NULL, (int*)NULL);

    finfo->image[currentImage].w = width;
    finfo->image[currentImage].h = height;
    finfo->image[currentImage].bpp = bit_depth;

    if(finfo->image[currentImage].bpp == 16)
	png_set_strip_16(png_ptr);

    if(finfo->image[currentImage].bpp < 8)
	png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && finfo->image[currentImage].bpp < 8)
	png_set_gray_1_2_4_to_8(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
	png_set_palette_to_rgb(png_ptr);
    }
	
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb(png_ptr);
	
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(png_ptr);

    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    number_passes = png_set_interlace_handling(png_ptr);
    
    finfo->image[currentImage].interlaced = number_passes > 1;
    finfo->image[currentImage].passes = number_passes;

    png_read_update_info(png_ptr, info_ptr);

    switch(color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    finfo->image[currentImage].hasalpha = true;
    	    strcpy(color_, "Grayscale with ALPHA");
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    finfo->image[currentImage].hasalpha = true;
	    strcpy(color_, "RGB with ALPHA");
	break;
	
	case PNG_COLOR_TYPE_RGB:
	    strcpy(color_, "RGB");
	break;
	
	case PNG_COLOR_TYPE_PALETTE:
	    strcpy(color_, "Color indexed");
	break;

	case PNG_COLOR_TYPE_GRAY:
	    strcpy(color_, "Grayscale");
	break;
	
	default:
	    strcpy(color_, "Unknown");
    }

    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    rows = (png_bytep)png_malloc(png_ptr, row_bytes);
    
    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\nDeflate method 8, 32K window\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	color_,
	bytes);

#if defined(PNG_TEXT_SUPPORTED)
    finfo->image[currentImage].meta = (fmt_metainfo *)calloc(1, sizeof(fmt_metainfo));

    if(finfo->image[currentImage].meta)
    {
	png_textp lines = info_ptr->text;

	if(!lines || !info_ptr->num_text)
	{
	    finfo->images++;
	    free(finfo->image[currentImage].meta);
	    finfo->image[currentImage].meta = 0;
	    return SQERR_OK;
	}

        finfo->image[currentImage].meta->m = (fmt_meta_entry *)calloc(info_ptr->num_text, sizeof(fmt_meta_entry));
        finfo->image[currentImage].meta->entries = info_ptr->num_text;
        fmt_meta_entry *entry = finfo->image[currentImage].meta->m;

        for(int i = 0;i < info_ptr->num_text;i++)
        {
            if(entry)
            {
                entry[i].datalen = lines[i].text_length;
                sprintf(entry[i].group, "PNG key [%s]", lines[i].key);
                entry[i].data = (char *)malloc(entry[i].datalen);

                if(entry[i].data)
                    memcpy(entry[i].data, lines[i].text, entry[i].datalen);
            }
        }
    }

#endif

    finfo->images++;

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    png_read_rows(png_ptr, &rows, png_bytepp_NULL, 1);

    memcpy(scan, rows, finfo->image[currentImage].w * sizeof(RGBA));

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    int w, h, bpp, hasalpha = 0;

    FILE *m_fptr = fopen(file, "rb");

    if(!m_fptr)
	return SQERR_NOFILE;

    png_structp	m_png_ptr;
    png_infop	m_info_ptr;
    png_uint_32	m_width, m_height, m_number_passes;
    int		m_color_type;
    char	m_color_[30];
    int		m_bit_depth, m_interlace_type;
    int i;

    if((m_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
    {
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    if((m_info_ptr = png_create_info_struct(m_png_ptr)) == NULL)
    {
	fclose(m_fptr);
	png_destroy_read_struct(&m_png_ptr, 0, 0);
	return SQERR_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(m_png_ptr)))
    {
	png_destroy_read_struct(&m_png_ptr, &m_info_ptr, 0);
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    png_init_io(m_png_ptr, m_fptr);
    png_read_info(m_png_ptr, m_info_ptr);
    png_get_IHDR(m_png_ptr, m_info_ptr, &m_width, &m_height, &m_bit_depth, &m_color_type, &m_interlace_type, (int*)NULL, (int*)NULL);

    w = m_width;
    h = m_height;
    bpp = m_bit_depth;

    if(bpp == 16)
	png_set_strip_16(m_png_ptr);

    if(bpp < 8)
	png_set_packing(m_png_ptr);

    if(m_color_type == PNG_COLOR_TYPE_GRAY && bpp < 8)
	png_set_gray_1_2_4_to_8(m_png_ptr);

    if(m_color_type == PNG_COLOR_TYPE_PALETTE)
    {
	png_set_palette_to_rgb(m_png_ptr);
    }

    if(m_color_type == PNG_COLOR_TYPE_GRAY || m_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb(m_png_ptr);
	
    switch(m_color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    hasalpha = true;
    	    strcpy(m_color_, "Grayscale with ALPHA");
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    hasalpha = true;
	    strcpy(m_color_, "RGBA");
	break;

	case PNG_COLOR_TYPE_RGB:
	    strcpy(m_color_, "RGB");
	break;
	
	case PNG_COLOR_TYPE_PALETTE:
	    strcpy(m_color_, "Color indexed");
	break;

	case PNG_COLOR_TYPE_GRAY:
	    strcpy(m_color_, "Grayscale");
	break;
	
	default:
	    strcpy(m_color_, "Unknown");
    }
    
    if(png_get_valid(m_png_ptr, m_info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(m_png_ptr);

    png_set_filler(m_png_ptr, 0xff, PNG_FILLER_AFTER);

    m_number_passes = png_set_interlace_handling(m_png_ptr);

    png_read_update_info(m_png_ptr, m_info_ptr);
/*    
    row_pointers = (png_bytep*)calloc(finfo->h, sizeof(png_bytep));

    for(row = 0; row < finfo->h; row++)
	row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

    row = 0;
	
    png_read_image(png_ptr, row_pointers);
*/

    const int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\nDeflate method 8, 32K window\n%d\n%d\n",
    fmt_quickinfo(),
    w, h,
    bpp, m_color_,
    1,
    m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);
    
    if(!*image)
    {
	fprintf(stderr, "libSQ_read_png: Image is null!\n");
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    png_bytep m_rows;

    int m_row_bytes = png_get_rowbytes(m_png_ptr, m_info_ptr);
    m_rows = (png_bytep)png_malloc(m_png_ptr, m_row_bytes);
    
    const int sss = w * sizeof(RGBA);

    for(unsigned int m_pass = 0;m_pass < m_number_passes;m_pass++)
    for(i = 0;i < h;i++)
    {
	png_read_rows(m_png_ptr, &m_rows, png_bytepp_NULL, 1);
	memcpy(*image + i*w, m_rows, sss);
    }
    
    png_free(m_png_ptr, m_rows);
    png_read_end(m_png_ptr, m_info_ptr);
    png_destroy_read_struct(&m_png_ptr, &m_info_ptr, png_infopp_NULL);
    fclose(m_fptr);

    return SQERR_OK;
}

int fmt_close()
{
//    for(row = 0; row < finfo->h; row++)
//	png_free(png_ptr, row_pointers[row]);

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    png_free(png_ptr, rows);

    fclose(fptr);

    return SQERR_OK;
}

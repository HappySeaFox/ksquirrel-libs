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
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <sstream>
#include <iostream>

#include <png.h>

#include "fmt_types.h"
#include "fmt_codec_png_defs.h"
#include "fmt_codec_png.h"

#include "error.h"

/*
 *
 * PNG (pronounced "ping") is a bitmap file format used to transmit and
 * store bitmapped images.  PNG supports the capability of storing up to
 * 16 bits (gray-scale) or 48 bits (truecolor) per pixel, and up to 16 bits
 * of alpha data. It handles the progressive display
 * of image data and the storage of gamma,
 * transparency and textual information, and it uses an efficient and
 * lossless form of data compression.
 *
 */

#ifndef png_jmpbuf
#define png_jmpbuf(p) ((p)->jmpbuf)
#endif

bool zerror;

void my_error_exit(png_struct *, const char *mes)
{
    cerr << "libSQ_read_png: " << mes << endl;
    zerror = true;
}

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.1.3");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Portable Network Graphics");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.png ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("\x0089\x0050\x004E\x0047\x000D\x000A\x001A\x000A");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,128,0,0,4,4,4,247,152,75,49,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,61,142,193,21,128,48,8,67,179,2,7,22,136,108,96,39,64,6,208,67,247,95,69,74,209,156,254,11,144,0,96,46,33,245,136,136,22,144,212,217,144,214,217,194,136,210,133,193,48,15,38,152,209,189,157,164,229,228,200,227,216,80,59,223,213,159,179,147,169,93,90,173,245,198,253,2,94,166,23,111,8,115,27,215,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQERR_NOFILE;
	
    currentImage = -1;
    
    zerror = false;

    rows = 0L;

    finfo.animated = false;
    finfo.images = 0;
	    
    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    s32		bit_depth, interlace_type;

    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, my_error_exit, 0)) == NULL)
    {
	zerror = true;
	return SQERR_NOMEMORY;
    }

    if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
	zerror = true;
	return SQERR_NOMEMORY;
    }
    
    if(setjmp(png_jmpbuf(png_ptr)))
    {
	zerror = true;
	return SQERR_BADFILE;
    }

    png_init_io(png_ptr, fptr);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)NULL, (int*)NULL);

    finfo.image[currentImage].w = width;
    finfo.image[currentImage].h = height;
    finfo.image[currentImage].bpp = bit_depth;

    if(finfo.image[currentImage].bpp == 16)
	png_set_strip_16(png_ptr);

    if(finfo.image[currentImage].bpp < 8)
	png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && finfo.image[currentImage].bpp < 8)
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

    finfo.image[currentImage].interlaced = number_passes > 1;
    finfo.image[currentImage].passes = number_passes;

    png_read_update_info(png_ptr, info_ptr);

    rows = (png_bytep*)calloc(finfo.image[currentImage].h, sizeof(png_bytep));
    
    if(!rows)
	return SQERR_NOMEMORY;

    for(s32 row = 0; row < finfo.image[currentImage].h; row++)
    {
	rows[row] = (png_bytep)0;
    }

    for(s32 row = 0; row < finfo.image[currentImage].h; row++)
    {
	rows[row] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));

	if(!rows[row])
	    return SQERR_NOMEMORY;
    }

    stringstream color_;

    switch(color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    finfo.image[currentImage].hasalpha = true;
    	    color_ <<  "Grayscale with ALPHA";
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    finfo.image[currentImage].hasalpha = true;
	    color_ <<  "RGB with ALPHA";
	break;
	
	case PNG_COLOR_TYPE_RGB:
	    color_ <<  "RGB";
	break;
	
	case PNG_COLOR_TYPE_PALETTE:
	    color_ <<  "Color indexed";
	break;

	case PNG_COLOR_TYPE_GRAY:
	    color_ <<  "Grayscale";
	break;
	
	default:
	    color_ <<  "Unknown";
    }

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
        << color_.str() << "\n"
        << "Deflate method 8, 32K window\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

#if defined(PNG_TEXT_SUPPORTED)
    png_textp lines = info_ptr->text;

    if(!lines || !info_ptr->num_text)
    {
        finfo.images++;
        return SQERR_OK;
    }

    for(s32 i = 0;i < info_ptr->num_text;i++)
        finfo.meta.push_back(fmt_metaentry());

    for(s32 i = 0;i < info_ptr->num_text;i++)
    {
        stringstream key;
	key << "PNG key [" << lines[i].key << "]";
        finfo.meta[i].data = lines[i].text;
    }
#endif

    finfo.images++;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    line = -1;

    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;

    png_read_rows(png_ptr, &rows[line], NULL, 1);

    memcpy(scan, rows[line], finfo.image[currentImage].w * sizeof(RGBA));
    
    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 w, h, bpp, hasalpha = 0;
    s32 m_bytes;

    FILE *m_fptr = fopen(file.c_str(), "rb");

    if(!m_fptr)
	return SQERR_NOFILE;

    png_structp	m_png_ptr;
    png_infop	m_info_ptr;
    png_uint_32	m_width, m_height, m_number_passes;
    s32		m_color_type;
    s32		m_bit_depth, m_interlace_type;
    s32 	i;

    if((m_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
    {
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    if((m_info_ptr = png_create_info_struct(m_png_ptr)) == NULL)
    {
	png_destroy_read_struct(&m_png_ptr, 0, 0);
	fclose(m_fptr);
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

    stringstream color_;
	
    switch(m_color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    hasalpha = true;
    	    color_ << "Grayscale with ALPHA";
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    hasalpha = true;
	    color_ << "RGBA";
	break;

	case PNG_COLOR_TYPE_RGB:
	    color_ << "RGB";
	break;
	
	case PNG_COLOR_TYPE_PALETTE:
	    color_ << "Color indexed";
	break;

	case PNG_COLOR_TYPE_GRAY:
	    color_ << "Grayscale";
	break;
	
	default:
	    color_ << "Unknown";
    }

    if(png_get_valid(m_png_ptr, m_info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(m_png_ptr);

    png_set_filler(m_png_ptr, 0xff, PNG_FILLER_AFTER);

    m_number_passes = png_set_interlace_handling(m_png_ptr);

    png_read_update_info(m_png_ptr, m_info_ptr);

    png_bytep m_rows[h];

    for(s32 row = 0; row < h; row++)
    {
	m_rows[row] = (png_bytep)malloc(png_get_rowbytes(m_png_ptr, m_info_ptr));
	
	if(!m_rows[row])
	{
	    for(s32 s = 0;s < row;s++)
		free(m_rows[s]);
		
	    png_read_end(m_png_ptr, m_info_ptr);
	    png_destroy_read_struct(&m_png_ptr, &m_info_ptr, png_infopp_NULL);
	    fclose(m_fptr);

	    return SQERR_NOMEMORY;
	}
    }

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
	<< h << "\n"
        << bpp << "\n"
        << color_.str() << "\n"
        << "Deflate method 8, 32K window" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);
    
    if(!*image)
    {

	png_read_end(m_png_ptr, m_info_ptr);
	png_destroy_read_struct(&m_png_ptr, &m_info_ptr, png_infopp_NULL);
	fclose(m_fptr);

	return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    const s32 sss = w * sizeof(RGBA);

    s32 m_line;

    for(u32 m_pass = 0;m_pass < m_number_passes;m_pass++)
    {
	m_line = 0;

	for(i = 0;i < h;i++)
	{
	    png_read_rows(m_png_ptr, &m_rows[m_line], NULL, 1);

	    if(m_pass == m_number_passes-1)
		memcpy(*image + i*w, m_rows[m_line], sss);

	    m_line++;
	}
    }

    png_read_end(m_png_ptr, m_info_ptr);
    png_destroy_read_struct(&m_png_ptr, &m_info_ptr, png_infopp_NULL);
    fclose(m_fptr);

    for(s32 row = 0; row < h; row++)
	free(m_rows[row]);
	
    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    if(!zerror)
	png_read_end(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fclose(fptr);

    if(rows)
    {
	for(u32 row = 0; row < height; row++)
	{
	    if(rows[row])
		free(rows[row]);
	}

	free(rows);
    }

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = true;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 1;
    opt->compression_max = 9;
    opt->compression_def = 7;
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &opt)
{
    FILE 	*m_fptr;
    png_structp	m_png_ptr;
    png_infop	m_info_ptr;
    s32		bpp = 8;

    if(!image || !w || !h)
	return SQERR_NOMEMORY;

    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
	return SQERR_NOFILE;

    m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, my_error_exit, NULL);

    if(!m_png_ptr)
    {
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    m_info_ptr = png_create_info_struct(m_png_ptr);

    if(!m_info_ptr)
    {
	png_destroy_write_struct(&m_png_ptr, png_infopp_NULL);
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(m_png_ptr)))
    {
	png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
	fclose(m_fptr);
	return SQERR_BADFILE;
    }

    png_init_io(m_png_ptr, m_fptr);

    png_set_IHDR(m_png_ptr, m_info_ptr, w, h, bpp, PNG_COLOR_TYPE_RGB_ALPHA,
	((opt.interlaced) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE), 
	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_color_8 sig_bit;

    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    
    png_set_sBIT(m_png_ptr, m_info_ptr, &sig_bit);

    s32 factor = (opt.compression_level < 1 || opt.compression_level > 9) ? 1 : opt.compression_level;

    png_set_compression_level(m_png_ptr, factor);

//    png_set_gAMA(m_png_ptr, m_info_ptr, 1.2);

    png_write_info(m_png_ptr, m_info_ptr);

    png_set_shift(m_png_ptr, &sig_bit);

    png_set_swap(m_png_ptr);

    png_set_packswap(m_png_ptr);

    s32 number_passes;

    number_passes = (opt.interlaced) ? png_set_interlace_handling(m_png_ptr) : 1;

    png_bytep row_pointers[h];

    for(s32 k = 0;k < h;k++)
	row_pointers[k] = (png_bytep)(image + k * w);

    for(s32 pass = 0; pass < number_passes; pass++)
    {
	for (s32 y = 0; y < h; y++)
	{
	    png_write_rows(m_png_ptr, &row_pointers[y], 1);
	}
    }

    png_write_end(m_png_ptr, m_info_ptr);

    png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
    
    fclose(m_fptr);

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

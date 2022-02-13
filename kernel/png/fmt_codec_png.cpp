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

#include <iostream>

#include <png.h>

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_png_defs.h"
#include "fmt_codec_png.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,128,0,0,76,76,76,56,9,211,35,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,92,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,161,51,35,167,206,12,141,96,88,58,51,50,116,234,84,168,8,144,5,18,1,74,77,157,9,101,128,212,192,117,193,205,1,89,10,54,153,11,236,142,5,12,0,55,194,57,126,77,105,246,125,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQE_R_NOFILE;
	
    currentImage = -1;
    
    zerror = false;

    rows = 0L;

    finfo.animated = false;
    finfo.images = 0;
	    
    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    s32		bit_depth, interlace_type;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, my_error_exit, 0)) == NULL)
    {
	zerror = true;
	return SQE_R_NOMEMORY;
    }

    if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
	zerror = true;
	return SQE_R_NOMEMORY;
    }
    
    if(setjmp(png_jmpbuf(png_ptr)))
    {
	zerror = true;
	return SQE_R_BADFILE;
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
	return SQE_R_NOMEMORY;

    for(s32 row = 0; row < finfo.image[currentImage].h; row++)
    {
	rows[row] = (png_bytep)0;
    }

    for(s32 row = 0; row < finfo.image[currentImage].h; row++)
    {
	rows[row] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));

	if(!rows[row])
	    return SQE_R_NOMEMORY;
    }

    std::string color_;

    switch(color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    finfo.image[currentImage].hasalpha = true;
    	    color_ = "Grayscale with ALPHA";
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    finfo.image[currentImage].hasalpha = true;
	    color_ = "RGBA";
	break;
	
	case PNG_COLOR_TYPE_RGB:
	    color_ = "RGB";
	break;
	
	case PNG_COLOR_TYPE_PALETTE:
	    color_ = "Color indexed";
	break;

	case PNG_COLOR_TYPE_GRAY:
	    color_ = "Grayscale";
	break;
	
	default:
	    color_ = "Unknown";
    }

    finfo.image[currentImage].compression = "Deflate method 8, 32K window";
    finfo.image[currentImage].colorspace = color_;

#if defined(PNG_TEXT_SUPPORTED)
    png_textp lines = info_ptr->text;

    if(!lines || !info_ptr->num_text)
    {
        finfo.images++;
        return SQE_OK;
    }

    for(s32 i = 0;i < info_ptr->num_text;i++)
        finfo.meta.push_back(fmt_metaentry());

    for(s32 i = 0;i < info_ptr->num_text;i++)
    {
        std::string key;
	key = key + "PNG key [" + lines[i].key + "]";
        finfo.meta[i].group = key;
        finfo.meta[i].data = lines[i].text;
    }
#endif

    finfo.images++;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;

    png_read_rows(png_ptr, &rows[line], NULL, 1);

    memcpy(scan, rows[line], finfo.image[currentImage].w * sizeof(RGBA));
    
    return SQE_OK;
}

void fmt_codec::fmt_read_close()
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
    opt->passes = 8;
    opt->needflip = false;
}

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    if(!image.w || !image.h || file.empty())
	return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
	return SQE_W_NOFILE;

    m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, my_error_exit, NULL);

    if(!m_png_ptr)
    {
	fclose(m_fptr);
	return SQE_W_NOMEMORY;
    }

    m_info_ptr = png_create_info_struct(m_png_ptr);

    if(!m_info_ptr)
    {
	png_destroy_write_struct(&m_png_ptr, png_infopp_NULL);
	fclose(m_fptr);
	return SQE_W_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(m_png_ptr)))
    {
	png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
	fclose(m_fptr);
	return SQE_W_ERROR;
    }

    png_init_io(m_png_ptr, m_fptr);

    png_set_IHDR(m_png_ptr, m_info_ptr, writeimage.w, writeimage.h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	((writeopt.interlaced) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE), 
	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_color_8 sig_bit;

    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    
    png_set_sBIT(m_png_ptr, m_info_ptr, &sig_bit);

    s32 factor = (writeopt.compression_level < 1 || writeopt.compression_level > 9) ? 1 : writeopt.compression_level;

    png_set_compression_level(m_png_ptr, factor);

//    png_set_gAMA(m_png_ptr, m_info_ptr, 1.2);

    png_write_info(m_png_ptr, m_info_ptr);

    png_set_shift(m_png_ptr, &sig_bit);

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next()
{
    png_set_swap(m_png_ptr);

    png_set_packswap(m_png_ptr);

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    png_set_interlace_handling(m_png_ptr);

    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    m_row_pointer = (png_bytep)scan;

    png_write_rows(m_png_ptr, &m_row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    png_write_end(m_png_ptr, m_info_ptr);

    png_destroy_write_struct(&m_png_ptr, &m_info_ptr);

    fclose(m_fptr);
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

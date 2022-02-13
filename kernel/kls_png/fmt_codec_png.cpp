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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_png_defs.h"
#include "fmt_codec_png.h"

#include "../xpm/codec_png.xpm"

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

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "1.1.3";
    o->name = "Portable Network Graphics";
    o->filter = "*.png ";
    o->config = "";
    o->mime = "\x0089\x0050\x004E\x0047\x000D\x000A\x001A\x000A";
    o->mimetype = "image/png";
    o->pixmap = codec_png;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = true;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    png_ptr = 0;
    info_ptr = 0;

    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQE_R_NOFILE;

    currentImage = -1;
    zerror = false;
    rows = 0;
    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    s32	bit_depth, interlace_type;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
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
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)0, (int*)0);

    image.w = width;
    image.h = height;
    image.bpp = bit_depth;

    if(image.bpp == 16)
	png_set_strip_16(png_ptr);

    if(image.bpp < 8)
	png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && image.bpp < 8)
	png_set_gray_1_2_4_to_8(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
	png_set_palette_to_rgb(png_ptr);
	
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb(png_ptr);
	
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(png_ptr);

    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    number_passes = png_set_interlace_handling(png_ptr);

    image.interlaced = number_passes > 1;
    image.passes = number_passes;

    png_read_update_info(png_ptr, info_ptr);

    rows = (png_bytep*)malloc(image.h * sizeof(png_bytep*));

    if(!rows)
	return SQE_R_NOMEMORY;

    for(s32 row = 0; row < image.h; row++)
    {
	rows[row] = 0;
    }

    for(s32 row = 0; row < image.h; row++)
    {
	rows[row] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));

	if(!rows[row])
	    return SQE_R_NOMEMORY;
    }

    std::string color_;

    switch(color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
    	    image.hasalpha = true;
    	    color_ = "Grayscale with ALPHA";
	break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    image.hasalpha = true;
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

    image.compression = "Deflate method 8, 32K window";
    image.colorspace = color_;

#if defined(PNG_TEXT_SUPPORTED)
    png_textp lines = info_ptr->text;

    if(!lines || !info_ptr->num_text)
    {
   	finfo.image.push_back(image);
        return SQE_OK;
    }

    for(s32 i = 0;i < info_ptr->num_text;i++)
    {
	fmt_metaentry mt;

        mt.group = lines[i].key;
        mt.data = lines[i].text;

        addmeta(mt);
    }
#endif

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(zerror || setjmp(png_jmpbuf(png_ptr)))
    {
        zerror = true;
        return SQE_R_BADFILE;
    }

    line++;

    png_read_rows(png_ptr, &rows[line], png_bytepp_NULL, 1);
    memcpy(scan, rows[line], im->w * sizeof(RGBA));

    return SQE_OK;
}

void fmt_codec::read_close()
{
    if(png_ptr)
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    if(fptr)
        fclose(fptr);

    if(rows)
    {
        for(u32 i = 0;i < height;i++)
        {
            if(rows[i])
                free(rows[i]);
        }

        free(rows);
    }

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = true;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 1;
    opt->compression_max = 9;
    opt->compression_def = 7;
    opt->passes = 8;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
}

s32 fmt_codec::write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    m_png_ptr = 0;
    m_info_ptr = 0;
    zerror = false;

    if(!image.w || !image.h || file.empty())
	return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
	return SQE_W_NOFILE;

    m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if(!m_png_ptr)
    {
	zerror = true;
	return SQE_W_NOMEMORY;
    }

    m_info_ptr = png_create_info_struct(m_png_ptr);

    if(!m_info_ptr)
    {
	zerror = true;
	return SQE_W_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(m_png_ptr)))
    {
        zerror = true;
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

    png_write_info(m_png_ptr, m_info_ptr);

    png_set_shift(m_png_ptr, &sig_bit);

    return SQE_OK;
}

s32 fmt_codec::write_next()
{
    png_set_swap(m_png_ptr);

    png_set_packswap(m_png_ptr);

    png_set_interlace_handling(m_png_ptr);

    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    if(zerror || setjmp(png_jmpbuf(m_png_ptr)))
    {
        zerror = true;
	return SQE_W_ERROR;
    }

    m_row_pointer = (png_bytep)scan;

    png_write_rows(m_png_ptr, &m_row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::write_close()
{
    if(m_png_ptr) png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
    if(m_fptr)    fclose(m_fptr);
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("png");
}

#include "fmt_codec_cd_func.h"

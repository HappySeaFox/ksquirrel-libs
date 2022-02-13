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

#include "ksquirrel-libs-png/png.h"

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_png_defs.h"
#include "fmt_codec_png.h"

#if defined CODEC_SVG || defined CODEC_DICOM
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>
#endif

#ifdef CODEC_SVG
#include "../xpm/codec_svg.xpm"
#elif defined CODEC_DICOM
#include "../xpm/codec_dicom.xpm"
#else
#include "../xpm/codec_png.xpm"
#endif

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

inline bool MALLOC_ROWS(png_bytep **A, const int RB, const int H)
{
    *A = (png_bytep*)malloc(H * sizeof(png_bytep*));

    if(!*A)
        return false;

    for(s32 row = 0; row < H; row++)
        (*A)[row] = 0;

    for(s32 row = 0; row < (s32)H; row++)
    {
        (*A)[row] = (png_bytep)malloc(RB);

        if(!(*A)[row])
            return false;

        memset((*A)[row], 0, RB);
    }

    return true;
}

inline void FREE_ROWS(png_bytep **A, const int H)
{
    if(*A)
    {
        for(s32 i = 0;i < H;i++)
        {
            if((*A)[i])
                free((*A)[i]);
        }

        free(*A);
        *A = 0;
    }
}

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
#ifdef CODEC_SVG
    o->version = "0.1.2";
    o->name = "Scalable Vector Graphics";
    o->filter = "*.svg *.svgz ";
    o->config = std::string(SVG_UI); // SVG_UI comes from Makefile.am
    o->mime = "";
    o->mimetype = "image/svg+xml";
    o->pixmap = codec_svg;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_DICOM
    o->version = "1.1.3";
    o->name = "DICOM";
    o->filter = "*.dcm ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-dicom";
    o->pixmap = codec_dicom;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#else
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
#endif
}

#ifdef CODEC_ANOTHER
void fmt_codec::fill_default_settings()
{
    settings_value val;

    // scale factor in percents
    val.type = settings_value::v_int;
    val.iVal = 1;

    m_settings["scale"] = val;
}
#endif

s32 fmt_codec::read_init(const std::string &file)
{
    png_ptr = 0;
    info_ptr = 0;
    fptr = 0;
    frame = 0;
    prev = 0;
    cur = 0;

#ifdef CODEC_SVG
    int status;

    fmt_settings::iterator it = m_settings.find("scale");

    // percents / 100
    int scale = (it == m_settings.end() || (*it).second.type != settings_value::v_int)
             ? 1 : (*it).second.iVal;

    if(scale < 1 || scale > 10)
        scale = 1;

    char z[32];
    snprintf(z, 32, "%d", scale);

    pid_t pid = fork();

    if(!pid)
    {
        execlp(SVG2PNG, SVG2PNG, "--binary", RSVG, "--input", file.c_str(), "--output", tmp.c_str(), "-z", z, (char *)0);
        exit(1);
    }
    else if(pid == -1)
        return SQE_R_BADFILE;

    ::waitpid(pid, &status, 0);

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#elif defined CODEC_DICOM

    int status;

    pid_t pid = fork();

    if(!pid)
    {
        execlp(DICOM, DICOM, file.c_str(), tmp.c_str(), (char *)0);
        exit(1);
    }
    else if(pid == -1)
        return SQE_R_BADFILE;

    ::waitpid(pid, &status, 0);

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#else
    fptr = fopen(file.c_str(), "rb");
#endif

    if(!fptr)
	return SQE_R_NOFILE;

    currentImage = -1;
    zerror = false;

    if((png_ptr = my_png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
    {
	zerror = true;
	return SQE_R_NOMEMORY;
    }

    if((info_ptr = my_png_create_info_struct(png_ptr)) == NULL)
    {
	zerror = true;
	return SQE_R_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        zerror = true;
        return SQE_R_BADFILE;
    }

    my_png_init_io(png_ptr, fptr);
    my_png_read_info(png_ptr, info_ptr);
    my_png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)0, (int*)0);

    img.w = next_frame_width = width;
    img.h = next_frame_height = height;
    img.bpp = bit_depth;
    img.hasalpha = true;

    if(img.bpp == 16)
	my_png_set_strip_16(png_ptr);

    if(img.bpp < 8)
	my_png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && img.bpp < 8)
	my_png_set_gray_1_2_4_to_8(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
	my_png_set_palette_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	my_png_set_gray_to_rgb(png_ptr);

    if(my_png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	my_png_set_tRNS_to_alpha(png_ptr);

    my_png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    number_passes = my_png_set_interlace_handling(png_ptr);

    my_png_read_update_info(png_ptr, info_ptr);

    finfo.animated = !!my_png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL);

    frames = finfo.animated ? my_png_get_num_frames(png_ptr, info_ptr) : 1;

    if(!frames) return SQE_R_BADFILE;

    img.interlaced = number_passes > 1;
    img.passes = finfo.animated ? 1 : number_passes;

    if(finfo.animated)
    {
        if(!MALLOC_ROWS(&prev, width * sizeof(RGBA), height))
            return SQE_R_NOMEMORY;

        if(!MALLOC_ROWS(&cur, width * sizeof(RGBA), height))
            return SQE_R_NOMEMORY;
    }

    std::string color_;

    switch(color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:  color_ = "Grayscale with ALPHA"; break;
	case PNG_COLOR_TYPE_RGB_ALPHA:   color_ = "RGBA";                 break;
	case PNG_COLOR_TYPE_RGB:         color_ = "RGB";                  break;
	case PNG_COLOR_TYPE_PALETTE:     color_ = "Color indexed";        break;
	case PNG_COLOR_TYPE_GRAY:        color_ = "Grayscale";            break;

	default:
	    color_ = "Unknown";
    }

    img.compression = "Deflate method 8, 32K window";
    img.colorspace = color_;
    if(!finfo.animated) img.delay = 0;

#ifdef PNG_TEXT_SUPPORTED
    png_textp lines = info_ptr->text;

    if(!lines || !info_ptr->num_text)
        return SQE_OK;

    for(s32 i = 0;i < info_ptr->num_text;i++)
    {
	fmt_metaentry mt;

        mt.group = lines[i].key;
        mt.data = lines[i].text;

        addmeta(mt);
    }
#endif

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage == frames)
	return SQE_NOTOK;

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        zerror = true;
        return SQE_R_BADFILE;
    }

    if(finfo.animated)
    {
        if(currentImage)
        {
            if(next_frame_dispose_op == PNG_DISPOSE_OP_BACKGROUND)
            {
                for(u32 j = next_frame_y_offset,i = 0;i < next_frame_height;j++,i++)
                    memset(cur[j]+next_frame_x_offset*sizeof(RGBA), 0, next_frame_width * sizeof(RGBA));
            }
            else if(next_frame_dispose_op == PNG_DISPOSE_OP_PREVIOUS)
            {
                for(u32 i = 0;i < height;i++)
                    memcpy(cur[i], prev[i], width*sizeof(RGBA));
            }
            else // next_frame_dispose_op == PNG_DISPOSE_OP_NONE
            {
            }

            for(u32 i = 0;i < height;i++)
                memcpy(prev[i], cur[i], width*sizeof(RGBA));
/*
        char nn[128];
        int bpp = 32;
        sprintf(nn, "/home/krasu/apng/dump-%02d.rawrgb", currentImage);
        FILE *fp = fopen(nn, "wb");
        fwrite(&next_frame_width, 4, 1, fp);
        fwrite(&next_frame_height, 4, 1, fp);
        fwrite(&bpp, 4, 1, fp);
        for(int i = 0;i < height;i++)
            fwrite(prev[i], 4, width, fp);
        fclose(fp);
*/
        }
        else if(my_png_get_first_frame_is_hidden(png_ptr, info_ptr))
        {
            if(!MALLOC_ROWS(&frame, width * sizeof(RGBA), height))
                return SQE_R_NOMEMORY;

            my_png_read_frame_head(png_ptr, info_ptr);
            my_png_read_image(png_ptr, frame);

            FREE_ROWS(&frame, height);

            frames--;

            if(frames == 1)
            {
                my_png_read_frame_head(png_ptr, info_ptr);
                finfo.animated = false;
                img.passes = number_passes;
                finfo.image.push_back(img);
                return SQE_OK;
            }
            else if(!frames)
                return SQE_R_BADFILE; // oops? 
        }

        FREE_ROWS(&frame, next_frame_height);

        my_png_read_frame_head(png_ptr, info_ptr);

        if(my_png_get_valid(png_ptr, info_ptr, PNG_INFO_fcTL))
        {
            my_png_get_next_frame_fcTL(png_ptr, info_ptr,
                                    &next_frame_width, &next_frame_height,
                                    &next_frame_x_offset, &next_frame_y_offset,
                                    &next_frame_delay_num, &next_frame_delay_den,
                                    &next_frame_dispose_op, &next_frame_blend_op);
        }
        else
        {
            next_frame_width = width;
            next_frame_height = height;
            next_frame_x_offset = 0;
            next_frame_y_offset = 0;
            next_frame_dispose_op = PNG_DISPOSE_OP_BACKGROUND;
            next_frame_blend_op   = PNG_BLEND_OP_SOURCE;
        }

        if(!next_frame_delay_den) next_frame_delay_den = 100;

        img.delay = (s32)(((double)next_frame_delay_num / next_frame_delay_den) * 1000);

        if(next_frame_width + next_frame_x_offset > width || next_frame_height + next_frame_y_offset > height)
            return SQE_R_BADFILE;

        if(!MALLOC_ROWS(&frame, next_frame_width * sizeof(RGBA), next_frame_height))
            return SQE_R_NOMEMORY;

        my_png_read_image(png_ptr, frame);

        // copy all pixel values including alpha
        if(!currentImage || next_frame_blend_op == PNG_BLEND_OP_SOURCE)
        {
            for(u32 j = next_frame_y_offset,i = 0;i < next_frame_height;j++,i++)
                memcpy(cur[j]+next_frame_x_offset*sizeof(RGBA), frame[i], next_frame_width * sizeof(RGBA));
        }
        else // over
        {
            RGBA *src, *dst;

            for(u32 j = next_frame_y_offset,i = 0;i < next_frame_height;j++,i++)
            {
                src = (RGBA *)frame[i];
                dst = (RGBA *)(cur[j]+next_frame_x_offset*sizeof(RGBA));
                u32 k = next_frame_width;

                while(k--)
                {
                    // fully transparent foreground
                    if(src->a == 0)
                        ;
                    else if(src->a == 255 || dst->a == 0)
                        *dst = *src;
                    else // composite
                    {
                        dst->r = ((src->a * (src->r - dst->r))>>8) + dst->r;
                        dst->g = ((src->a * (src->g - dst->g))>>8) + dst->g;
                        dst->b = ((src->a * (src->b - dst->b))>>8) + dst->b;
                        //dst->a = ((src->a * (src->a - dst->a))>>8) + dst->a;
                    }

                    src++;
                    dst++;
                }
            }
        }
    }

    finfo.image.push_back(img);

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

    line++;

    if(finfo.animated)
        memcpy(scan, cur[line], im->w * sizeof(RGBA));
    else
        my_png_read_row(png_ptr, (png_bytep)scan, png_bytep_NULL);

    return SQE_OK;
}

void fmt_codec::read_close()
{
    if(png_ptr) my_png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    if(fptr) fclose(fptr);

    FREE_ROWS(&frame, next_frame_height);
    FREE_ROWS(&prev, height);
    FREE_ROWS(&cur, height);

    finfo.meta.clear();
    finfo.image.clear();
}

#ifdef CODEC_PNG

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

    m_png_ptr = my_png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if(!m_png_ptr)
    {
	zerror = true;
	return SQE_W_NOMEMORY;
    }

    m_info_ptr = my_png_create_info_struct(m_png_ptr);

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

    my_png_init_io(m_png_ptr, m_fptr);

    my_png_set_IHDR(m_png_ptr, m_info_ptr, writeimage.w, writeimage.h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	((writeopt.interlaced) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE), 
	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_color_8 sig_bit;

    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    
    my_png_set_sBIT(m_png_ptr, m_info_ptr, &sig_bit);

    s32 factor = (writeopt.compression_level < 1 || writeopt.compression_level > 9) ? 1 : writeopt.compression_level;

    my_png_set_compression_level(m_png_ptr, factor);

    my_png_write_info(m_png_ptr, m_info_ptr);

    my_png_set_shift(m_png_ptr, &sig_bit);

    return SQE_OK;
}

s32 fmt_codec::write_next()
{
    my_png_set_swap(m_png_ptr);

    my_png_set_packswap(m_png_ptr);

    my_png_set_interlace_handling(m_png_ptr);

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

    my_png_write_rows(m_png_ptr, &m_row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::write_close()
{
    if(m_png_ptr) my_png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
    if(m_fptr)    fclose(m_fptr);
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("png");
}

#endif

#include "fmt_codec_cd_func.h"

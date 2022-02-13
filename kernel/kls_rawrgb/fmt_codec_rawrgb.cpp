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

#include <iostream>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_rawrgb_defs.h"
#include "fmt_codec_rawrgb.h"

#include "../xpm/codec_rawrgb.xpm"

/*
 *
 * This is a codec to read and write internal raw image format.
 * This format has a simple header folowed by uncompressed image data in
 * 24 or 32 bit format. Width, height and bit depth are integers (unsigned int, or u32).
 *
 * File structure:
 *
 * <WIDTH><HEIGHT><BIT_DEPTH>
 * <UNCOMPRESSED IMAGE DATA>
 *
 * Example:
 *
 * [  32][  32][  24]
 * [RGB][RGB][RGB][RGB]...
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "1.0.0";
    o->name = "Raw uncompressed RGB image";
    o->filter = "*.rawrgb ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-rawrgb";
    o->pixmap = codec_rawrgb;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = true;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    u32 w, h, bpp;
    frs.readK(&w, sizeof(u32));
    frs.readK(&h, sizeof(u32));
    frs.readK(&bpp, sizeof(u32));
    
    if(bpp != 32 && bpp != 24)
	return SQE_R_BADFILE;
    
    image.w = w;
    image.h = h;
    image.bpp = bpp;
    image.compression = "-";
    image.colorspace = (bpp == 24 ? "RGB" : "RGBA");

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    RGB rgb;
    RGBA rgba;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(im->bpp == 32)
	for(s32 i = 0;i < im->w;i++)
	{
	    frs.readK(&rgba, sizeof(RGBA));
	    memcpy(scan+i, &rgba, sizeof(RGBA));
	}
    else
	for(s32 i = 0;i < im->w;i++)
	{
	    frs.readK(&rgb, sizeof(RGB));
	    memcpy(scan+i, &rgb, sizeof(RGB));
	}

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
}

s32 fmt_codec::write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    if(!image.w || !image.h || file.empty())
	return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    fws.open(file.c_str(), ios::binary | ios::out);

    if(!fws.good())
	return SQE_W_NOFILE;

    return SQE_OK;
}

s32 fmt_codec::write_next()
{
    s32 bpp = 32;

    if(!fws.writeK(&writeimage.w, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&writeimage.h, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&bpp, sizeof(s32))) return SQE_W_ERROR;

    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    u8 *p, a;

    for(s32 j = 0;j < writeimage.w;j++)
    {
        p = (u8 *)(scan +j);

        fws.writeK(p,   sizeof(u8));
        fws.writeK(p+1, sizeof(u8));
        fws.writeK(p+2, sizeof(u8));

        a = (writeopt.alpha) ? *(p+3) : 255;

        if(!fws.writeK(&a, sizeof(u8))) return SQE_W_ERROR;
    }

    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("rawrgb");
}

#include "fmt_codec_cd_func.h"

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

#include "fmt_codec_avs_defs.h"
#include "fmt_codec_avs.h"

#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "../xpm/codec_avs.xpm"

/*
 *
 * AVS X
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.1";
    o->name = "AVS X image";
    o->filter = "*.x ";
    o->mime = "";
    o->mimetype = "image/x-avs";
    o->config = "";
    o->pixmap = codec_avs;
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
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    if(!frs.readK(&image.w, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&image.h, sizeof(s32))) return SQE_R_BADFILE;

    image.w = fmt_utils::konvertLong(image.w);
    image.h = fmt_utils::konvertLong(image.h);
    image.bpp = 32;

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(32);

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
    u8 a;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(s32 i = 0;i < im->w;i++)
    {
	if(!frs.readK(&a, sizeof(u8))) return SQE_R_BADFILE;
	if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

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
    opt->passes = 1;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
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
    s32	w = fmt_utils::konvertLong(writeimage.w);
    s32	h = fmt_utils::konvertLong(writeimage.h);

    if(!fws.writeK(&w, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&h, sizeof(s32))) return SQE_W_ERROR;

    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    RGBA rgba;

    for(s32 i = 0;i < writeimage.w;i++)
    {
	rgba.r = scan[i].a;
	rgba.g = scan[i].r;
	rgba.b = scan[i].g;
	rgba.a = scan[i].b;

	if(!fws.writeK(&rgba, sizeof(RGBA))) return SQE_W_ERROR;
    }

    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("x");
}


#include "fmt_codec_cd_func.h"

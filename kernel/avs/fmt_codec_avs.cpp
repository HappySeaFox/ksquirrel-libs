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

#include "fmt_types.h"
#include "fileio.h"

#include "fmt_codec_avs_defs.h"
#include "fmt_codec_avs.h"

#include "error.h"

#include "fmt_utils.h"

/*
 *
 * AVS X
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("AVS X image");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.x ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,78,78,78,174,174,174,202,202,202,70,70,70,254,254,254,178,178,178,158,254,2,242,242,242,222,222,222,2,2,2,84,216,247,160,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,86,73,68,65,84,120,218,99,16,4,129,4,6,6,6,97,99,99,99,195,73,32,70,104,104,168,97,103,2,148,33,40,0,97,24,27,3,25,34,46,16,192,32,94,14,6,37,12,226,171,150,47,175,170,90,5,100,44,7,49,208,68,86,173,42,175,2,51,170,150,131,69,96,186,224,230,128,44,5,155,204,8,118,135,0,3,0,127,44,39,184,165,210,199,100,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
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

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGB rgb;
    u8 a;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
    {
	if(!frs.readK(&a, sizeof(u8))) return SQE_R_BADFILE;
	if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

	memcpy(scan+i, &rgb, sizeof(RGB));
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
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

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
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

s32 fmt_codec::fmt_write_next()
{
    s32	w = fmt_utils::konvertLong(writeimage.w);
    s32	h = fmt_utils::konvertLong(writeimage.h);

    if(!fws.writeK(&w, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&h, sizeof(s32))) return SQE_W_ERROR;

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
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

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("x");
}

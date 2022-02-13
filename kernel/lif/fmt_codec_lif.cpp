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
#include "fmt_utils.h"

#include "fmt_codec_lif_defs.h"
#include "fmt_codec_lif.h"

#include "error.h"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.2");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Homeworld LIF");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.lif ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,0,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,10,211,141,76,76,76,100,96,186,229,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,89,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,157,51,59,58,59,58,192,140,25,51,59,103,64,25,29,51,97,34,32,6,80,69,39,152,1,211,5,55,7,100,41,216,100,46,176,59,22,48,0,0,233,205,60,128,175,60,76,203,0,0,0,0,73,69,78,68,174,66,96,130");
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

    if(!frs.readK(&lif, sizeof(lif_header))) return SQE_R_BADFILE;
/*
    lif.version = fmt_utils::konvertLong(lif.version);
    lif.flags = fmt_utils::konvertLong(lif.flags);
    lif.width = fmt_utils::konvertLong(lif.width);
    lif.height = fmt_utils::konvertLong(lif.height);
    lif.palOffset = fmt_utils::konvertLong(lif.palOffset);


    if(lif.version != LIF_VERSION || lif.flags != LIF_FLAGS)
	return SQE_R_BADFILE;
*/

    if(strcmp(lif.id, LIF_ID))
	return SQE_R_BADFILE;

    image.w = lif.width;
    image.h = lif.height;

    fstream::pos_type pos = frs.tellg();

    frs.seekg(lif.width * lif.height, ios::beg);

    if(!frs.readK(pal, sizeof(pal)))
	return SQE_R_BADFILE;

    frs.seekg(pos);

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(8);
    image.bpp = 8;
    image.hasalpha = (bool)(lif.flags & 0x08);

    bytes = image.hasalpha ? sizeof(RGBA) : sizeof(RGB);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u8 c;
    fmt_image *im = image(currentImage);

    memset(scan, 255, im->w * sizeof(RGBA));

    for(s32 i = 0;i < im->w;i++)
    {
	if(!frs.readK(&c, sizeof(u8))) return SQE_R_BADFILE;

	c++;

	memcpy(scan+i, pal+c, bytes);
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
    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA * /*scan*/)
{
    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

#include "fmt_codec_cd_func.h"

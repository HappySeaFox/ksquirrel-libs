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
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_lif_defs.h"
#include "fmt_codec_lif.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_lif.xpm"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.2";
    o->name = "Homeworld LIF";
    o->filter = "*.lif ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-lif";
    o->pixmap = codec_lif;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
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

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    u8 c;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(s32 i = 0;i < im->w;i++)
    {
	if(!frs.readK(&c, sizeof(u8))) return SQE_R_BADFILE;

	c++;

	memcpy(scan+i, pal+c, bytes);
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

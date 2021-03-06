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

#include "fmt_codec_wal_defs.h"
#include "fmt_codec_wal.h"

#include "ksquirrel-libs/error.h"

#include "q2pal.h"

#include "../xpm/codec_wal.xpm"

/*
 *
 * Quake2 WAL texture
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.2.0";
    o->name = "Quake2 texture";
    o->filter = "*.wal ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-wal";
    o->pixmap = codec_wal;
    o->readable = true;
    o->canbemultiple = true;
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

    bits = NULL;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage == 4)
        return SQE_NOTOK;

    if(!currentImage)
    {
	if(!frs.readK(&wal, sizeof(wal_header)))
	    return SQE_R_BADFILE;

	neww = wal.width;
	newh = wal.height;

	fmt_metaentry mt;

        mt.group = "Quake2 texture name";
	mt.data = wal.name;
	addmeta(mt);

        mt.group = "Quake2 next texture name";
	mt.data = wal.next_name;
	addmeta(mt);
    }
    else
    {
	neww /= 2;
	newh /= 2;
    }

    bits = (u8 *)realloc(bits, neww * newh);

    if(!bits)
	return SQE_R_NOMEMORY;

    fmt_image image;

    frs.seekg(wal.offset[currentImage], ios::beg);

    if(!frs.good())
	return SQE_R_BADFILE;

    image.w = neww;
    image.h = newh;
    image.bpp = 8;
    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(8);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(!frs.readK(bits, im->w))
	return SQE_R_BADFILE;

    for(s32 i = 0;i < im->w;i++)
    {
	scan[i].r = q2pal[bits[i] * 3];
	scan[i].g = q2pal[bits[i] * 3 + 1];
	scan[i].b = q2pal[bits[i] * 3 + 2];
    }
    
    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    if(bits) free(bits);
}

#include "fmt_codec_cd_func.h"

/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2007 Dmitry Baryshev <ksquirrel@tut.by>

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

#include "fmt_codec_dds_defs.h"
#include "fmt_codec_dds.h"

#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "../xpm/codec_dds.xpm"

typedef RGBA* RGBAP;

inline void FREE_ROWS(RGBAP **A, const int H)
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

/*
 *
 * DDS
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.0";
    o->name = "DirectDraw Surface";
    o->filter = "*.dds ";
    o->mime = "";
    o->mimetype = "image/x-dds";
    o->config = "";
    o->pixmap = codec_dds;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    dds.img = 0;
    dds.w = dds.h = 0;

    if(!dds_read(file, dds))
        return SQE_R_BADFILE;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    image.w = dds.w;
    image.h = dds.h;
    image.bpp = 32;

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(32);

    finfo.image.push_back(image);
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    line++;

    memcpy(scan, dds.img[line], dds.w * sizeof(RGBA));

    return SQE_OK;
}

void fmt_codec::read_close()
{
    FREE_ROWS(&dds.img, dds.h);

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

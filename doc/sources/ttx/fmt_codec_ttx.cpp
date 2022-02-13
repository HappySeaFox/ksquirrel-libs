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

#include "fmt_codec_ttx_defs.h"
#include "fmt_codec_ttx.h"

#include "error.h"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*. ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("");
}

s32 fmt_codec::fmt_read_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    finfo.image.push_back(fmt_image());



    finfo.images++;
    finfo.image[currentImage].compression = "";
    finfo.image[currentImage].colorspace = "";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGB rgb;
    RGBA rgba;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));


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
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &opt)
{
    return SQE_W_NOTSUPPORTED;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

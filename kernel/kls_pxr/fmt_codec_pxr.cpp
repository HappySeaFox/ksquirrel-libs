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
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"

#include "fmt_codec_pxr_defs.h"
#include "fmt_codec_pxr.h"

#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "../xpm/codec_pxr.xpm"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.0";
    o->name = "Pxrar format";
    o->filter = "*.pxr ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_pxr;
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

    u16 w, h;
    u8 bppi;

    frs.seekg(416, ios::beg);
    if(!frs.readK(&h, sizeof(u16))) return SQE_R_BADFILE;
    if(!frs.readK(&w, sizeof(u16))) return SQE_R_BADFILE;

    frs.seekg(424, ios::beg);
    if(!frs.readK(&bppi, sizeof(u8))) return SQE_R_BADFILE;

    fmt_image image;

    image.w = w;
    image.h = h;

    if(bppi == 0x08)
        image.bpp = 1;
    else if(bppi == 0x0E)
        image.bpp = 24;
    else if(bppi == 0x0F)
        image.bpp = 32;
    else
        return SQE_R_BADFILE;

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(image.bpp);

    frs.seekg(1024, ios::beg);

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
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(im->bpp)
    {
        case 1:
        {
            u8 c;

            for(s32 i = 0;i < im->w;i++)
            {
                if(!frs.readK(&c, sizeof(u8))) return SQE_R_BADFILE;

                memset(scan+i, c, sizeof(RGB));
            }
        }
        break;

        case 24:
        {
            for(s32 i = 0;i < im->w;i++)
            {
                if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

                memcpy(scan+i, &rgb, sizeof(RGB));
            }
        }
        break;

        case 32:
        {
            if(!frs.readK(scan, im->w * sizeof(RGBA)))
                return SQE_R_BADFILE;
        }
        break;
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
    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA * /*scan*/)
{
    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string();
}

#include "fmt_codec_cd_func.h"

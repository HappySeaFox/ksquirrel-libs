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
#include "fmt_utils.h"
#include "fileio.h"

#include "fmt_codec_jbig_defs.h"
#include "fmt_codec_jbig.h"

#include "error.h"

int call(char **);

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
    return std::string("JBIG");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.jbg ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,0,76,76,76,109,105,105,174,174,174,176,176,176,177,177,177,200,200,200,221,221,221,243,243,243,255,255,255,69,69,69,246,59,12,175,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,85,73,68,65,84,8,215,99,16,4,1,7,6,6,6,177,180,180,180,196,98,16,99,230,204,153,137,237,1,80,134,160,0,132,145,150,6,100,72,173,130,0,6,33,37,48,208,98,16,154,57,9,8,103,2,25,74,147,128,64,9,204,152,57,105,18,92,68,19,69,13,84,23,220,28,144,165,96,147,25,193,238,16,96,0,0,136,182,41,106,141,55,213,83,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    frs.close();
    
    tmp = fmt_utils::adjustTempName(file);

    const char *argv[2] =
    {
        file.c_str(),
        tmp.c_str()
    };

    if(call((char **)argv))
        return SQE_R_NOMEMORY;

    frs.open(tmp.c_str(), ios::binary | ios::in);

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

    u32 w, h, bpp;
    frs.readK(&w, sizeof(u32));
    frs.readK(&h, sizeof(u32));
    frs.readK(&bpp, sizeof(u32));

    if(bpp != 24)
        return SQE_R_BADFILE;

    image.w = w;
    image.h = h;
    image.bpp = bpp;

    image.compression = "?";
    image.colorspace = fmt_utils::colorSpaceByBpp(1);

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
    fmt_image *im = image(currentImage);

    memset(scan, 255, im->w * sizeof(RGBA));

    for(s32 i = 0;i < im->w;i++)
    {
        frs.readK(&rgb, sizeof(RGB));
        memcpy(scan+i, &rgb, sizeof(RGB));
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    unlink(tmp.c_str());
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

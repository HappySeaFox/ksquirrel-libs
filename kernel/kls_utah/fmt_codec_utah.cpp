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
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_utah_defs.h"
#include "fmt_codec_utah.h"

#include "../xpm/codec_utah.xpm"

/*
 *
 * The Utah RLE format was developed by Spencer Thomas at the University of Utah
 * Department of Computer Science. The first version appeared around 1983. The
 * work was partially funded by the NSF, DARPA, the Army Research Office, and the
 * Office of Naval Research. It was developed mainly to support the Utah Raster
 * Toolkit (URT), which is widely distributed in source form on the Internet.
 * Although superseded by more recent work, the Utah Raster Toolkit remains a
 * source of ideas and bitmap manipulation code for many.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.1";
    o->name = "UTAH RLE";
    o->filter = "*.rle ";
    o->config = "";
    o->mime = "\x0052\x00CC";
    o->pixmap = codec_utah;
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

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    if(!frs.readK(&utah, sizeof(UTAH_HEADER))) return SQE_R_BADFILE;

    if(utah.magic != UTAH_MAGIC) return SQE_R_BADFILE;

    if(utah.ncmap || (utah.ncolors != 3 && utah.ncolors != 4)) return SQE_R_NOTSUPPORTED;

    if(utah.ncmap != 1 && utah.ncmap != 0 && utah.ncmap != utah.ncolors) return SQE_R_BADFILE;

    image.w = utah.xsize;
    image.h = utah.ysize;
    image.bpp = 8;
    
    printf("flags: %d\nncolors: %d\nncmap: %d\ncmaplen: %d\nred: %d, green: %d,blue: %d\n",
    utah.flags,
    utah.ncolors,
    utah.ncmap,
    utah.cmaplen,
    utah.red,
    utah.green,
    utah.blue
    );

    bool clearfirst = utah.flags & 1;
    bool nobackgr   = utah.flags & 2;
    bool alpha      = utah.flags & 4;
    bool comments   = utah.flags & 8;

    // read comments, if present
    if(comments)
    {
        u16 len;
        
        if(!frs.readK(&len, sizeof(u16))) return SQE_R_BADFILE;

        frs.seekg(len, ios::cur);
    }

    printf("pos: %d\n", (int)frs.tellg());

    image.compression = "RLE";
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
    RGB rgb;
    RGBA rgba;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);


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
    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
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

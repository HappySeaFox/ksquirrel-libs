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

#include "fmt_codec_mdl_defs.h"
#include "fmt_codec_mdl.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_mdl.xpm"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.2.0";
    o->name = "HalfLife model";
    o->filter = "*.mdl ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_mdl;
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

    s32 id, ver;

    if(!frs.readK(&id, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&ver, sizeof(s32))) return SQE_R_BADFILE;

    if(id != 0x54534449 || ver != 10)
	return SQE_R_BADFILE;

    frs.seekg(172, ios::cur);

    if(!frs.readK(&numtex, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&texoff, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&texdataoff, sizeof(s32))) return SQE_R_BADFILE;

    if(!numtex || !texoff || !texdataoff)
	return SQE_R_BADFILE;

    frs.seekg(texoff, ios::beg);

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage == numtex)
        return SQE_NOTOK;

    fmt_image image;

    if(currentImage)
	frs.seekg(opos);

    if(!frs.readK(tex.name, sizeof(tex.name))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.flags, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.width, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.height, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.offset, sizeof(s32))) return SQE_R_BADFILE;
    
    opos = frs.tellg();

    if(!tex.offset)
        return SQE_R_BADFILE;

    frs.seekg(tex.offset, ios::beg);
    
    if(!frs.good())
	return SQE_R_BADFILE;

    image.w = tex.width;
    image.h = tex.height;

    fstream::pos_type pos = frs.tellg();

    frs.seekg(tex.width * tex.height, ios::cur);

    if(!frs.readK(pal, sizeof(RGB) * 256)) return SQE_R_BADFILE;

    frs.seekg(pos);

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(8);
	image.bpp = 8;

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    u8 index;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(s32 x = 0;x < im->w;x++)
    {
	if(!frs.readK(&index, sizeof(u8))) return SQE_R_BADFILE;

	memcpy(scan+x, pal+index, sizeof(RGB));
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

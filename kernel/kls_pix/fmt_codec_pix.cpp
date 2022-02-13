/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>

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

#include "fmt_codec_pix_defs.h"
#include "fmt_codec_pix.h"

#include "../xpm/codec_pix.xpm"

/*
 *
 * This format is sourced on IRIX
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "1.1.0";
    o->name = "Irix PIX image";
    o->filter = "*.pix ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_pix;
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
    u16 _tmp;
    PIX_HEADER	pfh;

    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    if(!frs.be_getshort(&pfh.width)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&pfh.height)) return SQE_R_BADFILE;

    if(!frs.readK(&_tmp, sizeof(u16))) return SQE_R_BADFILE;
    if(!frs.readK(&_tmp, sizeof(u16))) return SQE_R_BADFILE;

    if(!frs.be_getshort(&pfh.bpp)) return SQE_R_BADFILE;

    if(pfh.bpp != 24)	
	return SQE_R_BADFILE;

    image.w = pfh.width;
    image.h = pfh.height;
    image.bpp = pfh.bpp;
    image.compression = "RLE";
    image.colorspace = fmt_utils::colorSpaceByBpp(24);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    s32	len = 0, i, counter = 0;
    u8 count;
    RGB	rgb;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(im->bpp)
    {
	case 24:
	    do
	    {
		if(!frs.readK(&count, 1)) return SQE_R_BADFILE;
		len += count;
		
		if(!frs.readK(&rgb.b, 1)) return SQE_R_BADFILE;
		if(!frs.readK(&rgb.g, 1)) return SQE_R_BADFILE;
		if(!frs.readK(&rgb.r, 1)) return SQE_R_BADFILE;

		for(i = 0;i < count;i++)
		    memcpy(scan+counter++, &rgb, 3);
		    
	    }while(len < im->w);
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
    opt->compression_scheme = CompressionInternal;
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

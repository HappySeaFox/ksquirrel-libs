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
#include <sstream>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_sct_defs.h"
#include "fmt_codec_sct.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_sct.xpm"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.2.2";
    o->name = "Scitex CT";
    o->filter = "*.sct *.ct ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_sct;
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

    s8	comment[0x50 + 1], sig[2];

    if(!frs.readK(comment, sizeof(comment) - 1)) return SQE_R_BADFILE;
    if(!frs.readK(sig, sizeof(sig))) return SQE_R_BADFILE;

    comment[0x50] = '\0';

    frs.seekg(0x400, ios::beg);

    if(!frs.readK(&sct, sizeof(sct_header))) return SQE_R_BADFILE;

    sct.format = fmt_utils::konvertWord(sct.format);

    if(sct.format != SCT_FORMAT_RGB && sct.format != SCT_FORMAT_GRAY && sct.format != SCT_FORMAT_CMYK)
	return SQE_R_BADFILE;

    if(sct.format == SCT_FORMAT_RGB && sct.channels != 3)
	return SQE_R_BADFILE;

    if(sct.format == SCT_FORMAT_GRAY && sct.channels != 1)
	return SQE_R_BADFILE;

    if(sct.format == SCT_FORMAT_CMYK && sct.channels != 4)
	return SQE_R_BADFILE;

    if((sct.width[0] != '+' && sct.width[0] != '-') || (sct.height[0] != '+' && sct.height[0] != '-'))
	return SQE_R_BADFILE;

    std::string buf;
    
    buf.assign(sct.width, sizeof(sct.width));

    std::stringstream ss(buf);

    ss.setf(ios::hex);

    ss >> image.h;

    buf.assign(sct.height, sizeof(sct.height));

    std::stringstream ss1(buf);

    ss1.setf(ios::hex);

    ss1 >> image.w;

    image.compression = "-";

    switch(sct.format)
    {
	case SCT_FORMAT_RGB:
	    image.colorspace = "RGB";
	    image.bpp = 24;
	break;

	case SCT_FORMAT_GRAY:
	    image.colorspace = "Grayscale";
	    image.bpp = 8;
	break;

	case SCT_FORMAT_CMYK:
	    image.colorspace = "CMYK";
	    image.bpp = 32;
	break;
    }

	fmt_metaentry mt;

    mt.group = "Comment";
    mt.data = comment;

    addmeta(mt);

    finfo.image.push_back(image);

    frs.seekg(0x800, ios::beg);

    return (frs.good()) ? SQE_OK : SQE_R_BADFILE;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    RGB rgb;
    RGBA rgba;
    u8 c, *p;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(sct.format)
    {
	case SCT_FORMAT_RGB:
	    for(s32 ch = 0;ch < 3;ch++)
		for(s32 i = 0;i < im->w;i++)
		{
		    if(!frs.readK(&c, sizeof(u8))) return SQE_R_BADFILE;

		    p = (u8 *)(scan + i);
		    *(p + ch) = c;
		}
	break;

	case SCT_FORMAT_GRAY:
	    for(s32 i = 0;i < im->w;i++)
	    {
		if(!frs.readK(&c, sizeof(c))) return SQE_R_BADFILE;

		(scan+i)->r = c;
		(scan+i)->g = c;
		(scan+i)->b = c;
	    }
	break;

	case SCT_FORMAT_CMYK:
	    for(s32 ch = 0;ch < 4;ch++)
		for(s32 i = 0;i < im->w;i++)
		{
		    if(!frs.readK(&c, sizeof(u8))) return SQE_R_BADFILE;

		    p = (u8 *)(scan + i);
		    *(p + ch) = c;
		}

		for(s32 i = 0;i < im->w;i++)
		{
            	    scan[i].r = (scan[i].r * scan[i].a) >> 8;
		    scan[i].g = (scan[i].g * scan[i].a) >> 8;
		    scan[i].b = (scan[i].b * scan[i].a) >> 8;
		    scan[i].a = 255;
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

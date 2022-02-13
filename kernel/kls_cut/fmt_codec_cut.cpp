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
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_cut_defs.h"
#include "fmt_codec_cut.h"

#include "../xpm/codec_cut.xpm"

/*
 *
 * The Dr. Halo file format is a device-independent interchange format used for
 * transporting image data from one hardware environment or operating system to
 * another. This format is associated with the HALO Image File Format
 * Library.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    for(s32 i = 0;i < 256;i++)
	memset(pal+i, i, sizeof(RGB));	
}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.1";
    o->name = "Dr. Halo CUT";
    o->filter = "*.cut ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_cut;
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
    s16 width, height;
    s32 dummy;

    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    if(!frs.readK(&width, sizeof(s16))) return SQE_R_BADFILE;
    if(!frs.readK(&height, sizeof(s16))) return SQE_R_BADFILE;
    if(!frs.readK(&dummy, sizeof(s32))) return SQE_R_BADFILE;

    image.w = width;
    image.h = height;
    image.bpp = 8;

    image.compression = "RLE";
    image.colorspace = "Color indexed";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    s32 i = 0, j;
    u8 count, run, c;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);
    
    while(i < im->w)
    {
	if(!frs.readK(&count, 1)) return SQE_R_BADFILE;

	if(count == 0)
	{
	    frs.readK(&c, 1);

	    if(!frs.readK(&c, 1)) return SQE_R_BADFILE;
	    
	    continue; // ugly hack
	}
	else if(count & 0x80)
	{
	    count &= ~(0x80);

	    if(!frs.readK(&run, 1)) return SQE_R_BADFILE;

	    for(j = 0;j < count;j++)
	    {
		memcpy(scan+i, pal+run, sizeof(RGB));
		i++;
	    }
	}
	else
	{
	    for(j = 0;j < count;j++)
	    {
		if(!frs.readK(&run, 1)) return SQE_R_BADFILE;

		memcpy(scan+i, pal+run, sizeof(RGB));

		i++;
	    }
	}
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

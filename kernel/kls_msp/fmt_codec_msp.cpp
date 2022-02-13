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
#include <vector>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"

#include "fmt_codec_msp_defs.h"
#include "fmt_codec_msp.h"

#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "../xpm/codec_msp.xpm"

static const RGB palmono[2] = { RGB(255,255,255), RGB(0,0,0) };

/*
 *
 * The Microsoft Paint (MSP) image file format is used exclusively for storing
 * black-and-white images. The vast majority of MSP files contain line drawings
 * and clip art. MSP is used most often by Microsoft Windows applications, but
 * may be used by MS-DOS-based programs as well. The Microsoft Paint format is
 * apparently being replaced by the more versatile Microsoft Windows BMP format;
 * it contains information specifically for use in the Microsoft Windows operating
 * environment. For information on the Windows-specific use of the header information,
 * refer to the Microsoft Paint format specification available from Microsoft.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.0";
    o->name = "Microsoft Paint";
    o->filter = "*.msp ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-msp";
    o->pixmap = codec_msp;
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
    bytes = NULL;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;
    
    if(!frs.readK(&msp, sizeof(msp_header)))
	return SQE_R_BADFILE;

    if(msp.key1 == MAGIC_OLD_1 || msp.key2 == MAGIC_OLD_2)
	version = 1;
    else if(msp.key1 == MAGIC_1 || msp.key2 == MAGIC_2)
	version = 2;
    else 
	return SQE_R_BADFILE;

    bytes = new u8 [msp.width];
    
    if(!bytes)
	return SQE_R_NOMEMORY;

    printf("\n");
	
    if(version == 2)
    {
        u16 map_entry;

	frs.seekg(32, ios::beg);

        for(s32 i = 0;i < msp.height;i++)
	{
	    if(!frs.readK(&map_entry, sizeof(u16)))
		return SQE_R_BADFILE;

	    printf("%d,", map_entry);

	    scanmap.push_back(map_entry);
	}
    }

    printf("\n");

    image.w = msp.width;
    image.h = msp.height;
    image.bpp = 1;
    image.compression = (version == 2) ? "RLE" : "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(1);

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
    RGB rgb;
    RGBA rgba;
    s32 i = 0, k = 0;
    u8 c, count, value;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    line++;
    
    const u16 sz = scanmap[line];
    
    memset(bytes, 0, im->w);

    printf("LINE %d, SZ %d --------------------------\n", line, sz);

    while(i < sz)
    {
	if(!frs.readK(&c, sizeof(u8)))
	    return SQE_R_BADFILE;

	i++;

	if(!c)
	{
	    if(!frs.readK(&count, sizeof(u8))) return SQE_R_BADFILE;
	    if(!frs.readK(&value, sizeof(u8))) return SQE_R_BADFILE;

	    i += 2;

	    printf("!C %d,%d\n", count, value);

	    if(count)
	    {
		for(s32 s = 0;s < (s32)count;s++)
		    bytes[k+s] = value;

		k += count;
	    }
	}
	else
	{
	    if(!frs.readK(bytes+k, sizeof(u8) * c)) return SQE_R_BADFILE;

	    printf("C\n");

	    i++;
	    k += c;
	}
    }
    
    s32 aa = 320;
    for(s32 k = 0;k <= line;k++)
    aa += scanmap[k];
    
    printf("calc seek: %d, orig seek: %d", aa, (int)frs.tellg());

    s32 ind = 0;

    for(i = 0;i < sz;i++)
    {
	fmt_utils::expandMono1Byte(bytes[i], byte);
	printf("*** %d => %d,%d,%d,%d,%d,%d,%d,%d\n", bytes[i], byte[0], byte[1], byte[2], byte[3], byte[4], byte[5], byte[6], byte[7]);

	for(s32 j = 0;j < 8 || ind < im->w;j++)
	{
	    memcpy(scan+ind, palmono+byte[i], sizeof(RGB));
	    ind++;
	}
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    scanmap.clear();

    delete [] bytes;
    bytes = NULL;
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

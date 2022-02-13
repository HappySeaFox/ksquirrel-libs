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
#include "error.h"

#include "fmt_codec_rawrgb_defs.h"
#include "fmt_codec_rawrgb.h"

/*
 *
 * This is a codec to read and write internal raw image format.
 * This format has a simple header folowed by uncompressed image data in
 * 24 or 32 bit format. Width, height and bit depth are integers (unsigned int, or u32).
 *
 * File structure:
 *
 * <WIDTH><HEIGHT><BIT_DEPTH>
 * <UNCOMPRESSED IMAGE DATA>
 *
 * Example:
 *
 * [  32][  32][  24]
 * [RGB][RGB][RGB][RGB]...
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.0.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Raw uncompressed RGB image");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.rawrgb ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,242,242,242,202,202,202,86,106,142,178,178,178,174,174,174,254,254,254,78,78,78,222,222,222,238,238,254,70,70,70,147,120,168,25,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,109,73,68,65,84,120,218,53,141,33,14,133,48,16,68,71,115,2,244,26,18,236,26,90,197,105,170,9,8,2,150,32,208,163,138,34,255,11,2,167,164,75,195,83,111,51,153,89,140,70,11,224,39,34,245,106,18,66,168,203,54,75,53,30,89,68,58,96,190,51,248,235,139,199,20,53,146,186,227,82,46,36,119,76,155,50,105,18,101,164,107,188,69,164,115,30,195,215,186,191,157,94,140,244,162,176,243,236,240,0,144,145,50,150,72,18,227,15,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

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
    
    if(bpp != 32 && bpp != 24)
	return SQE_R_BADFILE;
    
    image.w = w;
    image.h = h;
    image.bpp = bpp;
    image.compression = "-";
    image.colorspace = (bpp == 24 ? "RGB" : "RGBA");

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
    RGBA rgba;
    fmt_image *im = image(currentImage);

    memset(scan, 255, im->w * sizeof(RGBA));

    if(im->bpp == 32)
	for(s32 i = 0;i < im->w;i++)
	{
	    frs.readK(&rgba, sizeof(RGBA));
	    memcpy(scan+i, &rgba, sizeof(RGBA));
	}
    else
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
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
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
    s32 bpp = 32;

    if(!fws.writeK(&writeimage.w, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&writeimage.h, sizeof(s32))) return SQE_W_ERROR;
    if(!fws.writeK(&bpp, sizeof(s32))) return SQE_W_ERROR;

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    u8 *p, a;

    for(s32 j = 0;j < writeimage.w;j++)
    {
        p = (u8 *)(scan +j);

        fws.writeK(p,   sizeof(u8));
        fws.writeK(p+1, sizeof(u8));
        fws.writeK(p+2, sizeof(u8));

        a = (writeopt.alpha) ? *(p+3) : 255;

        if(!fws.writeK(&a, sizeof(u8))) return SQE_W_ERROR;
    }

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("rawrgb");
}

#include "fmt_codec_cd_func.h"

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

#include "fmt_types.h"
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_pix_defs.h"
#include "fmt_codec_pix.h"

/*
 *
 * This format is sourced on IRIX
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.1.0");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Irix PIX image");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.pix ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,78,78,78,174,174,174,202,202,202,70,70,70,254,254,254,254,206,50,178,178,178,242,242,242,222,222,222,2,2,2,244,185,180,46,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,95,73,68,65,84,120,218,99,16,4,129,2,6,6,6,97,99,99,99,195,73,32,70,104,104,168,73,39,80,200,25,200,48,20,20,128,136,24,27,3,25,46,80,192,32,150,6,6,41,12,110,89,171,210,150,45,91,150,194,32,150,5,162,151,65,69,178,210,192,34,105,80,17,40,3,174,11,110,142,176,113,49,208,90,7,6,6,22,71,17,23,23,71,1,6,0,69,226,39,86,87,102,14,67,0,0,0,0,73,69,78,68,174,66,96,130");
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
    u16 tmp;
    PIX_HEADER	pfh;

    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    if(!frs.be_getshort(&pfh.width)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&pfh.height)) return SQE_R_BADFILE;

    if(!frs.readK(&tmp, sizeof(u16))) return SQE_R_BADFILE;
    if(!frs.readK(&tmp, sizeof(u16))) return SQE_R_BADFILE;

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

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32	len = 0, i, counter = 0;
    u8 count;
    RGB	rgb;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
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
		    
	    }while(len < finfo.image[currentImage].w);
	break;

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
    opt->compression_scheme = CompressionInternal;
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
    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
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

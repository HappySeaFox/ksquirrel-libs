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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_utah_defs.h"
#include "fmt_codec_utah.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("UTAH RLE");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.rle ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("\x0052\x00CC");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,78,78,78,170,194,90,202,202,202,70,70,70,254,254,254,178,178,178,242,242,242,174,174,174,222,222,222,2,2,2,65,60,145,233,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,94,73,68,65,84,120,218,99,16,4,129,4,6,6,6,97,99,99,99,195,25,32,70,104,104,168,73,37,80,200,25,200,48,20,20,128,136,24,27,3,25,46,80,192,32,164,4,6,42,12,78,171,22,105,41,105,173,82,97,16,90,164,5,100,0,69,132,192,34,139,144,69,128,140,85,96,53,48,93,112,115,64,150,26,27,59,48,48,176,128,184,142,2,12,0,152,20,27,171,101,163,83,199,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&utah, sizeof(UTAH_HEADER))) return SQE_R_BADFILE;

    if(utah.magic != UTAH_MAGIC)  return SQE_R_BADFILE;

    if(utah.ncolors != 3 && utah.ncolors != 4) return SQE_R_NOTSUPPORTED;

    if(utah.ncmap != 1 && utah.ncmap != 0 && utah.ncmap != utah.ncolors) return SQE_R_BADFILE;

    finfo.image[currentImage].w = utah.xsize;
    finfo.image[currentImage].h = utah.ysize;
    finfo.image[currentImage].bpp = 8;
    
    printf("flags: %d\nncolors: %d\nncmap: %d\ncmaplen: %d\nred: %d, green: %d,blue: %d\n",
    utah.flags,
    utah.ncolors,
    utah.ncmap,
    utah.cmaplen,
    utah.red,
    utah.green,
    utah.blue
    );

    finfo.images++;
    finfo.image[currentImage].compression = "RLE";
    finfo.image[currentImage].colorspace = "Color indexed";

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

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));


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

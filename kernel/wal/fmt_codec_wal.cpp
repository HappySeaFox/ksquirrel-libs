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
#include "fmt_utils.h"

#include "fmt_codec_wal_defs.h"
#include "fmt_codec_wal.h"

#include "error.h"

#include "q2pal.h"

/*
 *
 * Quake2 WAL texture
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Quake2 texture");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.wal ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,198,73,142,76,76,76,185,109,193,98,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,91,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,51,167,206,12,157,25,9,100,76,157,58,117,102,36,50,99,101,100,228,212,80,24,99,102,100,104,4,66,23,220,28,144,165,96,147,185,192,238,88,192,0,0,70,94,57,78,0,45,81,153,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;
    finfo.images = 0;

    bits = NULL;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage == 4)
        return SQE_NOTOK;

    if(!currentImage)
    {
	if(!frs.readK(&wal, sizeof(wal_header)))
	    return SQE_R_BADFILE;

	neww = wal.width;
	newh = wal.height;

	finfo.meta.push_back(fmt_metaentry());
	finfo.meta.push_back(fmt_metaentry());

        finfo.meta[0].group = "Quake2 texture name";
	finfo.meta[0].data = wal.name;
        finfo.meta[1].group = "Quake2 next texture name";
	finfo.meta[1].data = wal.next_name;
    }
    else
    {
	neww /= 2;
	newh /= 2;
    }

    bits = (u8 *)realloc(bits, neww * newh);

    if(!bits)
	return SQE_R_NOMEMORY;

    finfo.image.push_back(fmt_image());

    frs.seekg(wal.offset[currentImage], ios::beg);

    if(!frs.good())
	return SQE_R_BADFILE;

    finfo.images++;
    finfo.image[currentImage].w = neww;
    finfo.image[currentImage].h = newh;
    finfo.image[currentImage].bpp = 8;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = fmt_utils::colorSpaceByBpp(8);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    if(!frs.readK(bits, finfo.image[currentImage].w))
	return SQE_R_BADFILE;

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
    {
	scan[i].r = q2pal[bits[i] * 3];
	scan[i].g = q2pal[bits[i] * 3 + 1];
	scan[i].b = q2pal[bits[i] * 3 + 2];
    }
    
    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    if(bits) free(bits);
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

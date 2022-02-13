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
#include "fileio.h"
#include "error.h"

#include "fmt_codec_cut_defs.h"
#include "fmt_codec_cut.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Dr. Halo CUT");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.cut ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,132,47,163,76,76,76,191,215,159,11,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,87,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,83,35,167,78,157,26,10,100,128,232,153,145,32,198,76,24,3,38,2,84,19,10,102,192,116,193,205,1,89,10,54,153,11,236,142,5,12,0,123,238,58,138,176,181,106,30,0,0,0,0,73,69,78,68,174,66,96,130");
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
    s16 width, height;
    s32 dummy;

    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&width, sizeof(s16))) return SQE_R_BADFILE;
    if(!frs.readK(&height, sizeof(s16))) return SQE_R_BADFILE;
    if(!frs.readK(&dummy, sizeof(s32))) return SQE_R_BADFILE;

    finfo.image[currentImage].w = width;
    finfo.image[currentImage].h = height;
    finfo.image[currentImage].bpp = 8;

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
    s32 i = 0, j;
    u8 count, run, c;
    
    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    while(i < finfo.image[currentImage].w)
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

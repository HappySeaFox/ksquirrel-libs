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

#include "fmt_codec_mac_defs.h"
#include "fmt_codec_mac.h"

#include "error.h"

#include "fmt_utils.h"

static const RGB palmono[2] = { RGB(0,0,0), RGB(255,255,255) };

bool checkForMacBinary(u8 *);

/*
 *
 * Macintosh Paint (MacPaint) is the original and most common
 * graphics file format used on the Apple Macintosh. Most Macintosh
 * applications that use graphics are able to read and write the
 * MacPaint format. MacPaint files on the Macintosh have the file
 * type PNTG, while on the PC they usually have the extension .MAC.
 * The first real image files widely available to PC users were MacPaint
 * files. PC users usually obtained them from BBSs or shareware disks,
 * and a number of programs exist that allow MacPaint files to be displayed
 * and printed using a PC under MS-DOS. Today, extensive black-and-white
 * clip art and graphics are available in the MacPaint format.
 * MacPaint files are also used to store line drawings, text, and scanned
 * images.
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
    return std::string("Macintosh Paint");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.mac ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,14,113,117,76,76,76,132,224,63,90,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,86,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,145,83,167,134,78,13,5,50,66,67,167,78,157,10,23,129,50,166,34,24,32,53,48,93,112,115,64,150,130,77,230,2,187,99,1,3,0,118,206,58,174,108,19,81,141,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    u8 head[128];
    
    if(!frs.readK(head, sizeof(head)))
	return SQE_R_BADFILE;

    s32 seek = /*checkForMacBinary(head) ? 512 : */384;

    frs.seekg(seek, ios::cur);

    if(!frs.good())
	return SQE_R_BADFILE;

    image.w = 576;
    image.h = 720;
    image.bpp = 1;
    image.compression = "RLE";
    image.colorspace = fmt_utils::colorSpaceByBpp(1);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 i = 0;
    u8 c, count, value;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
    printf("---------------------------------\n");

    while(i < 72)
    {
	if(!frs.readK(&c, sizeof(u8)))
	    return SQE_R_BADFILE;

	if(c > 128)
	{
//	    c <<= 1;
//	    c >>= 1;
	    count = ~c;
	    
	    if(i + count > 72)
		count = 72 - i;

	    printf("!C %d\n", count);
	    if(!frs.readK(&value, sizeof(u8)))
		return SQE_R_BADFILE;

	    memset(bytes+i, value, count);

	    i += count;
	}
	else
	{
	    count = c + 1;

	    if(i + count > 72)
		count = 72 - i;

	    printf("C %d\n", count);
	    
	    if(!frs.readK(bytes+i, sizeof(u8) * count))
		return SQE_R_BADFILE;

	    i += count;
	}
    }

    s32 k, m, ind = 0;

    for(k = 0;k < 72;k++)
    {
	fmt_utils::expandMono1Byte(*(bytes+k), byte);

	for(m = 0;m < 8;m++)
	{
	    memcpy(scan+ind, palmono+byte[m], sizeof(RGB));

	    ind++;
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

bool checkForMacBinary(u8 *h)
{
    for(s32 i = 101;i < 128;i++)
	if(h[i])
	    return false;
    
    if(h[2] < 1 || h[2] > 63)
	return false;
	
    return true;
}

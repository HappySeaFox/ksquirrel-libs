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

#include "fmt_codec_koala_defs.h"
#include "fmt_codec_koala.h"

#include "error.h"

// Code partially taken from FreeImage3

/*
 *
 *  Commodore 64 Koala
 *
 */

const RGB c64pal[16] =
{
        RGB(   0,   0,   0 ),      // Black
        RGB( 255, 255, 255 ),      // White
        RGB( 170,  17,  17 ),      // Red
        RGB(  12, 204, 204 ),      // Cyan
        RGB( 221,  51, 221 ),      // Purple
        RGB(  0,  187,  0  ),      // Green
        RGB(   0,   0, 204 ),      // Blue
        RGB( 255, 255, 140 ),      // Yellow
        RGB( 204, 119,  34 ),      // Orange
        RGB( 136,  68,   0 ),      // Brown
        RGB( 255, 153, 136 ),      // Light red
        RGB(  92,  92,  92 ),      // Gray 1
        RGB( 170, 170, 170 ),      // Gray 2
        RGB( 140, 255, 178 ),      // Light green
        RGB(  39, 148, 255 ),      // Light blue
        RGB( 196, 196, 196 )       // Gray 3
};

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.2.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Commodore 64 Koala");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.koa *.kla ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,0,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,188,112,121,76,76,76,150,112,46,59,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,89,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,145,83,167,134,78,13,5,50,66,129,172,169,83,65,140,153,48,6,72,36,20,166,6,36,2,211,5,55,7,100,41,216,100,46,176,59,22,48,0,0,106,186,58,102,90,65,81,220,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;
    
    pixel_mask[0] = 0xc0;
    pixel_mask[1] = 0x30;
    pixel_mask[2] = 0x0c;
    pixel_mask[3] = 0x03;

    pixel_displ[0] = 6;
    pixel_displ[1] = 4;
    pixel_displ[2] = 2;
    pixel_displ[3] = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    u8	load[2];
    
    if(!frs.readK(load, sizeof(load)))
	return SQE_R_BADFILE;

    if(load[0] != 0x00 || load[1] != 0x60)
    {
	((u8 *)&koala)[0] = load[0];
	((u8 *)&koala)[1] = load[1];

	if(!frs.readK((u8 *)&koala + 2, 10001 - 2))
	    return SQE_R_BADFILE;
    }
    else
    {
	if(!frs.readK((u8 *)&koala, 10001))
	    return SQE_R_BADFILE;
    }

    foundcolor = 0;

    image.w = KOALA_WIDTH;
    image.h = KOALA_HEIGHT;
    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(8);

    finfo.image.push_back(image);

    line = -1;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;
    fmt_image *im = image(currentImage);

    memset(scan, 255, im->w * sizeof(RGBA));

    for(s32 x = 0;x < KOALA_WIDTH / 2;x++)
    {
        index = (x / 4) * 8 + (line % 8) + (line / 8) * KOALA_WIDTH;
        colorindex = (x / 4) + (line / 8) * 40;
        pixel = (koala.image[index] & pixel_mask[x % 4]) >> pixel_displ[x % 4];

        // Retrieve RGB values
        switch(pixel)
        {
            case 0: // Background
                foundcolor = koala.background;
            break;

            case 1: // Color 1
                foundcolor = koala.color1[colorindex] >> 4;
            break;

            case 2: // Color 2
                foundcolor = koala.color1[colorindex] & 0xf;
            break;

            case 3: // Color 3
                foundcolor = koala.color2[colorindex] & 0xf;
            break;
        }

	memcpy(scan+x*2, c64pal+foundcolor, sizeof(RGB));
	memcpy(scan+x*2+1, c64pal+foundcolor, sizeof(RGB));
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

#include "fmt_codec_cd_func.h"

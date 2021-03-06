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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_koala_defs.h"
#include "fmt_codec_koala.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_koala.xpm"

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

void fmt_codec::options(codec_options *o)
{
    o->version = "0.2.2";
    o->name = "Commodore 64 Koala";
    o->filter = "*.koa *.kla ";
    o->config = "";
    o->mime = "\x0000\x0060";
    o->mimetype = "image/x-koala";
    o->pixmap = codec_koala;
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

s32 fmt_codec::read_next()
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
    image.bpp = 8;
    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(8);

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
    line++;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

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

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

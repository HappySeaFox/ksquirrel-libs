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
#include <cctype>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_sun_defs.h"
#include "fmt_codec_sun.h"

#include "../xpm/codec_sun.xpm"

/*
 *
 * The icons found in the Open Look and SunView Graphical User Interfaces available
 * on the Sun Microsystems UNIX-based platforms are stored in a simple format known
 * as the Sun Icon format
 *
 */

static const RGB mono[2] = { RGB(255,255,255), RGB(0,0,0) };

static const char * lex[5] = 
{
    "Format_version",
    "Width",
    "Height",
    "Depth",
    "Valid_bits_per_item"
};

// internal function
bool scanForLex(ifstreamK &f, bool digit = false);

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.9.1";
    o->name = "SUN Icon";
    o->filter = "*.sun ";
    o->config = "";
    o->mime = "/\\* Format_";
    o->mimetype = "image/x-sun";
    o->pixmap = codec_sun;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    s8 str[128];
    u32 var, ver;
    
    u32 * pointers[5] = 
    {
	&ver,
	(u32 *)&image.w,
	(u32 *)&image.h,
	(u32 *)&image.bpp,
	&validbits
    };

    for(s32 i = 0;i < 5;i++)
    {
	if(!scanForLex(frs))
	    return SQE_R_BADFILE;

	frs.getline(str, 127, '=');

	if(!frs.good())
	    return SQE_R_BADFILE;

	if(strncmp(lex[i], str, strlen(lex[i])))
	    return SQE_R_BADFILE;

	frs >> var;

	*(pointers[i]) = var;
    }

    if(ver != SUN_ICON_VERSION)
	return SQE_R_BADFILE;
	
    // ignore images with width != 64 or height != 64
    // TODO: get rid of this if()
    if(image.w != 64 || image.h != 64)
	return SQE_R_NOTSUPPORTED;

    if(image.bpp != 1)
	return SQE_R_NOTSUPPORTED;

    if(validbits != 16 && validbits != 32)
	return SQE_R_BADFILE;

    if(!scanForLex(frs, true))
	return SQE_R_BADFILE;

    image.compression = "-";
    image.colorspace = "Monochrome";

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
    u32 var, i = 0, j = 0;
    s32 decoded = 0;
    u8  indexes[32];
    bool lasthex;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);
    
    line++;

    while(decoded < im->w)
    {
	if(!frs.readCHex(var))
	    return SQE_R_BADFILE;

	if(validbits == 16)
	    fmt_utils::expandMono2Byte(var, indexes);
	else
	    fmt_utils::expandMono4Byte(var, indexes);

	decoded += validbits;

	lasthex = (decoded >= im->w && line == im->h-1);

	if(!scanForLex(frs, true) && !lasthex)
	    return SQE_R_BADFILE;

	j = i + validbits;

	for(u32 k = 0;i < j;i++,k++)
	    memcpy(scan+i, mono+indexes[k], sizeof(RGB));
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

bool scanForLex(ifstreamK &f, bool digit)
{
    u8 c;
    bool found = false;
    
    while(f.readK(&c, sizeof(u8)))
    {
//	cout << "Read " << c << endl;

	if((!digit && isalpha(c)) || (digit && isdigit(c)))
	{
	    found = true;
	    break;
	}
    }

    if(found)
	f.seekg(-1, ios::cur);

    return found;
}

#include "fmt_codec_cd_func.h"

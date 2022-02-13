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

#include "fmt_types.h"
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_sun_defs.h"
#include "fmt_codec_sun.h"

using namespace fmt_utils;

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.9.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("SUN Icon");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.ico ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("/. Format_");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,150,254,170,202,202,202,114,114,114,254,254,254,178,178,178,174,174,174,242,242,242,78,78,78,222,222,222,70,70,70,168,13,218,242,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,104,73,68,65,84,120,218,99,232,0,129,0,6,6,134,69,74,74,74,90,211,64,12,23,23,23,173,202,0,8,67,163,99,1,132,161,164,212,192,192,208,181,10,2,24,22,27,131,129,21,195,98,65,97,97,67,67,99,41,134,133,198,198,64,134,176,20,80,4,196,48,4,50,140,97,82,130,198,134,194,32,6,92,215,42,152,57,139,84,85,85,149,130,128,86,112,173,88,209,181,170,107,1,3,0,50,59,40,184,41,212,93,7,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(std::string file)
{
    frs.open(file.c_str(), ios::in);

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

    s8 str[128];
    u32 var, ver;
    
    u32 * pointers[5] = 
    {
	&ver,
	(u32 *)&finfo.image[currentImage].w,
	(u32 *)&finfo.image[currentImage].h,
	(u32 *)&finfo.image[currentImage].bpp,
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
    // @todo get rid of this if()
    if(finfo.image[currentImage].w != 64 || finfo.image[currentImage].h != 64)
	return SQE_R_NOTSUPPORTED;

    if(finfo.image[currentImage].bpp != 1)
	return SQE_R_NOTSUPPORTED;

    if(validbits != 16 && validbits != 32)
	return SQE_R_BADFILE;

    if(!scanForLex(frs, true))
	return SQE_R_BADFILE;

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = "Monochrome";

    line = -1;

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
    u32 var, i = 0, j = 0;
    s32 decoded = 0;
    u8  indexes[32];
    bool lasthex;
    
    line++;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    while(decoded < finfo.image[currentImage].w)
    {
	if(!frs.readCHex(var))
	    return SQE_R_BADFILE;

	if(validbits == 16)
	    fmt_utils::expandMono2Byte(var, indexes);
	else
	    fmt_utils::expandMono4Byte(var, indexes);

	decoded += validbits;

	lasthex = (decoded >= finfo.image[currentImage].w && line == finfo.image[currentImage].h-1);

	if(!scanForLex(frs, true) && !lasthex)
	    return SQE_R_BADFILE;

	j = i + validbits;

	for(u32 k = 0;i < j;i++,k++)
	    memcpy(scan+i, mono+indexes[k], sizeof(RGB));
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
}

s32 fmt_codec::fmt_write_init(std::string file, const fmt_image &image, const fmt_writeoptions &opt)
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

bool fmt_codec::fmt_readable() const
{
    return true;
}

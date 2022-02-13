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

#include <csetjmp>
#include <sstream>
#include <iostream>
#include <cctype>

#include "fmt_types.h"
#include "fmt_codec_sun_defs.h"
#include "fmt_codec_sun.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,65,0,0,0,192,192,192,255,255,255,112,112,112,0,255,0,4,4,4,40,229,232,128,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,89,73,68,65,84,120,218,99,96,96,72,3,1,6,32,16,82,82,82,82,3,49,146,140,141,141,213,210,160,12,160,144,32,20,48,136,184,128,129,35,131,72,104,72,136,171,171,75,32,131,168,139,11,144,17,18,8,20,1,49,92,129,12,23,152,84,168,139,107,8,136,1,215,5,55,7,98,178,49,200,178,36,37,168,173,96,103,36,0,0,248,95,25,196,95,2,95,106,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage)
        return SQERR_NOTOK;

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
	    return SQERR_BADFILE;

	frs.getline(str, 127, '=');

	if(!frs.good())
	    return SQERR_BADFILE;

	if(strncmp(lex[i], str, strlen(lex[i])))
	    return SQERR_BADFILE;

	frs >> var;

	*(pointers[i]) = var;
    }

    if(ver != SUN_ICON_VERSION)
	return SQERR_BADFILE;
	
    // ignore images with width != 64 or height != 64
    // @todo get rid of this if()
    if(finfo.image[currentImage].w != 64 || finfo.image[currentImage].h != 64)
	return SQERR_NOTSUPPORTED;

    if(finfo.image[currentImage].bpp != 1)
	return SQERR_NOTSUPPORTED;

    if(validbits != 16 && validbits != 32)
	return SQERR_BADFILE;

    if(!scanForLex(frs, true))
	return SQERR_BADFILE;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Monochrome" << "\n"
        << "-" << "\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();
    
    line = -1;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
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
	    return SQERR_BADFILE;

	if(validbits == 16)
	    fmt_utils::expandMono2Byte(var, indexes);
	else
	    fmt_utils::expandMono4Byte(var, indexes);

	decoded += validbits;

	lasthex = (decoded >= finfo.image[currentImage].w && line == finfo.image[currentImage].h-1);

	if(!scanForLex(frs, true) && !lasthex)
	    return SQERR_BADFILE;

	j = i + validbits;

	for(u32 k = 0;i < j;i++,k++)
	    memcpy(scan+i, mono+indexes[k], sizeof(RGB));
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;

    m_frs.open(file.c_str(), ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();
        return SQERR_BADFILE;
    }

    s8 str[128];
    u32 var, ver, m_validbits;

    u32 * pointers[5] = 
    {
	&ver,
	(u32 *)&w,
	(u32 *)&h,
	(u32 *)&bpp,
	&m_validbits
    };

    for(s32 i = 0;i < 5;i++)
    {
	if(!scanForLex(m_frs))
	    longjmp(jmp, 1);

	m_frs.getline(str, 127, '=');

	if(!m_frs.good())
	    longjmp(jmp, 1);

	if(strncmp(lex[i], str, strlen(lex[i])))
	    longjmp(jmp, 1);

	m_frs >> var;

	*(pointers[i]) = var;
    }

    if(ver != SUN_ICON_VERSION)
	longjmp(jmp, 1);
	
    // ignore images with width != 64 or height != 64
    // @todo get rid of this if()
    if(w != 64 || h != 64)
	longjmp(jmp, 1);

    if(bpp != 1)
	longjmp(jmp, 1);

    if(m_validbits != 16 && m_validbits != 32)
	longjmp(jmp, 1);

    if(!scanForLex(m_frs, true))
	longjmp(jmp, 1);

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Monocrome" << "\n"
        << "-" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    s32 m_line = -1;

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;

	RGB rgb;
	RGBA rgba;
	u32 var, i = 0, j = 0;
	s32 decoded = 0;
	u8  indexes[32];
	bool lasthex;

	m_line++;

	while(decoded < w)
	{
	    if(!m_frs.readCHex(var))
		longjmp(jmp, 1);

	    if(m_validbits == 16)
		fmt_utils::expandMono2Byte(var, indexes);
	    else
		fmt_utils::expandMono4Byte(var, indexes);

	    decoded += m_validbits;

	    lasthex = (decoded >= w && m_line == h-1);

	    if(!scanForLex(m_frs, true) && !lasthex)
		longjmp(jmp, 1);

	    j = i + m_validbits;

	    for(u32 k = 0;i < j;i++,k++)
		memcpy(scan+i, mono+indexes[k], sizeof(RGB));
	}
    }

    m_frs.close();

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &opt)
{
    return SQERR_OK;
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

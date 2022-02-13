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

#include "fmt_types.h"
#include "fmt_codec_rawrgb_defs.h"
#include "fmt_codec_rawrgb.h"

#include "error.h"

/*
 *
 * This is a codec to read and write internal raw image format.
 * This format has a simple header folowed by uncompressed image data in
 * 24 or 32 bit format. Width, height and bit depth are integers (unsigned int, or u32).
 *
 * File structure:
 *
 * <WIDTH><HEIGHT><BIT_DEPTH>
 * <UNCOMPRESSED DATA>
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    cerr << "libSQ_codec_rawrgb: reads internal uncompressed raw rgb format with bit depth 24 or 32" << endl;
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.0.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Raw uncompressed RGB image");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.rawrgb ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,57,192,192,192,255,255,255,0,0,0,84,106,142,237,236,253,4,4,4,175,149,124,179,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,84,73,68,65,84,120,218,61,142,193,17,192,32,8,4,175,5,62,22,64,11,137,21,8,21,136,255,228,99,255,37,40,200,184,175,157,99,56,0,48,29,108,126,34,42,33,204,92,102,202,142,158,4,175,4,13,117,136,138,73,71,21,115,186,39,41,98,195,84,219,25,185,220,173,219,115,154,185,228,209,184,26,111,124,11,149,31,27,232,188,121,46,5,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

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

    finfo.image[currentImage].passes = 1;

    u32 w, h, bpp;
    frs.readK(&w, sizeof(u32));
    frs.readK(&h, sizeof(u32));
    frs.readK(&bpp, sizeof(u32));
    
    if(bpp != 32 && bpp != 24)
	return SQERR_BADFILE;
    
    finfo.image[currentImage].w = w;
    finfo.image[currentImage].h = h;
    finfo.image[currentImage].bpp = bpp;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << (bpp == 24 ? "RGB" : "RGBA") << "\n"
        << "-\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

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

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    if(finfo.image[currentImage].bpp == 32)
	for(s32 i = 0;i < finfo.image[currentImage].w;i++)
	{
	    frs.readK(&rgba, sizeof(RGBA));
	    memcpy(scan+i, &rgba, sizeof(RGBA));
	}
    else
	for(s32 i = 0;i < finfo.image[currentImage].w;i++)
	{
	    frs.readK(&rgb, sizeof(RGB));
	    memcpy(scan+i, &rgb, sizeof(RGB));
	}

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    u32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();
        return SQERR_BADFILE;
    }

    m_frs.readK(&w, sizeof(u32));
    m_frs.readK(&h, sizeof(u32));
    m_frs.readK(&bpp, sizeof(u32));

    if(bpp != 32 && bpp != 24)
	longjmp(jmp, 1);

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << (bpp == 32 ? "RGBA" : "RGB") << "\n"
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

    RGB rgb;
    RGBA rgba;

    for(u32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;

	RGB rgb;
	RGBA rgba;

	if(bpp == 32)
	    for(u32 i = 0;i < w;i++)
	    {
		m_frs.readK(&rgba, sizeof(RGBA));
		memcpy(scan+i, &rgba, sizeof(RGBA));
	    }
	else
	    for(u32 i = 0;i < w;i++)
	    {
		m_frs.readK(&rgb, sizeof(RGB));
		memcpy(scan+i, &rgb, sizeof(RGB));
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
    u32 bpp = 32;
    RGBA *s;
    u8 *p;
    u8 a;

    ofstreamK m_frs;
    
    m_frs.open(file.c_str(), ios::out | ios::binary);
    
    if(!m_frs.good())
	return SQERR_NOFILE;

    m_frs.writeK((u32*)&w, sizeof(u32));
    m_frs.writeK((u32*)&h, sizeof(u32));
    if(!m_frs.writeK(&bpp, sizeof(u32))) return SQERR_BADFILE;

    for(int i = 0;i < h;i++)
    {
	s = image + i * w;

	for(int j = 0;j < w;j++)
	{
	    p = (u8 *)(s + j);

	    m_frs.writeK(p,   sizeof(u8));
	    m_frs.writeK(p+1, sizeof(u8));
	    m_frs.writeK(p+2, sizeof(u8));

	    a = (opt.alpha) ? *(p+3) : 255;

	    if(!m_frs.writeK(&a, sizeof(u8))) return SQERR_BADFILE;
	}
    }

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

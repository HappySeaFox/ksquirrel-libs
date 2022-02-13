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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_utah_defs.h"
#include "fmt_codec_utah.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,8,6,0,0,0,31,243,255,97,0,0,0,186,73,68,65,84,120,218,165,147,49,15,130,48,16,133,191,154,254,47,150,206,116,198,196,24,227,228,224,239,113,54,38,50,151,249,6,248,95,36,117,48,96,105,41,18,120,211,245,122,239,122,185,190,167,174,247,11,33,158,143,151,103,5,206,183,147,2,208,241,69,223,247,180,109,187,72,54,198,140,113,210,64,235,111,170,40,138,89,114,215,117,147,73,15,108,128,136,32,34,0,40,192,179,3,26,160,124,151,155,200,205,177,249,237,192,85,110,188,176,181,29,115,67,156,171,153,44,49,71,156,171,25,48,89,162,171,220,34,57,158,34,105,96,107,139,173,109,82,180,122,130,220,139,97,195,248,172,0,191,231,23,118,235,64,205,153,73,68,178,82,14,37,109,140,73,189,16,107,126,149,18,151,220,246,207,210,31,160,33,78,92,252,132,72,89,0,0,0,0,73,69,78,68,174,66,96,130,130");
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

    if(!frs.readK(&utah, sizeof(UTAH_HEADER))) return SQERR_BADFILE;

    if(utah.magic != UTAH_MAGIC)  return SQERR_BADFILE;

    if(utah.ncolors != 3 && utah.ncolors != 4) return SQERR_NOTSUPPORTED;

    if(utah.ncmap != 1 && utah.ncmap != 0 && utah.ncmap != utah.ncolors) return SQERR_BADFILE;

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

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Color indexed" << "\n"
        << "RLE" << "\n"
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


    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
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

/*
    w = 
    h = 
    bpp = 
*/
    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "??" << "\n"
        << "??" << "\n"
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

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;


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


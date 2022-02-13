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


/*
 *  The Wireless Application Protocol Bitmap Format (WBMP) is designed for use 
 *  with applications that operate over wireless communication networks.
 *  The WBMP format is commonly used in mobile phones (WAP phones) and enables
 *  graphical information to be sent to the handset.
 *  This format is very simple and allows to store image only in 1-bit
 *  format (black and white).
*/


#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_wbmp_defs.h"
#include "fmt_codec_wbmp.h"

#include "error.h"

/*
 *
 *  WBMP: Wireless Bitmap Type 0: B/W, Uncompressed Bitmap
 *  Specification of the WBMP format can be found in the 
 *  SPEC-WAESpec-19990524.pdf
 *
 *  You can download the WAP specification on: http://www.wapforum.com/
 *
 */

//#define SQ_HAVE_FMT_UTILS
//#include "fmt_utils.h"

static const RGB mono[2] = {  RGB(255,255,255), RGB(0,0,0) };

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
    return std::string("Wireless Application Protocol Bitmap");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.wbmp ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,0,0,0,192,192,192,255,255,255,185,167,0,4,4,4,244,94,246,60,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,99,96,96,8,5,1,6,32,16,82,82,82,82,5,49,130,140,141,141,85,67,161,12,160,144,32,20,48,136,184,128,129,35,131,176,139,179,179,177,179,139,33,131,176,51,8,24,67,24,96,17,17,19,19,32,19,202,128,136,192,116,193,205,129,152,108,12,178,44,72,9,106,43,216,25,1,0,44,21,22,188,130,146,75,57,0,0,0,0,73,69,78,68,174,66,96,130,130");
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

    u8 type;

    frs.readK(&type, sizeof(u8));
    
    wbmp.type = type;

    if(wbmp.type != 0)
	return SQERR_BADFILE;

    if(skipheader(frs))
	return SQERR_BADFILE;

    wbmp.width = getmbi(frs);

    if(wbmp.width == -1)
	return SQERR_BADFILE;

    wbmp.height = getmbi(frs);

    if(wbmp.height == -1)
	return SQERR_BADFILE;

    finfo.image[currentImage].w = wbmp.width;
    finfo.image[currentImage].h = wbmp.height;
    finfo.image[currentImage].bpp = 1;

    wbmp.bitmap = new s32 [wbmp.width * wbmp.height];

    if(!wbmp.bitmap)
	return SQERR_NOMEMORY;

    s32 row, col, byte, pel, pos;
    u8 b;

    pos = 0;

    for(row = 0;row < wbmp.height;row++)
    {
        for(col = 0;col < wbmp.width;)
        {
            if(!frs.readK(&b, sizeof(u8)))
		return SQERR_BADFILE;

	    byte = b;

            for(pel = 7;pel >= 0;pel--)
            {
                if(col++ < wbmp.width)
                {
                    if(byte & 1 << pel)
                        wbmp.bitmap[pos] = WBMP_WHITE;
                    else
                        wbmp.bitmap[pos] = WBMP_BLACK;

                    pos++;
                }
            }
	}
    }

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Monochrome" << "\n"
        << "-\n"
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

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
    line++;

    for(int i = 0;i < finfo.image[currentImage].w;i++)
	memcpy(scan+i, mono + (wbmp.bitmap[line * finfo.image[currentImage].w + i]), sizeof(RGB));

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;
    Wbmp		_w;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    _w.bitmap = 0;

    if(setjmp(jmp))
    {
        m_frs.close();
	
        if(_w.bitmap)
	    delete [] _w.bitmap;

        return SQERR_BADFILE;
    }

    u8 type;

    m_frs.readK(&type, sizeof(u8));
    
    _w.type = type;

    if(_w.type != 0)
	longjmp(jmp, 1);

    if(skipheader(m_frs))
	longjmp(jmp, 1);

    _w.width = getmbi(m_frs);

    if(_w.width == -1)
	longjmp(jmp, 1);

    _w.height = getmbi(m_frs);

    if(wbmp.height == -1)
	longjmp(jmp, 1);

    w = _w.width;
    h = _w.height;
    bpp = 1;

    _w.bitmap = new s32 [_w.width * _w.height];

    if(!_w.bitmap)
	longjmp(jmp, 1);

    s32 row, col, byte, pel, pos;
    u8 b;

    pos = 0;

    for(row = 0;row < _w.height;row++)
    {
        for(col = 0;col < _w.width;)
        {
            if(!m_frs.readK(&b, sizeof(u8)))
		longjmp(jmp, 1);;

	    byte = b;

            for(pel = 7;pel >= 0;pel--)
            {
                if(col++ < _w.width)
                {
                    if(byte & 1 << pel)
                        _w.bitmap[pos] = WBMP_WHITE;
                    else
                        _w.bitmap[pos] = WBMP_BLACK;

                    pos++;
                }
            }
	}
    }

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Monochrome" << "\n"
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

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;

	for(int i = 0;i < w;i++)
	    memcpy(scan+i, mono + (_w.bitmap[h2 * w + i]), sizeof(RGB));
    }

    m_frs.close();
    
    delete [] _w.bitmap;

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    frs.close();

    if(wbmp.bitmap)
        delete [] wbmp.bitmap;

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

s32 fmt_codec::fmt_writeimage(std::string, RGBA *, s32, s32, const fmt_writeoptions &)
{
    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

/*
 *
 * These functions were taken from Wbmp program by Johan Van den Brande
 *
 * (c) 2000 Johan Van den Brande <johan@vandenbrande.com>
 *
 */

s32 fmt_codec::getmbi(ifstreamK &f)
{
    s32 mbi = 0;
    s8 i;

    do
    {
        f.readK(&i, sizeof(s8));

        if(i < 0)
            return -1;

        mbi = (mbi << 7) | (i & 0x7f);

    }while(i & 0x80);

    return mbi;
}

s32 fmt_codec::putmbi(s32 i, ofstreamK &f)
{
    s32 cnt, l, accu;
    u8 s;

    /* Get number of septets */
    cnt = 0;
    accu = 0;

    while(accu != i)
        accu += i & 0x7f << 7*cnt++;

    /* Produce the multibyte output */
    for(l = cnt-1;l > 0;l--)
    {
	s = 0x80 | (i & 0x7f << 7*l ) >> 7*l;
        f.writeK(&s, sizeof(s8));
    }

    s = i & 0x7f; 

    f.writeK(&s, sizeof(u8));

    return 0;
}

s32 fmt_codec::skipheader(ifstreamK &f)
{
    s8 i;
    bool b;

    do
    {
        b = f.readK(&i, sizeof(s8));

	if(!b)
	    return -1;

        if(i < 0)
                return -1;

    }while(i & 0x80);

    return 0;
}

Wbmp* fmt_codec::createwbmp(s32 width, s32 height, s32 color)
{
    s32     i;

    Wbmp *_wbmp = new Wbmp;

    if(!_wbmp)
        return NULL;

    _wbmp->bitmap = new s32 [width * height];

    if(!_wbmp->bitmap)
    {
        delete _wbmp;
        return NULL;
    }

    _wbmp->width = width;
    _wbmp->height= height;

    for(i = 0;i < width * height;_wbmp->bitmap[i++] = color)
    {}

    return _wbmp;
}

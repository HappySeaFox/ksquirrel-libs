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


#include <iostream>

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_wbmp_defs.h"
#include "fmt_codec_wbmp.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,185,167,0,76,76,76,217,123,198,192,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,51,167,78,13,157,58,51,130,97,233,84,16,8,133,48,192,34,43,35,35,129,76,40,3,34,2,211,5,55,7,100,41,216,100,46,176,59,22,48,0,0,96,54,57,234,103,230,193,140,0,0,0,0,73,69,78,68,174,66,96,130");
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
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    u8 type;

    frs.readK(&type, sizeof(u8));
    
    wbmp.type = type;

    if(wbmp.type != 0)
	return SQE_R_BADFILE;

    if(skipheader(frs))
	return SQE_R_BADFILE;

    wbmp.width = getmbi(frs);

    if(wbmp.width == -1)
	return SQE_R_BADFILE;

    wbmp.height = getmbi(frs);

    if(wbmp.height == -1)
	return SQE_R_BADFILE;

    finfo.image[currentImage].w = wbmp.width;
    finfo.image[currentImage].h = wbmp.height;
    finfo.image[currentImage].bpp = 1;

    wbmp.bitmap = new s32 [wbmp.width * wbmp.height];

    if(!wbmp.bitmap)
	return SQE_R_NOMEMORY;

    s32 row, col, byte, pel, pos;
    u8 b;

    pos = 0;

    for(row = 0;row < wbmp.height;row++)
    {
        for(col = 0;col < wbmp.width;)
        {
            if(!frs.readK(&b, sizeof(u8)))
		return SQE_R_BADFILE;

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

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
    line++;

    for(int i = 0;i < finfo.image[currentImage].w;i++)
	memcpy(scan+i, mono + (wbmp.bitmap[line * finfo.image[currentImage].w + i]), sizeof(RGB));

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
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

bool fmt_codec::fmt_readable() const
{
    return true;
}
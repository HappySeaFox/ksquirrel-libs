/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>

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
#include "fmt_codec_cut_defs.h"
#include "fmt_codec_cut.h"

#include "error.h"

/*
 *
 * The Dr. Halo file format is a device-independent interchange format used for
 * transporting image data from one hardware environment or operating system to
 * another. This format is associated with the HALO Image File Format
 * Library.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.0-a1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Dr. Halo CUT");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.cut ");
}

std::string fmt_codec::fmt_mime()
{
/*  QRegExp pattern  */
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,0,0,0,192,192,192,255,255,255,132,74,163,248,228,243,4,4,4,64,16,98,114,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,81,73,68,65,84,120,218,99,96,96,72,3,1,6,32,16,82,82,82,82,3,49,146,140,141,141,213,210,160,12,160,144,32,20,48,136,184,128,129,35,131,136,107,136,171,171,107,40,144,1,162,93,66,64,12,23,24,3,38,2,84,19,10,102,192,116,193,205,129,152,108,12,178,44,73,9,106,43,216,25,9,0,225,141,25,116,164,218,131,113,0,0,0,0,73,69,78,68,174,66,96,130,130");
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
    s16 width, height;
    s32 dummy;

    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    for(s32 i = 0;i < 256;i++)
	memset(pal+i, i, sizeof(RGB));	

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&width, sizeof(s16))) return SQERR_BADFILE;
    if(!frs.readK(&height, sizeof(s16))) return SQERR_BADFILE;
    if(!frs.readK(&dummy, sizeof(s32))) return SQERR_BADFILE;

    finfo.image[currentImage].w = width;
    finfo.image[currentImage].h = height;
    finfo.image[currentImage].bpp = 8;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
    
    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Color indexed" << "\n"
        << "RLE\n"
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
//    static s32 s = 0;
//    printf("Reading scan %d\n", s++);
    s32 count = 0;
    u8 c, run;
    
    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
    while(count < finfo.image[currentImage].w)
    {
	if(!frs.readK(&c, 1)) return SQERR_BADFILE;

	if(!c)
	{
	    frs.readK(&c, 1);
	    if(!frs.readK(&c, 1)) return SQERR_BADFILE;
	    break;
	}
	else if(c & 0x80)
	{
	    c &= ~(0x80);

	    if(!frs.readK(&run, 1)) return SQERR_BADFILE;

	    count += c;

	    for(s32 i = 0;i < c;i++)
		memcpy(scan+i, pal+run, sizeof(RGB));
	}
	else
	{
	    count += c;

	    for(s32 i = 0;i < c;i++)
	    {
		if(!frs.readK(&run, 1)) return SQERR_BADFILE;
		memcpy(scan+i, pal+run, sizeof(RGB));
	    }
	}
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;
    RGB			m_pal[256];

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();
        return SQERR_BADFILE;
    }

    for(s32 i = 0;i < 256;i++)
	memset(m_pal+i, i, sizeof(RGB));	

    s16 width, height;
    s32 dummy;

    if(!m_frs.readK(&width, sizeof(s16))) longjmp(jmp, 1);
    if(!m_frs.readK(&height, sizeof(s16))) longjmp(jmp, 1);
    if(!m_frs.readK(&dummy, sizeof(s32))) longjmp(jmp, 1);
    
    w = width;
    h = height;
    bpp = 1;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Color indexed" << "\n"
        << "RLE" << "\n"
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

        s32 count = 0;
	s8 c, run;

        memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
	while(count < finfo.image[currentImage].w)
	{
	    if(!m_frs.readK(&c, 1)) longjmp(jmp, 1);

	    if(!c)
	    {
		m_frs.readK(&c, 1);
		if(!m_frs.readK(&c, 1)) longjmp(jmp, 1);
		break;
	    }
	    else if(c & 0x80)
	    {
		c &= ~(0x80);

		if(!m_frs.readK(&run, 1)) longjmp(jmp, 1);

		count += c;

		for(s32 i = 0;i < c;i++)
		    memcpy(scan+i, pal+run, sizeof(RGB));
	    }
	    else
	    {
		count += c;

		for(s32 i = 0;i < c;i++)
		{
		    if(!m_frs.readK(&run, 1)) longjmp(jmp, 1);
		    memcpy(scan+i, pal+run, sizeof(RGB));
		}
	    }
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
    opt->compression_scheme = CompressionInternal;
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

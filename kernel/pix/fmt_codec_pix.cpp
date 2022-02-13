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
#include "fmt_codec_pix_defs.h"
#include "fmt_codec_pix.h"

#include "error.h"

/*
 *
 * This format is sourced on IRIX
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.1.0");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Irix PIX image");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.pix ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,204,51,4,4,4,195,151,166,176,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,76,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,144,97,236,98,98,98,2,98,152,152,64,25,64,17,103,176,148,9,92,10,198,128,234,130,155,3,49,89,73,20,106,41,216,86,176,51,2,0,148,190,24,56,148,160,248,187,0,0,0,0,73,69,78,68,174,66,96,130,130");
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
    u16 tmp;
    PIX_HEADER	pfh;

    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    if(!frs.be_getshort(&pfh.width)) return SQERR_BADFILE;
    if(!frs.be_getshort(&pfh.height)) return SQERR_BADFILE;

    if(!frs.readK(&tmp, sizeof(u16))) return SQERR_BADFILE;
    if(!frs.readK(&tmp, sizeof(u16))) return SQERR_BADFILE;

    if(!frs.be_getshort(&pfh.bpp)) return SQERR_BADFILE;

    if(pfh.bpp != 24)	
	return SQERR_BADFILE;

    finfo.image[currentImage].w = pfh.width;
    finfo.image[currentImage].h = pfh.height;
    finfo.image[currentImage].bpp = pfh.bpp;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
	<< finfo.image[currentImage].w << "x"
	<< finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
	<< "RGB" << "\n"
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
    s32	len = 0, i, counter = 0;
    u8 count;
    RGB	rgb;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
	case 24:
	    do
	    {
		if(!frs.readK(&count, 1)) return SQERR_BADFILE;
		len += count;
		
		if(!frs.readK(&rgb.b, 1)) return SQERR_BADFILE;
		if(!frs.readK(&rgb.g, 1)) return SQERR_BADFILE;
		if(!frs.readK(&rgb.r, 1)) return SQERR_BADFILE;

		for(i = 0;i < count;i++)
		    memcpy(scan+counter++, &rgb, 3);
		    
	    }while(len < finfo.image[currentImage].w);
	break;

    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    PIX_HEADER		pfh;
    s32 		w, h, bpp;
    u16 	tmp;
    s32 		m_bytes;
    jmp_buf		jmp;
    ifstreamK		m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	m_frs.close();
	return SQERR_BADFILE;
    }

    if(!m_frs.be_getshort(&pfh.width)) longjmp(jmp, 1);
    if(!m_frs.be_getshort(&pfh.height)) longjmp(jmp, 1);

    if(!m_frs.readK(&tmp, sizeof(u16))) longjmp(jmp, 1);
    if(!m_frs.readK(&tmp, sizeof(u16))) longjmp(jmp, 1);

    if(!m_frs.be_getshort(&pfh.bpp)) longjmp(jmp, 1);

    if(pfh.bpp != 24)	
	longjmp(jmp, 1);

    w = pfh.width;
    h = pfh.height;
    bpp = pfh.bpp;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "RGB" << "\n"
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
	s32 len = 0, i, counter = 0;
        u8	count;
	RGB		rgb;
        RGBA 	*scan = *image + h2 * w;

        switch(bpp)
	{
	    case 24:
		do
    		{
		    if(!m_frs.readK(&count, 1)) longjmp(jmp, 1);
		    len += count;

		    if(!m_frs.readK(&rgb.b, 1)) longjmp(jmp, 1);
		    if(!m_frs.readK(&rgb.g, 1)) longjmp(jmp, 1);
		    if(!m_frs.readK(&rgb.r, 1)) longjmp(jmp, 1);

		    for(i = 0;i < count;i++)
			memcpy(scan+counter++, &rgb, sizeof(RGB));

		}while(len < w);
	    break;
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

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

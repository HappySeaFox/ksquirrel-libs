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
#include "fmt_codec_xcur_defs.h"
#include "fmt_codec_xcur.h"

#include "error.h"

/*
 *
 * Library to support X cursors.
 * 
 * You can test it with nice cursors from http://kde-apps.org
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.4.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X Cursors");
}

std::string fmt_codec::fmt_filter()
{
    return std::string();
}

std::string fmt_codec::fmt_mime()
{
    return std::string("Xcur");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,0,0,0,192,192,192,255,255,255,255,255,0,4,4,4,37,60,155,71,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,99,96,96,8,5,1,6,32,16,82,82,82,82,5,49,130,140,141,141,85,67,161,12,160,144,32,20,48,136,184,128,129,35,131,160,35,58,67,68,196,81,208,209,209,81,16,40,226,232,2,100,128,164,28,5,93,128,4,146,46,152,57,16,147,141,65,150,5,41,65,109,5,59,35,0,0,30,33,22,117,242,237,176,248,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;
    currentToc = -1;

    if(!frs.readK(&xcur_h, sizeof(XCUR_HEADER))) return SQERR_BADFILE;

    tocs = new XCUR_CHUNK_DESC [xcur_h.ntoc];

    if(!frs.readK(tocs, sizeof(XCUR_CHUNK_DESC) * xcur_h.ntoc)) return SQERR_BADFILE;

    lastToc = false;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(lastToc)
    {
	finfo.animated = (currentToc > 0);
	return SQERR_NOTOK;
    }

    do
    {
	 currentToc++;
    }
    while(tocs[currentToc].type != XCUR_CHUNK_TYPE_IMAGE && currentToc < (s32)xcur_h.ntoc);

    if(currentToc == (s32)xcur_h.ntoc-1)
	lastToc = true;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;    

    frs.seekg(tocs[currentToc].pos, ios::beg);

    if(!frs.readK(&xcur_chunk, sizeof(XCUR_CHUNK_HEADER))) return SQERR_BADFILE;
    if(!frs.readK(&xcur_im, sizeof(XCUR_CHUNK_IMAGE))) return SQERR_BADFILE;
    
    finfo.image[currentImage].w = xcur_im.width;
    finfo.image[currentImage].h = xcur_im.height;
    finfo.image[currentImage].bpp = 32;
    finfo.image[currentImage].delay = xcur_im.delay;
    finfo.image[currentImage].hasalpha = true;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
    
    std::string type("ARGB");

    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << type << "\n"
	<< "-" << "\n"
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
    RGBA rgba;

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
    {
	if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

	(scan+i)->r = rgba.b;
	(scan+i)->g = rgba.g;
	(scan+i)->b = rgba.r;
	(scan+i)->a = rgba.a;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 		w, h, bpp;
    XCUR_HEADER 	m_xcur_h;
    XCUR_CHUNK_HEADER 	m_xcur_chunk;
    XCUR_CHUNK_IMAGE 	m_xcur_im;
    s32 		m_currentToc;
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

    m_currentToc = -1;
    
    if(!m_frs.readK(&m_xcur_h, sizeof(XCUR_HEADER))) longjmp(jmp, 1);

    XCUR_CHUNK_DESC m_tocs[m_xcur_h.ntoc];

    if(!m_frs.readK(m_tocs, sizeof(XCUR_CHUNK_DESC) * m_xcur_h.ntoc)) longjmp(jmp, 1);

    do
    {
	 m_currentToc++;
    }
    while(m_tocs[m_currentToc].type != XCUR_CHUNK_TYPE_IMAGE && m_currentToc < (s32)m_xcur_h.ntoc);

    m_frs.seekg(m_tocs[m_currentToc].pos, ios::beg);
    
    if(!m_frs.readK(&m_xcur_chunk, sizeof(XCUR_CHUNK_HEADER))) longjmp(jmp, 1);
    if(!m_frs.readK(&m_xcur_im, sizeof(XCUR_CHUNK_IMAGE))) longjmp(jmp, 1);

    w = m_xcur_im.width;
    h = m_xcur_im.height;
    bpp = 32;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "ARGB" << "\n"
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

    for(s32 h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	RGBA rgba;

	for(s32 i = 0;i < w;i++)
	{
	    if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

	    (scan+i)->r = rgba.b;
	    (scan+i)->g = rgba.g;
	    (scan+i)->b = rgba.r;
	    (scan+i)->a = rgba.a;
	}
    }
    
    m_frs.close();
    
    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    frs.close();

    if(tocs)
	delete [] tocs;
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

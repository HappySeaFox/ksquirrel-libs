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
#include "fmt_codec_wmf_defs.h"
#include "fmt_codec_wmf.h"

#include "error.h"

/*
 *
 * Microsoft Windows Metafile Format (WMF) files are used to store both
 * vector and bitmap-format graphical data in memory or in disk files.
 * The vector data stored in WMF files is described as Microsoft Windows
 * Graphics Device Interface (GDI) commands. In the Window environment
 * these commands are interpreted and played back on an output device
 * using the Windows API PlayMetaFile() function. Bitmap data stored in
 * a WMF file may be stored in the form of a Microsoft Device Dependent
 * Bitmap (DDB), or Device Independent Bitmap (DIB).
 *
 */

extern int call(int, char **, unsigned char **, int *, int *);

fmt_codec::fmt_codec() : fmt_codec_base()
{
    cerr << "libSQ_codec_wmf: using libwmf 0.2.8.3" << endl;
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.9.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Windows Metafile");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.wmf ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,15,80,76,84,69,25,0,0,192,192,192,255,255,255,0,0,0,4,4,4,109,150,124,48,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,72,73,68,65,84,120,218,99,96,96,112,1,1,6,32,112,20,20,20,20,1,51,148,148,148,68,92,160,12,160,144,49,20,32,24,70,198,70,70,198,202,74,202,96,134,146,178,50,178,136,17,136,1,20,1,18,80,6,134,118,136,201,74,34,80,75,193,182,130,157,225,0,0,119,2,20,9,72,251,91,108,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    frs.close();

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    const char * argv[] = 
    {
	"wmf2gd",
	file.c_str()
    };

    call(2, (char **)argv, &buf, &w, &h);

    if(!buf)
	return SQERR_NOMEMORY;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage)
        return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    finfo.image[currentImage].bpp = 32;
    finfo.image[currentImage].w = w;
    finfo.image[currentImage].h = h;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Vectorized RGB" << "\n"
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
    
    line++;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    memcpy(scan, buf + line * finfo.image[currentImage].w * sizeof(RGBA), finfo.image[currentImage].w * sizeof(RGBA));

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;
    u8			*m_buf;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    m_frs.close();

    m_buf = 0;

    if(setjmp(jmp))
    {
        if(m_buf)
	    delete [] m_buf;

        return SQERR_BADFILE;
    }

    const char * argv[] = 
    {
	"wmf2gd",
	file.c_str()
    };

    call(2, (char **)argv, &m_buf, &w, &h);

    if(!m_buf)
	longjmp(jmp, 1);

    bpp = 32;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Vectorized RGB" << "\n"
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

    int line = -1;

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;

	line++;

	memcpy(scan, m_buf + line * w * sizeof(RGBA), w * sizeof(RGBA));
    }

    delete [] m_buf;

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    finfo.meta.clear();
    finfo.image.clear();

    delete [] buf;
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
    return SQERR_NOTSUPPORTED;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

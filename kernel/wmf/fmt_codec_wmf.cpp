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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_wmf_defs.h"
#include "fmt_codec_wmf.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,0,0,0,76,76,76,38,185,199,251,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,95,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,51,167,78,157,25,25,26,193,176,116,234,212,169,161,145,145,51,192,12,160,200,12,134,149,145,145,64,70,36,50,3,166,11,110,14,200,82,176,201,92,96,119,44,96,0,0,55,98,57,74,221,72,115,50,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    frs.close();

    currentImage = -1;

    finfo.animated = false;

    const char * argv[] = 
    {
	"wmf2gd",
	file.c_str()
    };

    call(2, (char **)argv, &buf, &w, &h);

    if(!buf)
	return SQE_R_NOMEMORY;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    image.bpp = 32;
    image.w = w;
    image.h = h;
    image.compression = "-";
    image.colorspace = "Vectorized RGB";

    finfo.image.push_back(image);

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
    
    line++;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    memcpy(scan, buf + line * finfo.image[currentImage].w * sizeof(RGBA), finfo.image[currentImage].w * sizeof(RGBA));

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
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
    opt->passes = 1;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
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

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

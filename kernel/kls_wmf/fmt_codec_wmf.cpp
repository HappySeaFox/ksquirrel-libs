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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_wmf_defs.h"
#include "fmt_codec_wmf.h"

#include "../xpm/codec_wmf.xpm"

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
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.9.0";
    o->name = "Windows Metafile";
    o->filter = "*.wmf ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_wmf;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
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

    buf = NULL;

    call(2, (char **)argv, &buf, &w, &h);

    if(!buf)
	return SQE_R_NOMEMORY;

    return SQE_OK;
}

s32 fmt_codec::read_next()
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

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);
    
    line++;

    memcpy(scan, buf + line * im->w * sizeof(RGBA), im->w * sizeof(RGBA));

    return SQE_OK;
}

void fmt_codec::read_close()
{
    finfo.meta.clear();
    finfo.image.clear();

    delete [] buf;
    buf = NULL;
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
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

s32 fmt_codec::write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
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

s32 fmt_codec::write_next()
{
    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA * /*scan*/)
{
    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string();
}

#include "fmt_codec_cd_func.h"

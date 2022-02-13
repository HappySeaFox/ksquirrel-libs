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

#include <iostream>

#include <tiffio.h>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_tiff_defs.h"
#include "fmt_codec_tiff.h"

#include "../xpm/codec_tiff.xpm"

/*
 *
 * The TIFF specification was originally released in
 * 1986 by Aldus Corporation as a standard method of storing
 * black-and-white images created by scanners and desktop publishing
 * applications. This first public release of TIFF was
 * the third major revision of the TIFF format, and
 * although it was not assigned a specific version number, this release
 * may be thought of as TIFF Revision 3.0.  The first
 * widely used revision of TIFF, 4.0, was released in
 * April 1987. TIFF 4.0 added support for uncompressed
 * RGB color images and was quickly followed by the
 * release of TIFF Revision 5.0 in August
 * 1988. TIFF 5.0 was the first revision to add the
 * capability of storing palette color images and support for the
 * LZW compression algorithm. TIFF 6.0 was released in June
 * 1992 and added support for CMYK and YCbCr color
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "1.0.1";
    o->name = "Tagged Image File Format";
    o->filter = "*.tif *.tiff ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_tiff;
    o->readable = true;
    o->canbemultiple = true;
    o->writestatic = true;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    currentImage = -1;

    if((ftiff = TIFFOpen(file.c_str(), "r")) == NULL)
	return SQE_R_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

    finfo.animated = false;

    dircount = 0;

    while(TIFFReadDirectory(ftiff))
    {
	dircount++;
    }

//    printf("dircount: %d\n", dircount);

//    if(dircount > 1)
//	dircount = 1;

    TIFFSetDirectory(ftiff, 0);

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(dircount)
    {
	if(currentImage == dircount)
	    return SQE_NOTOK;
    }
    else
	if(currentImage)
	    return SQE_NOTOK;

    if(dircount != 1 && dircount != 0)
	if(!TIFFReadDirectory(ftiff))
	    return SQE_R_BADFILE;

    if(currentImage)
	TIFFRGBAImageEnd(&img);

    fmt_image image;

    s32 bps, spp;

    TIFFGetField(ftiff, TIFFTAG_IMAGEWIDTH,  &image.w);
    TIFFGetField(ftiff, TIFFTAG_IMAGELENGTH, &image.h);

    memset(&img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&img, ftiff, 1, 0);

    bps = img.bitspersample;
    spp = img.samplesperpixel;

//    printf("bps: %d, spp: %d\n", bps, spp);

    image.bpp = bps * spp;
    image.hasalpha = true;
    image.compression = "-"; 
    image.colorspace = fmt_utils::colorSpaceByBpp(image.bpp);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}
    
s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    const s32 W = im->w * sizeof(RGBA);

    uint32 buf[W];
    
    TIFFRGBAImageGet(&img, buf, im->w, 1);

    memcpy(scan, buf, W);

    img.row_offset++;

    return SQE_OK;
}

void fmt_codec::read_close()
{
    TIFFRGBAImageEnd(&img);
    TIFFClose(ftiff);

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionRLE;
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

    out = TIFFOpen(file.c_str(), "w");

    if(!out)
	return SQE_W_NOFILE;

    return SQE_OK;
}

s32 fmt_codec::write_next()
{
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH,  writeimage.w);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, writeimage.h);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(out, TIFFTAG_COMPRESSION, (writeopt.compression_scheme == CompressionRLE ? COMPRESSION_PACKBITS : COMPRESSION_NONE));
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, (uint32) -1));
    
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    ++line;

    if(TIFFWriteScanline(out, (u8 *)scan, line, 0) < 0)
	return SQE_W_ERROR;

    return SQE_OK;
}

void fmt_codec::write_close()
{
    TIFFClose(out);
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("tiff");
}

#include "fmt_codec_cd_func.h"

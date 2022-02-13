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

#include "fmt_types.h"
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_tiff_defs.h"
#include "fmt_codec_tiff.h"

using namespace fmt_utils;

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

std::string fmt_codec::fmt_version()
{
    return std::string("1.0.1");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Tagged Image File Format");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.tiff *.tif ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string(); // "II|MM" is too common to be a regexp :-)
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,0,0,153,76,76,76,230,241,105,213,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,87,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,145,161,83,67,129,140,169,51,35,35,167,206,132,48,66,167,70,162,137,64,24,48,93,112,115,64,150,130,77,230,2,187,99,1,3,0,64,118,57,214,21,155,128,128,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
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

s32 fmt_codec::fmt_read_next()
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

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}
    
s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    const s32 W = im->w * sizeof(RGBA);

    uint32 buf[W];
    
    memset(scan, 255, W);

    TIFFRGBAImageGet(&img, buf, im->w, 1);

    memcpy(scan, buf, W);

    img.row_offset++;

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    TIFFRGBAImageEnd(&img);
    TIFFClose(ftiff);

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
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

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
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

s32 fmt_codec::fmt_write_next()
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

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    ++line;

    if(TIFFWriteScanline(out, (u8 *)scan, line, 0) < 0)
	return SQE_W_ERROR;

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    TIFFClose(out);
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("tiff");
}

#include "fmt_codec_cd_func.h"

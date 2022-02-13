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

#include <tiffio.h>

#include "fmt_types.h"
#include "fmt_codec_tiff_defs.h"
#include "fmt_codec_tiff.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

#include "error.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,0,153,4,4,4,41,96,241,199,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,79,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,10,16,42,1,25,78,46,78,46,42,46,80,134,146,10,170,136,146,10,152,1,211,5,55,7,98,178,146,40,212,82,176,173,96,103,4,0,0,107,22,23,177,172,1,179,111,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    currentImage = -1;

    if((ftiff = TIFFOpen(file.c_str(), "r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

    finfo.animated = false;
    finfo.images = 0;

    dircount = 0;

    while(TIFFReadDirectory(ftiff))
    {
	dircount++;
    }

//    printf("dircount: %d\n", dircount);

//    if(dircount > 1)
//	dircount = 1;

    TIFFSetDirectory(ftiff, 0);

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(dircount)
    {
	if(currentImage == dircount)
	    return SQERR_NOTOK;
    }
    else
	if(currentImage)
	    return SQERR_NOTOK;

    if(dircount != 1 && dircount != 0)
	if(!TIFFReadDirectory(ftiff))
	    return SQERR_BADFILE;

    if(currentImage)
	TIFFRGBAImageEnd(&img);

    finfo.image.push_back(fmt_image());

    s32 bps, spp;

    TIFFGetField(ftiff, TIFFTAG_IMAGEWIDTH, &finfo.image[currentImage].w);
    TIFFGetField(ftiff, TIFFTAG_IMAGELENGTH, &finfo.image[currentImage].h);

    memset(&img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&img, ftiff, 1, 0);

    bps = img.bitspersample;
    spp = img.samplesperpixel;

//    printf("bps: %d, spp: %d\n", bps, spp);

    finfo.image[currentImage].bpp = bps * spp;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.image[currentImage].hasalpha = true;

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
	<< finfo.image[currentImage].w << "x"
	<< finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
	<< fmt_utils::colorSystemByBpp(finfo.image[currentImage].bpp) << "\n"
	<< "-\n"
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
    const s32 W = finfo.image[currentImage].w * sizeof(RGBA);

    uint32 buf[W];
    
    memset(scan, 255, W);

    TIFFRGBAImageGet(&img, buf, finfo.image[currentImage].w, 1);

    memcpy(scan, buf, W);

    img.row_offset++;

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    TIFF	*m_ftiff;
    TIFFRGBAImage m_img;
    s32 m_bytes;
    s32 w, h, bpp;

    if((m_ftiff = TIFFOpen(file.c_str(), "r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

    s32 m_dircount = 0;

    while(TIFFReadDirectory(m_ftiff))
    {
	m_dircount++;
    }

    TIFFSetDirectory(m_ftiff, 0);

    if(m_dircount != 1 && m_dircount != 0)
    if(!TIFFReadDirectory(m_ftiff))
    {
	TIFFClose(m_ftiff);
        return SQERR_BADFILE;
    }

    s32 bps, spp;

    TIFFGetField(m_ftiff, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(m_ftiff, TIFFTAG_IMAGELENGTH, &h);

    memset(&m_img, 0, sizeof(TIFFRGBAImage));

    TIFFRGBAImageBegin(&m_img, m_ftiff, 1, 0);
    
    bps = m_img.bitspersample;
    spp = m_img.samplesperpixel;

    bpp = bps * spp;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << fmt_utils::colorSystemByBpp(bpp) << "\n"
        << "-" << "\n"
        << m_dircount << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
	TIFFRGBAImageEnd(&m_img);
	TIFFClose(m_ftiff);
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    const s32 W = w * sizeof(RGBA);

    uint32 buf[W];

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

	memset(scan, 255, W);

	TIFFRGBAImageGet(&m_img, buf, w, 1);

	memcpy(scan, buf, W);

	m_img.row_offset++;
    }

    TIFFRGBAImageEnd(&m_img);
    TIFFClose(m_ftiff);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &opt)
{
    TIFF *out;
    u32 spp = 4;

    out = TIFFOpen(file.c_str(), "w");

    if(!out)
	return SQERR_NOFILE;

    TIFFSetField(out, TIFFTAG_IMAGEWIDTH,  w);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, spp);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(out, TIFFTAG_COMPRESSION, (opt.compression_scheme == CompressionRLE ? COMPRESSION_PACKBITS : COMPRESSION_NONE));
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, (uint32) -1));
//    TIFFWriteDirectory(out);

    for(s32 row = 0; row < h; row++)
    {
	RGBA *scan = image + row * w;

        if(TIFFWriteScanline(out, (u8 *)scan, row, 0) < 0)
    	    break;
    }

    TIFFClose(out);

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

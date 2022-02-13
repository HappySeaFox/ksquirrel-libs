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

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include <jasper/jasper.h>
#include "fmt_codec_jpeg2000_defs.h"
#include "fmt_codec_jpeg2000.h"

#include "error.h"

/*
 *
 *    JPEG 2000 standard supports lossy and lossless compression of
 *    single-component (e.g., grayscale) and multicomponent (e.g., color)
 *    imagery.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    cerr << "libSQ_codec_jpeg2000: using jasper 1.701.0" << endl;
    jas_init();
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.3.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("JPEG 2000");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.jp2 *.j2k ");
}

std::string fmt_codec::fmt_mime()
{
    // some jp2 files don't have this mime header (why ?)
    // "....\152\120\040\040"
    return std::string(); 
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,128,63,0,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,207,98,234,76,76,76,170,148,76,211,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,87,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,32,35,50,20,8,193,12,16,128,138,204,4,49,128,220,153,83,65,106,66,103,70,206,132,168,129,234,130,155,3,178,20,108,50,23,216,29,11,24,0,230,11,56,50,77,27,163,148,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    in = jas_stream_fopen(file.c_str(), "rb");

    if(!in)
	return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

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

    image = jas_image_decode(in, -1, 0);

    jas_stream_close(in);
    
    if(!image)
	return SQE_R_NOMEMORY;

    gs.image = image;

    family = jas_clrspc_fam(jas_image_clrspc(gs.image));

    if(!convert_colorspace())
	return SQE_R_BADFILE;

    finfo.image[currentImage].w = jas_image_width(gs.image);
    finfo.image[currentImage].h = jas_image_height(gs.image);

    finfo.images++;

    switch(family)
    {
	case JAS_CLRSPC_FAM_RGB:
	    finfo.image[currentImage].colorspace = "RGB";
	    finfo.image[currentImage].bpp = 24;
	break;

	case JAS_CLRSPC_FAM_YCBCR:
	    finfo.image[currentImage].colorspace = "YCbCr";
	    finfo.image[currentImage].bpp = 24;
	break;
		
	case JAS_CLRSPC_FAM_GRAY:
	    finfo.image[currentImage].colorspace = "Grayscale";
	    finfo.image[currentImage].bpp = 8;
	break;

	case JAS_CLRSPC_FAM_LAB:
	    finfo.image[currentImage].colorspace = "LAB";
	    finfo.image[currentImage].bpp = 24;
	break;
		
	default:
	    finfo.image[currentImage].colorspace = "Unknown";
	    finfo.image[currentImage].bpp = 0;
    }

    finfo.image[currentImage].compression = "JPEG2000";

    if((gs.cmptlut[0] = jas_image_getcmptbytype(gs.altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
       (gs.cmptlut[1] = jas_image_getcmptbytype(gs.altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
       (gs.cmptlut[2] = jas_image_getcmptbytype(gs.altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0)

	return SQE_R_NOMEMORY;

    const s32 *cmptlut = gs.cmptlut;

    // check that all components have the same size.
    const s32 width = jas_image_cmptwidth(gs.altimage, cmptlut[0]);
    const s32 height = jas_image_cmptheight(gs.altimage, cmptlut[0]);

    for(s32 i = 1; i < 3; ++i)
    {
	if(jas_image_cmptwidth(gs.altimage, cmptlut[i]) != width ||
		jas_image_cmptheight(gs.altimage, cmptlut[i]) != height)

	return SQE_R_BADFILE;
    }

    line = -1;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 v[3];
    const s32* cmptlut = gs.cmptlut;

    line++;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    u8 *data = (u8 *)scan;

    for(s32 x = 0; x < finfo.image[currentImage].w;++x)
    {
	for(int k = 0; k < 3; ++k)
	{
		v[k] = jas_image_readcmptsample(gs.altimage, cmptlut[k], x, line);

		// if the precision of the component is too small, increase
		// it to use the complete value range.
		v[k] <<= 8 - jas_image_cmptprec(gs.altimage, cmptlut[k]);

		if(v[k] < 0) v[k] = 0;
		else if(v[k] > 255) v[k] = 255;
	}

	*data = v[0];
	*(data+1) = v[1];
	*(data+2) = v[2];

	data += 4;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    if(gs.image) jas_image_destroy(gs.image);
    if(gs.altimage) jas_image_destroy(gs.altimage);

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->passes = 1;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 100;
    opt->compression_def = 50;
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

s32 fmt_codec::fmt_write_scanline(RGBA * /*scan*/)
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

// helper method
bool fmt_codec::convert_colorspace()
{
    jas_cmprof_t *outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB);

    if(!outprof)
	return false;
			    
    gs.altimage = jas_image_chclrspc(gs.image, outprof, JAS_CMXFORM_INTENT_PER);

    if(!gs.altimage)
	return false;

    return true;
}
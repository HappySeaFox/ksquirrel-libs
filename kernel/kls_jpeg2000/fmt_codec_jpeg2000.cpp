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
#include "ksquirrel-libs/fmt_utils.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_jpeg2000.xpm"

/*
 *
 *    JPEG 2000 standard supports lossy and lossless compression of
 *    single-component (e.g., grayscale) and multicomponent (e.g., color)
 *    imagery.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    jas_init();
}

fmt_codec::~fmt_codec()
{
    jas_cleanup();
}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.3.1";
    o->name = "JPEG 2000";
    o->filter = "*.jp2 *.j2k ";

    // some jp2 files don't have this mime header (why ?)
    // "....\152\120\040\040" => o->mime is empty
    o->mime = "";
    o->mimetype = "image/jp2";
    o->config = "";
    o->pixmap = codec_jpeg2000;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    in = jas_stream_fopen(file.c_str(), "rb");

    if(!in)
	return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    jp2_image = jas_image_decode(in, -1, 0);

    jas_stream_close(in);
    
    if(!jp2_image)
	return SQE_R_NOMEMORY;

    gs.image = jp2_image;

    family = jas_clrspc_fam(jas_image_clrspc(gs.image));

    if(!convert_colorspace())
	return SQE_R_BADFILE;

    image.w = jas_image_width(gs.image);
    image.h = jas_image_height(gs.image);

    switch(family)
    {
	case JAS_CLRSPC_FAM_RGB:
	    image.colorspace = "RGB";
	    image.bpp = 24;
	break;

	case JAS_CLRSPC_FAM_YCBCR:
	    image.colorspace = "YCbCr";
	    image.bpp = 24;
	break;
		
	case JAS_CLRSPC_FAM_GRAY:
	    image.colorspace = "Grayscale";
	    image.bpp = 8;
	break;

	case JAS_CLRSPC_FAM_LAB:
	    image.colorspace = "LAB";
	    image.bpp = 24;
	break;
		
	default:
	    image.colorspace = "Unknown";
	    image.bpp = 0;
    }

    image.compression = "JPEG2000";

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
    s32 v[3];
    const s32* cmptlut = gs.cmptlut;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    line++;

    u8 *data = (u8 *)scan;

    for(s32 x = 0; x < im->w;++x)
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

void fmt_codec::read_close()
{
    if(gs.image) jas_image_destroy(gs.image);
    if(gs.altimage) jas_image_destroy(gs.altimage);

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->passes = 1;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 100;
    opt->compression_def = 50;
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

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string();
}

#include "fmt_codec_cd_func.h"

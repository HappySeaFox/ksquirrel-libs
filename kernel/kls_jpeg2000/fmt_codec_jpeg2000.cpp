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
    o->version = "0.4.0";
    o->name = "JPEG 2000";
    o->filter = "*.jp2 *.j2k ";

    // mime is "....\152\120\040\040",
    // but some jp2 files don't have this mime header (why ?)
    //  => o->mime is empty
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
    gs.image = 0;
    gs.altimage = 0;
    gs.data[0] = 0;
    gs.data[1] = 0;
    gs.data[2] = 0;

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

    gs.image = jas_image_decode(in, -1, 0);

    jas_stream_close(in);

    if(!gs.image)
	return SQE_R_NOMEMORY;

    s32 family = jas_clrspc_fam(jas_image_clrspc(gs.image));

    if(!convert_colorspace())
	return SQE_R_BADFILE;

    jas_image_destroy(gs.image);
    gs.image = gs.altimage;
    gs.altimage = 0;

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

    if((gs.cmptlut[0] = jas_image_getcmptbytype(gs.image, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
       (gs.cmptlut[1] = jas_image_getcmptbytype(gs.image, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
       (gs.cmptlut[2] = jas_image_getcmptbytype(gs.image, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0)

	return SQE_R_NOMEMORY;

    const s32 *cmptlut = gs.cmptlut;

    // check that all components have the same size.
    const s32 width = jas_image_cmptwidth(gs.image, cmptlut[0]);
    const s32 height = jas_image_cmptheight(gs.image, cmptlut[0]);

    for(s32 i = 1; i < 3; ++i)
    {
	if(jas_image_cmptwidth(gs.image, cmptlut[i]) != width ||
		jas_image_cmptheight(gs.image, cmptlut[i]) != height)

	return SQE_R_BADFILE;
    }

    for(s32 i = 0;i < 3;i++)
    {
        if(!(gs.data[i] = jas_matrix_create(1, image.w)))
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
    fmt_image *im = image(currentImage);
    jas_seqent_t v;

    fmt_utils::fillAlpha(scan, im->w);

    line++;

    u8 *data = (u8 *)scan;

    for(s32 cmptno = 0; cmptno < 3; ++cmptno)
    {
        if(jas_image_readcmpt(gs.image, gs.cmptlut[cmptno], 0, line, im->w, 1, gs.data[cmptno]))
            return SQE_R_BADFILE;

        gs.d[cmptno] = jas_matrix_getref(gs.data[cmptno], 0, 0);
    }

    for(s32 x = 0; x < im->w;++x)
    {
	for(int k = 0; k < 3; ++k)
	{
            v = *gs.d[k];

	    if(v < 0)
                v = 0;
    	    else if(v > 255)
                v = 255;

	    *data = v;
            data++;

            ++gs.d[k];
	}

	data++;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    for(s32 cmptno = 0; cmptno < 3; ++cmptno)
    {
        if (gs.data[cmptno])
            jas_matrix_destroy(gs.data[cmptno]);
    }

    if(gs.image) jas_image_destroy(gs.image);
    if(gs.altimage) jas_image_destroy(gs.altimage);

    finfo.meta.clear();
    finfo.image.clear();
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

    jas_cmprof_destroy(outprof);

    return true;
}

#include "fmt_codec_cd_func.h"

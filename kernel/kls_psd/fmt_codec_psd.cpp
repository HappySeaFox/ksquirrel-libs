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
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_psd_defs.h"
#include "fmt_codec_psd.h"

#include "../xpm/codec_psd.xpm"

/*
 *
 * Adobe's Photoshop is probably the fullest featured and most highly
 * respected commercial image-processing bitmap manipulation program in
 * the PC and Macintosh worlds. Its wide distribution
 * has meant that image data is often left in PSD
 * format files and may persist in this form after the original image
 * data is long gone.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.8.1";
    o->name = "Adobe Photoshop PSD";
    o->filter = "*.psd ";
    o->config = "";
    o->mime = "\x0038\x0042\x0050\x0053\x0001";
    o->mimetype = "image/psd;image/x-vnd.adobe.photoshop";
    o->pixmap = codec_psd;
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
	
    currentImage = -1;
    layer = -1;
    
    u32 ident;
    u16 ver;

    if(!frs.be_getlong(&ident))
	return SQE_R_BADFILE;

    if(ident != 0x38425053)
	return SQE_R_NOTSUPPORTED;

    if(!frs.be_getshort(&ver))
	return SQE_R_BADFILE;

    if(ver != 1)
	return SQE_R_BADFILE;

    last = 0;
    L = 0;

    s8 dummy[6];
    if(!frs.readK(dummy, 6)) return SQE_R_BADFILE;

    if(!frs.be_getshort(&channels))
	return SQE_R_BADFILE;

    if(!frs.be_getlong(&height))
	return SQE_R_BADFILE;

    if(!frs.be_getlong(&width))
	return SQE_R_BADFILE;

    if(!frs.be_getshort(&depth))
	return SQE_R_BADFILE;

    if(!frs.be_getshort(&mode))
	return SQE_R_BADFILE;

    if(depth != 8)
	return SQE_R_NOTSUPPORTED;

    if(mode != PSD_RGB && mode != PSD_CMYK && mode != PSD_INDEXED && mode != PSD_GRAYSCALE)
	return SQE_R_NOTSUPPORTED;

    if(mode == PSD_RGB && channels != 3 && channels != 4)
	return SQE_R_NOTSUPPORTED;

    if(mode == PSD_CMYK && channels != 4 && channels != 5)
	return SQE_R_NOTSUPPORTED;

    if(mode == PSD_INDEXED && channels != 1)
	return SQE_R_NOTSUPPORTED;

    u32 data_count;

    if(!frs.be_getlong(&data_count))
	return SQE_R_BADFILE;

    if(data_count)
    {
	if(!frs.readK(pal, 256 * sizeof(RGB))) return SQE_R_BADFILE;
    }

    if(!frs.be_getlong(&data_count))
	return SQE_R_BADFILE;

    if(data_count)
	frs.seekg(data_count, ios::cur);

    if(!frs.be_getlong(&data_count))
	return SQE_R_BADFILE;

    if(data_count)
	frs.seekg(data_count, ios::cur);

    // find out if the data is compressed
    //   0: no compressiod
    //   1: RLE compressed

    if(!frs.be_getshort(&compression))
	return SQE_R_BADFILE;

    if(compression != 1 && compression != 0)
	return SQE_R_NOTSUPPORTED;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    image.hasalpha = (mode == PSD_RGB) ? true : ((channels == 5) ? true : false);
    image.passes = (channels == 5) ? 4 : channels;
    image.h = height;
    image.w = width;
    
    if(compression)
    {
	u16 b[height * channels];

	if(!frs.readK(b, 2 * height * channels)) return SQE_R_BADFILE;
    }

    std::string type;
    
    switch(mode)
    {
	case PSD_RGB:
	    type = "RGB";
	    image.bpp = 24;
	break;

	case PSD_CMYK:
	    type = "CMYK";
	    image.bpp = (channels == 5) ? 32 : 24;
	break;
	
	case PSD_INDEXED:
	    type = "Color indexed";
	    image.bpp = 8;
	break;

	case PSD_GRAYSCALE:
	    type = "Grayscale";
	    image.bpp = 8;
	break;
    }

    last = (RGBA **)calloc(height, sizeof(RGBA*));

    if(!last)
        return SQE_R_NOMEMORY;

    const s32 S = width * sizeof(RGBA);

    for(u32 i = 0;i < height;i++)
    {
	last[i] = (RGBA*)0;
    }

    for(u32 i = 0;i < height;i++)
    {
	last[i] = (RGBA*)malloc(S);

	if(!last[i])
	    return SQE_R_NOMEMORY;
	    
	memset(last[i], 255, S);
    }
    
    line = -1;

    L = (u8*)calloc(width, 1);
    
    if(!L)
	return SQE_R_NOMEMORY;

    image.compression = ((compression) ? "RLE" : "-");
    image.colorspace = type;

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    layer++;
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    u8 c, value, *p;
    s32 count = 0;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);
    
    line++;

    memcpy(scan, last[line], im->w * sizeof(RGBA));

    if(compression)
    {
	while(count < im->w)
	{
	    if(!frs.readK(&c, 1)) return SQE_R_BADFILE;

	    if(c == 128)
	    {} // do nothing
	    else if(c > 128)
	    {
		c ^= 0xff;
		c += 2;

		if(!frs.readK(&value, 1)) return SQE_R_BADFILE;

		for(s32 i = count; i < count+c;i++)
		{
		    p = (u8*)(scan+i);
		    *(p+layer) = value;
		}

		count += c;
	    }
	    else if(c < 128)
	    {
		c++;

		for(s32 i = count; i < count+c;i++)
		{
		    if(!frs.readK(&value, 1)) return SQE_R_BADFILE;

		    p = (u8*)(scan+i);
		    *(p+layer) = value;
		}

		count += c;
	    }
	}
    }
    else
    {
	if(!frs.readK(L, width)) return SQE_R_BADFILE;

	for(u32 i = 0;i < width;i++)
	{
	    p = (u8*)(scan+i);
	    *(p+layer) = L[i];
	}
    }

    memcpy(last[line], scan, im->w * sizeof(RGBA));

    if(layer == im->passes-1)
    {
	if(mode == PSD_CMYK)
	{
	    for(s32 i = 0;i < im->w;i++)
	    {
		scan[i].r = (scan[i].r * scan[i].a) >> 8;
		scan[i].g = (scan[i].g * scan[i].a) >> 8;
		scan[i].b = (scan[i].b * scan[i].a) >> 8;
	    
		if(channels == 4)
		    scan[i].a = 255;
	    }
	}
	else if(mode == PSD_INDEXED)
	{
	    u8 r;
	    const s32 z1 = 768/3;
	    const s32 z2 = z1 << 1;

	    for(s32 i = 0;i < im->w;i++)
	    {
		u8 *p = (u8*)pal;
		r = scan[i].r;

		(scan+i)->r = *(p+r);
		(scan+i)->g = *(p+r+z1);
		(scan+i)->b = *(p+r+z2);
		scan[i].a = 255;
	    }	    
	}
	else if(mode == PSD_GRAYSCALE)
	{
	    u8 v;

	    for(s32 i = 0;i < im->w;i++)
	    {
		v = scan[i].r;

		(scan+i)->r = v;
		(scan+i)->g = v;
		(scan+i)->b = v;
		scan[i].a = 255;
	    }	    
	}
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    if(last)
    {
	for(u32 i = 0;i < height;i++)
	{
	    if(last[i])
		free(last[i]);
	}

	free(last);
    }
    
    finfo.meta.clear();
    finfo.image.clear();

    if(L)
	free(L);
}

#include "fmt_codec_cd_func.h"

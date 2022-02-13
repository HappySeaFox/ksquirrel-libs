/*  This file is part of ksquirrel (http://ksquirrel.sf.net)

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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_ico_defs.h"
#include "fmt_codec_ico.h"

#include "../xpm/codec_ico.xpm"

/*
 *
 * An icon-resource file contains image data for icons used by Windows
 * applications. The file consists of an icon directory identifying the number
 * and types of icon images in the file, plus one or more icon images. The
 * default filename extension for an icon-resource file is .ICO.
 *
*/

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.4.2";
    o->name = "Windows icons";
    o->filter = "*.ico *.cur ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-ico";
    o->pixmap = codec_ico;
    o->readable = true;
    o->canbemultiple = true;
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

    pal_entr = 0;
    bAND = 0;

    if(!frs.readK(&ifh, sizeof(ICO_HEADER)))
	return SQE_R_BADFILE;

    if(ifh.idType != 1 && ifh.idType != 2)
	return SQE_R_BADFILE;
	
    ide = (ICO_DIRENTRY*)calloc(ifh.idCount, sizeof(ICO_DIRENTRY));

    if(!ide)
	return SQE_R_NOMEMORY;

    if(!frs.readK(ide, sizeof(ICO_DIRENTRY) * ifh.idCount))
	return SQE_R_BADFILE;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage == ifh.idCount)
	return SQE_NOTOK;

    fmt_image image;

    RGBA	rgba;
    s32		i;
    fstream::pos_type	pos;

    image.w = ide[currentImage].bWidth;
    image.h = ide[currentImage].bHeight;
    
    frs.seekg(ide[currentImage].dwImageOffset, ios::beg);

    if(!frs.readK(&bih, sizeof(BITMAPINFO_HEADER)))
	return SQE_R_BADFILE;

    image.bpp = bih.BitCount;

    if(image.bpp < 16)
    {
	pal_entr = 1 << image.bpp;

	for(i = 0;i < pal_entr;i++)
	{
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		pal[i].r = rgba.b;
		pal[i].g = rgba.g;
		pal[i].b = rgba.r;
	}
    }
    else
    {
	pal_entr = 0;
    }

    pos = frs.tellg();

    s32 count = image.w * image.h;
    s32 count2 = (image.bpp < 16) ? (count / (8 / image.bpp)) : (count * (image.bpp / 8));
    s32 count3 = count / 8;

    frs.seekg(/*ide[currentImage].dwBytes - sizeof(BITMAPINFO_HEADER) - */count2, ios::cur);

    bAND = (u8 *)realloc(bAND, count * sizeof(u8));

    if(!bAND)
        return SQE_R_NOMEMORY;

    u8 realAND[count3];

    if(!frs.readK(realAND, count3)) return SQE_R_BADFILE;

    s32 r = 0;

    for(i = 0;i < count3;i++)
    {
	for(s32 z = 0,f = 128;z < 8;z++,f >>= 1)
	{
	    bAND[r] = (realAND[i] & f) ? 1 : 0;
	    r++;
	}
    }

    frs.seekg(pos);

    image.needflip = true;
    image.hasalpha = true;

    image.compression = "-";
    image.colorspace = ((pal_entr) ? "Color indexed":"RGB");

    finfo.image.push_back(image);

    pixel = 0;

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}
    
s32 fmt_codec::read_scanline(RGBA *scan)
{
    s32 i, j, count;
    u8 bt, ind;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(im->bpp)
    {
    	case 1:
	    j = im->w / 8;
	    count = 0;

	    for(i = 0;i < j;i++)
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		for(s32 z = 0, f = 128;z < 8;z++,f >>= 1)
		{
		    ind = (bt & f) ? 1 : 0;

		    memcpy(scan+count, pal+ind, sizeof(RGB));

		    count++;
		    pixel++;
		}
	    }
	break;

	case 4:
	    j = 0;
	    do
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		ind = bt >> 4;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
		ind = bt&15;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
	    }while(j < im->w);

	break;

	case 8:
	    for(i = 0;i < im->w;i++)
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		memcpy(scan+i, pal+bt, sizeof(RGB));
//		(scan+i)->a = (bAND[pixel]) ? 0 : 255;
		pixel++;
	    }
	break;
	
	case 24:
	{
	    RGB rgb;
	    
	    for(i = 0;i < im->w;i++)
	    {
		if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

		(scan+i)->r = rgb.b;
		(scan+i)->g = rgb.g;
		(scan+i)->b = rgb.r;
		pixel++;
	    }
	}
	break;
	
	case 32:
	{
	    RGBA rgba;
	    
	    for(i = 0;i < im->w;i++)
	    {
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		(scan+i)->r = rgba.b;
		(scan+i)->g = rgba.g;
		(scan+i)->b = rgba.r;
		(scan+i)->a = rgba.a;
		pixel++;
	    }
	}
	break;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    if(bAND)
	free(bAND);

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

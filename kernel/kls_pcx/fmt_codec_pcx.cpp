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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_pcx_defs.h"
#include "fmt_codec_pcx.h"

#include "../xpm/codec_pcx.xpm"

/*
 *
 * PCX is one of the most widely used storage
 * formats. It originated with ZSoft's MS-DOS-based
 * PC Paintbrush, and because of this,
 * PCX is sometimes referred to as the
 * PC Paintbrush format. ZSoft entered into an
 * OEM arrangement with Microsoft, which allowed
 * Microsoft to bundle PC Paintbrush with various
 * products, including a version called Microsoft Paintbrush for Windows;
 * this product was distributed with every copy of Microsoft Windows
 * sold. This distribution established the importance of
 * PCX, not only on Intel-based
 * MS-DOS platforms, but industry-wide.
 *
 */

bool getrow(ifstreamK &s, u8*, int);

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.8.1";
    o->name = "ZSoft PCX";
    o->filter = "*.pcx ";
    o->config = "";
    o->mime = "\x000A[\x0002\x0003\x0004\x0005]";
    o->mimetype = "image/x-pcx";
    o->pixmap = codec_pcx;
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
    pal_entr = 0;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
	return SQE_NOTOK;
	    
    fmt_image image;

    if(!frs.readK(&pfh, sizeof(PCX_HEADER))) return SQE_R_BADFILE;

    if(pfh.ID != 10 || pfh.Encoding != 1)
	return SQE_R_BADFILE;

    image.w = pfh.Xmax - pfh.Xmin + 1;
    image.h = pfh.Ymax - pfh.Ymin + 1;
    image.bpp = pfh.bpp * pfh.NPlanes;
    pal_entr = 0;

    if(pfh.bpp == 1)
    {
	pal_entr = 2;

	memset(pal, 0, sizeof(RGB));
	memset(pal+1, 255, sizeof(RGB));

    }
    else if(pfh.bpp <= 4)
    {
	pal_entr = 16;

	memcpy(pal, pfh.Palette, 48);
    }
    else if(pfh.bpp == 8 && pfh.NPlanes == 1)
    {
	pal_entr = 256;

	frs.seekg(-769, ios::end);

	s8 test;
	if(!frs.readK(&test, 1)) return SQE_R_BADFILE;

	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQE_R_BADFILE;

	if(!frs.readK(pal, 768)) return SQE_R_BADFILE;
    }

    frs.seekg(128, ios::beg);

    TotalBytesLine = pfh.NPlanes * pfh.BytesPerLine;

    image.compression = "-";
    image.colorspace = ((pal_entr) ? "Color indexed":"RGB");

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    u16  i, j;
    u8 channel[4][finfo.image[currentImage].w];
    u8 indexes[finfo.image[currentImage].w];
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(i = 0;i < 4;i++)
	memset(channel[i], 255, im->w);

    switch(im->bpp)
    {
    	case 1:
	{
	}
	break;

	case 4:
	{
	}
	break;

	case 8:
	    if(!getrow(frs, indexes, pfh.BytesPerLine))
		return SQE_R_BADFILE;

	    for(i = 0;i < im->w;i++)
		memcpy(scan+i, pal+indexes[i], sizeof(RGB));
	break;

	case 16:
	{
	}
	break;

	case 24:
	{
	    for(j = 0;j < pfh.NPlanes;j++)
	    {
		if(!getrow(frs, channel[j], pfh.BytesPerLine))
		    return SQE_R_BADFILE;
	    }

	    for(i = 0;i < im->w;i++)
	    {
    		scan[i].r = channel[0][i];
    		scan[i].g = channel[1][i];
    		scan[i].b = channel[2][i];
	    }
	}
	break;

	default:;
    }

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

/* helper function */
bool getrow(ifstreamK &f, u8 *pcxrow, s32 bytesperline)
{
    static s32 	repetitionsLeft = 0;
    static u8	c;
    s32 	bytesGenerated;

    bytesGenerated = 0;
    while(bytesGenerated < bytesperline)
    {
        if(repetitionsLeft > 0)
	{
            pcxrow[bytesGenerated++] = c;
            --repetitionsLeft;
        }
	else
	{
	    if(!f.readK(&c, 1)) return false;

	    if(c <= 192)
                pcxrow[bytesGenerated++] = c;
            else
	    {
                repetitionsLeft = c&63;
		if(!f.readK(&c, 1)) return false;
            }
        }
    }

    return true;
}

#include "fmt_codec_cd_func.h"

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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_pcx_defs.h"
#include "fmt_codec_pcx.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.8.1");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("ZSoft PCX");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.pcx ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("\x000A[\x0002\x0003\x0004\x0005]");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,128,128,128,76,76,76,100,182,213,138,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,83,67,167,78,5,50,34,167,130,0,68,100,230,204,72,144,200,76,168,8,144,1,81,3,211,5,55,7,100,41,216,100,46,176,59,22,48,0,0,122,158,58,186,129,16,211,111,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    pal_entr = 0;

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

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&pfh, sizeof(PCX_HEADER))) return SQE_R_BADFILE;

    if(pfh.ID != 10 || pfh.Encoding != 1)
	return SQE_R_BADFILE;

    finfo.image[currentImage].w = pfh.Xmax - pfh.Xmin + 1;
    finfo.image[currentImage].h = pfh.Ymax - pfh.Ymin + 1;
    finfo.image[currentImage].bpp = pfh.bpp * pfh.NPlanes;
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
//	s32 i;
//	for(i=0;i<256;i++)
//	printf("%d %d %d\n",(finfo.image[currentImage].pal)[i].r,(finfo.image[currentImage].pal)[i].g,(finfo.image[currentImage].pal)[i].b);
    }

    frs.seekg(128, ios::beg);
/*    
    printf("ID: %d\nVersion: %d\nEncoding: %d\nbpp: %d\nNPlanes: %d\nBytesPerLine: %d\nPaletteInfo: %d\n",
    pfh.ID, pfh.Version, pfh.Encoding, pfh.bpp, pfh.NPlanes, pfh.BytesPerLine, pfh.PaletteInfo);
*/
    TotalBytesLine = pfh.NPlanes * pfh.BytesPerLine;

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = ((pal_entr) ? "Color indexed":"RGB");

    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u16  i, j;
    u8 channel[4][finfo.image[currentImage].w];
    u8 indexes[finfo.image[currentImage].w];

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    
    for(i = 0;i < 4;i++)
	memset(channel[i], 255, finfo.image[currentImage].w);

    switch(finfo.image[currentImage].bpp)
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

	    for(i = 0;i < finfo.image[currentImage].w;i++)
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

	    for(i = 0;i < finfo.image[currentImage].w;i++)
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

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal; // maybe set to CompressionRLE ?
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

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
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

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

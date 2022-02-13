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
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_iff_defs.h"
#include "fmt_codec_iff.h"

#include "../xpm/codec_iff.xpm"

/*
 *
 * IFF (Interchange File Format) is a general purpose data storage
 * format that can associate and store multiple types of data.
 * IFF is portable and has many well-defined extensions that support
 * still-picture, sound, music, video, and textual data.  Because of
 * this extensibility, IFF has fathered a family of special purpose file
 * formats all based on IFF's simple data structure.
 *
 */

bool skip_unknown(ifstreamK &f, u32 waiting);

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.1";
    o->name = "Interchange File Format";
    o->filter = "*.iff *.ilbm ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_iff;
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

    finfo.animated = false;
    pal = NULL;
    tile = NULL;
    dline = NULL;

    return SQE_OK;
}

unsigned char *src;
unsigned char *dest;
unsigned pixel_size;
unsigned n_width;
unsigned plane_size;
unsigned src_size;
unsigned width = 0, height = 0, planes = 0, depth = 0, comp = 0;

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    CHUNK_HEAD chunk;
    u32 ilbm;
    
    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQE_R_BADFILE;

    if(chunk.type != IFF_FORM) return SQE_R_NOTSUPPORTED;

    if(!frs.readK(&ilbm, 4)) return SQE_R_BADFILE;

    if(ilbm != IFF_ILBM) return SQE_R_NOTSUPPORTED;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQE_R_BADFILE;

    if(chunk.type != IFF_BMHD) return SQE_R_NOTSUPPORTED;

    if(!frs.readK(&bmhd, sizeof(CHUNK_BMHD))) return SQE_R_BADFILE;

    if(!skip_unknown(frs, IFF_CMAP)) return SQE_R_BADFILE;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQE_R_BADFILE;

    if(chunk.type != IFF_CMAP) return SQE_R_NOTSUPPORTED;

    pal_entr = fmt_utils::konvertLong(chunk.size) / 3;

    printf("Masking: %d\nCompress: %d\n", bmhd.Masking, bmhd.Compress);

    printf("pal_entr: %d\n", pal_entr);

    pal = new RGB [pal_entr];

    if(!pal)
	return SQE_R_NOMEMORY;

    if(!frs.readK(pal, pal_entr * sizeof(RGB))) return SQE_R_BADFILE;
    
    for(s32 i = 0;i < pal_entr;i++)
	printf("PAL %d,%d,%d\n", pal[i].r, pal[i].g, pal[i].b);

    if(!skip_unknown(frs, IFF_BODY)) return SQE_R_BADFILE;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQE_R_BADFILE;

    if(chunk.type != IFF_BODY) return SQE_R_BADFILE;

    bmhd.Width = fmt_utils::konvertWord(bmhd.Width);
    bmhd.Height = fmt_utils::konvertWord(bmhd.Height);
    bmhd.Left = fmt_utils::konvertWord(bmhd.Left);
    bmhd.Top = fmt_utils::konvertWord(bmhd.Top);
    bmhd.Transparency = fmt_utils::konvertWord(bmhd.Transparency);
    bmhd.PageWidth = fmt_utils::konvertWord(bmhd.PageWidth);
    bmhd.PageHeight = fmt_utils::konvertWord(bmhd.PageHeight);

    tile = new u8 * [bmhd.Bitplanes];

    if(!tile)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < bmhd.Bitplanes;i++)
	tile[i] = NULL;

    for(s32 i = 0;i < bmhd.Bitplanes;i++)
    {
	tile[i] = new u8 [bmhd.Width];
	
	if(!tile[i])
	    return SQE_R_NOMEMORY;
    }

    dline = new u8 [bmhd.Width];

    if(!dline)
	return SQE_R_NOMEMORY;

    image.w = bmhd.Width;
    image.h = bmhd.Height;
    image.bpp = 8;

    printf("%dx%d@%d, transparent: %d\n", bmhd.Width, bmhd.Height, bmhd.Bitplanes, bmhd.Transparency);

    
width = bmhd.Width;
height = bmhd.Height;
planes = bmhd.Bitplanes;
comp = bmhd.Compress;

depth = 8;    
    
pixel_size = depth / 8;
n_width=(width+15)&~15;
plane_size = n_width/8;
src_size = plane_size * planes;
src = (unsigned char *)  malloc(src_size);
dest = (unsigned char *) malloc(width * height);
unsigned char *p = dest;

for (unsigned y = 0; y < height; ++y)
{
    // read all planes in one hit,
    // 'coz PSP compresses across planes...

    if(comp)
    {
	for(unsigned x = 0; x < src_size;)
	{
    	    signed char t;

	    frs.readK(&t, 1);

    	    if (t >= 0)
    	    {
    		++t;

    		frs.readK(src + x, t);

    		x += t;
    	    }
    	    else if (t != -128)
    	    {
    		signed char b;

    		frs.readK(&b, 1);

    		t =- t +1;

    		memset(src + x, b, t);

    		x += t;
    	    }
	}
    }
    else
    {
	frs.readK(src, src_size);
    }

    	// lazy planar->chunky...

    for (unsigned x = 0; x < width; ++x)
    {
	for (unsigned n = 0; n < planes; ++n)
	{
		char bit = src[n * plane_size + (x / 8)] >> ((x^7) & 7);

		p[x * pixel_size + (n / 8)] |= (bit & 1) << (n & 7);
	}
    }

    p += width;
}

    free(src);

    for(u32 i = 0;i < height;i++)
    {
	for(u32 j = 0;j < width;j++)
	    printf("%d,", dest[i * width + j]);

	printf("\n");
    }
	
    image.compression = ((bmhd.Compress) ? "RLE":"-");
    image.colorspace = "RGB";

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
    RGB rgb;
    RGBA rgba;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    line++;
/*
    if(!line || line % bmhd.Bitplanes == 0)
    {
	for(s32 i = 0;i < bmhd.Bitplanes;i++)
	{
	    if(!frs.readK(tile[i], bmhd.Width))
		return SQE_R_BADFILE;
	}
    }
*/
    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    delete [] pal;

    if(tile)
    {
	for(s32 i = 0;i < bmhd.Bitplanes;i++)
	{
	    delete [] tile[i];
	}

	delete [] tile;
        tile = NULL;
    }

    delete [] dline;
    dline = NULL;
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
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

s32 fmt_codec::write_scanline(RGBA *scan)
{
    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

/*
 *
 * skip unknown chunks
 * return true if everything is OK
 *
 */

bool skip_unknown(ifstreamK &f, u32 waiting)
{
    CHUNK_HEAD chunk;

    while(true)
    {
        if(!f.readK(&chunk, sizeof(CHUNK_HEAD))) return false;
	
	if(chunk.type != waiting)
	{
	    f.seekg(fmt_utils::konvertLong(chunk.size), ios::cur);

	    if(!f.good()) return false;
	}
	else
	    break;
    }

    f.seekg(-sizeof(CHUNK_HEAD), ios::cur);

    return true;
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string();
}

#include "fmt_codec_cd_func.h"
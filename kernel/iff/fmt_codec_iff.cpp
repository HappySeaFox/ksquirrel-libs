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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_iff_defs.h"
#include "fmt_codec_iff.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Interchange File Format");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.iff *.ilbm ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string(); // "FORM....ILBM" is too common to be a regexp :-)
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,0,0,4,4,4,13,236,244,37,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,76,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,138,147,146,138,146,18,152,225,226,162,2,21,113,81,82,65,21,129,50,96,186,224,230,64,76,86,18,133,90,10,182,21,236,140,0,0,90,148,23,119,76,2,101,204,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;
    pal = 0;

    return SQERR_OK;
}

unsigned char *src;
unsigned char *dest;
unsigned pixel_size;
unsigned n_width;
unsigned plane_size;
unsigned src_size;
unsigned width = 0, height = 0, planes = 0, depth = 0, comp = 0;


s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage)
        return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    CHUNK_HEAD chunk;
    u32 ilbm;
    
    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQERR_BADFILE;

    if(chunk.type != IFF_FORM) return SQERR_NOTSUPPORTED;

    if(!frs.readK(&ilbm, 4)) return SQERR_BADFILE;

    if(ilbm != IFF_ILBM) return SQERR_NOTSUPPORTED;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQERR_BADFILE;

    if(chunk.type != IFF_BMHD) return SQERR_NOTSUPPORTED;

    if(!frs.readK(&bmhd, sizeof(CHUNK_BMHD))) return SQERR_BADFILE;

    if(!skip_unknown(frs, IFF_CMAP)) return SQERR_BADFILE;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQERR_BADFILE;

    if(chunk.type != IFF_CMAP) return SQERR_NOTSUPPORTED;

    pal_entr = fmt_utils::konvertLong(chunk.size) / 3;

    printf("Masking: %d\nCompress: %d\n", bmhd.Masking, bmhd.Compress);

    printf("pal_entr: %d\n", pal_entr);

    pal = new RGB [pal_entr];

    if(!pal)
	return SQERR_NOMEMORY;

    if(!frs.readK(pal, pal_entr * sizeof(RGB))) return SQERR_BADFILE;
    
    for(s32 i = 0;i < pal_entr;i++)
	printf("PAL %d,%d,%d\n", pal[i].r, pal[i].g, pal[i].b);

    if(!skip_unknown(frs, IFF_BODY)) return SQERR_BADFILE;

    if(!frs.readK(&chunk, sizeof(CHUNK_HEAD))) return SQERR_BADFILE;

    if(chunk.type != IFF_BODY) return SQERR_BADFILE;

    bmhd.Width = fmt_utils::konvertWord(bmhd.Width);
    bmhd.Height = fmt_utils::konvertWord(bmhd.Height);
    bmhd.Left = fmt_utils::konvertWord(bmhd.Left);
    bmhd.Top = fmt_utils::konvertWord(bmhd.Top);
    bmhd.Transparency = fmt_utils::konvertWord(bmhd.Transparency);
    bmhd.PageWidth = fmt_utils::konvertWord(bmhd.PageWidth);
    bmhd.PageHeight = fmt_utils::konvertWord(bmhd.PageHeight);

    tile = new u8 * [bmhd.Bitplanes];

    if(!tile)
	return SQERR_NOMEMORY;
	
    for(s32 i = 0;i < bmhd.Bitplanes;i++)
    {
	tile[i] = new u8 [bmhd.Width];
	
	if(!tile[i])
	    return SQERR_NOMEMORY;
    }

    dline = new u8 [bmhd.Width];

    if(!dline)
	return SQERR_NOMEMORY;

    finfo.image[currentImage].w = bmhd.Width;
    finfo.image[currentImage].h = bmhd.Height;

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
	
    
    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "RGB" << "\n"
        << ((bmhd.Compress) ? "RLE":"-") << "\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();
    
    line = -1;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGB rgb;
    RGBA rgba;
    
    line++;
/*
    if(!line || line % bmhd.Bitplanes == 0)
    {
	for(s32 i = 0;i < bmhd.Bitplanes;i++)
	{
	    if(!frs.readK(tile[i], bmhd.Width))
		return SQERR_BADFILE;
	}
    }
*/
    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    jmp_buf             jmp;
    ifstreamK           m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();
        return SQERR_BADFILE;
    }

    w = 1;
    h = 1;
    bpp = 1;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "RGB" << "\n"
        << "RLE" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;


    }

    m_frs.close();

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    if(pal)
	delete [] pal;

    if(tile)
    {
	for(s32 i = 0;i < bmhd.Bitplanes;i++)
	{
	    if(tile[i])
		delete [] tile[i];
	}

	delete [] tile;
    }

    if(dline)
	delete [] dline;
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

s32 fmt_codec::fmt_writeimage(std::string, RGBA *, s32, s32, const fmt_writeoptions &)
{
    return SQERR_NOTSUPPORTED;
}

bool fmt_codec::fmt_writable() const
{
    return false;
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

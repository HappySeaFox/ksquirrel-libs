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
#include "fmt_codec_psd_defs.h"
#include "fmt_codec_psd.h"

#include "error.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.8.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Adobe Photoshop PSD");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.psd ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("\x0038\x0042\x0050\x0053\x0001");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,255,255,0,128,128,0,4,4,4,204,13,117,30,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,65,17,192,48,8,4,177,192,7,1,88,200,85,1,56,104,5,180,159,248,151,80,114,161,221,215,206,13,28,136,200,92,72,241,168,170,81,220,221,102,75,69,163,17,36,9,193,184,78,4,74,138,40,86,18,160,32,59,65,198,193,153,111,235,239,217,205,110,125,148,87,249,198,253,2,11,129,25,221,25,73,23,33,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;
	
    currentImage = -1;
    layer = -1;
    
    u32 ident;
    u16 ver;

    if(!frs.be_getlong(&ident))
	return SQERR_BADFILE;

    if(ident != 0x38425053)
	return SQERR_NOTSUPPORTED;

    if(!frs.be_getshort(&ver))
	return SQERR_BADFILE;

    if(ver != 1)
	return SQERR_BADFILE;

    last = 0;
    L = 0;

    s8 dummy[6];
    if(!frs.readK(dummy, 6)) return SQERR_BADFILE;

    if(!frs.be_getshort(&channels))
	return SQERR_BADFILE;

    if(channels != 3 && channels != 4 && channels != 1)
	return SQERR_NOTSUPPORTED;

    if(!frs.be_getlong(&height))
	return SQERR_BADFILE;

    if(!frs.be_getlong(&width))
	return SQERR_BADFILE;

    if(!frs.be_getshort(&depth))
	return SQERR_BADFILE;

    if(!frs.be_getshort(&mode))
	return SQERR_BADFILE;

    if(depth != 8)
	return SQERR_NOTSUPPORTED;

    if(mode != PSD_RGB && mode != PSD_CMYK && mode != PSD_INDEXED && mode != PSD_GRAYSCALE)
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_RGB && (channels != 3 && channels != 4))
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_CMYK && channels != 4 && channels != 5)
	return SQERR_NOTSUPPORTED;

    if(mode == PSD_INDEXED && channels != 1)
	return SQERR_NOTSUPPORTED;

    u32 data_count;

    if(!frs.be_getlong(&data_count))
	return SQERR_BADFILE;

    if(data_count)
    {
	if(!frs.readK(pal, 256 * sizeof(RGB))) return SQERR_BADFILE;
    }

    if(!frs.be_getlong(&data_count))
	return SQERR_BADFILE;

    if(data_count)
	frs.seekg(data_count, ios::cur);

    if(!frs.be_getlong(&data_count))
	return SQERR_BADFILE;

    if(data_count)
	frs.seekg(data_count, ios::cur);

    // find out if the data is compressed
    //   0: no compressiod
    //   1: RLE compressed

    if(!frs.be_getshort(&compression))
	return SQERR_BADFILE;

    if(compression != 1 && compression != 0)
	return SQERR_NOTSUPPORTED;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].hasalpha = (mode == PSD_RGB) ? true : ((channels == 5) ? true : false);
    finfo.image[currentImage].passes = (channels == 5) ? 4 : channels;
    finfo.image[currentImage].h = height;
    finfo.image[currentImage].w = width;
    
    if(compression)
    {
	u16 b[height * channels];

	if(!frs.readK(b, 2 * height * channels)) return SQERR_BADFILE;
    }

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    stringstream type;
    
    switch(mode)
    {
	case PSD_RGB:
	    type << "RGB";
	    finfo.image[currentImage].bpp = 24;
	break;

	case PSD_CMYK:
	    type << "CMYK";
	    finfo.image[currentImage].bpp = (channels == 5) ? 32 : 24;
	break;
	
	case PSD_INDEXED:
	    type << "Color indexed";
	    finfo.image[currentImage].bpp = 8;
	break;

	case PSD_GRAYSCALE:
	    type << "Grayscale";
	    finfo.image[currentImage].bpp = 8;
	break;
    }

    last = (RGBA**)calloc(height, sizeof(RGBA*));

    if(!last)
        return SQERR_NOMEMORY;

    const s32 S = width * sizeof(RGBA);

    for(u32 i = 0;i < height;i++)
    {
	last[i] = (RGBA*)0;
    }

    for(u32 i = 0;i < height;i++)
    {
	last[i] = (RGBA*)malloc(S);

	if(!last[i])
	    return SQERR_NOMEMORY;
	    
	memset(last[i], 255, S);
    }
    
    line = -1;

    L = (u8*)calloc(width, 1);
    
    if(!L)
	return SQERR_NOMEMORY;


    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
	<< finfo.image[currentImage].w << "x"
	<< finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
	<< type.str() << "\n"
	<< ((compression) ? "RLE" : "-") << "\n"
	<< bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    layer++;
    line = -1;

    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u8 c, value, *p;
    s32 count = 0;
    
    line++;

    memcpy(scan, last[line], sizeof(RGBA) * finfo.image[currentImage].w);

    if(compression)
    {
	while(count < finfo.image[currentImage].w)
	{
	    if(!frs.readK(&c, 1)) return SQERR_BADFILE;

	    if(c == 128)
	    {} // do nothing
	    else if(c > 128)
	    {
		c ^= 0xff;
		c += 2;

		if(!frs.readK(&value, 1)) return SQERR_BADFILE;

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
		    if(!frs.readK(&value, 1)) return SQERR_BADFILE;

		    p = (u8*)(scan+i);
		    *(p+layer) = value;
		}

		count += c;
	    }
	}
    }
    else
    {
	if(!frs.readK(L, width)) return SQERR_BADFILE;

	for(u32 i = 0;i < width;i++)
	{
	    p = (u8*)(scan+i);
	    *(p+layer) = L[i];
	}
    }

    memcpy(last[line], scan, sizeof(RGBA) * finfo.image[currentImage].w);

    if(layer == finfo.image[currentImage].passes-1)
    {
	if(mode == PSD_CMYK)
	{
	    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
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

	    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
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

	    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
	    {
		v = scan[i].r;

		(scan+i)->r = v;
		(scan+i)->g = v;
		(scan+i)->b = v;
		scan[i].a = 255;
	    }	    
	}
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 	bpp;
    s32 	m_layer, m_line, passes;
    u16 m_depth, m_mode, m_channels, m_compression;
    u32 w, h;
    RGB 	m_pal[256];
    s32 	m_bytes;
    jmp_buf	jmp;
    ifstreamK	m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;
			
    if(setjmp(jmp))
    {
	m_frs.close();
	return SQERR_BADFILE;
    }

    m_layer = -1;

    u32 ident;
    u16 ver;
    
    if(!m_frs.be_getlong(&ident))
	longjmp(jmp, 1);

    if(ident != 0x38425053)
	longjmp(jmp, 1);

    if(!m_frs.be_getshort(&ver))
	longjmp(jmp, 1);

    if(ver != 1)
	longjmp(jmp, 1);

    s8 dummy[6];
    if(!m_frs.readK(dummy, 6))
	longjmp(jmp, 1);

    if(!m_frs.be_getshort(&m_channels))
	longjmp(jmp, 1);

    if(m_channels != 3 && m_channels != 4 && m_channels != 1)
	longjmp(jmp, 1);

    if(!m_frs.be_getlong(&h))
	longjmp(jmp, 1);

    if(!m_frs.be_getlong(&w))
	longjmp(jmp, 1);

    if(!m_frs.be_getshort(&m_depth))
	longjmp(jmp, 1);

    if(!m_frs.be_getshort(&m_mode))
	longjmp(jmp, 1);

    passes = (m_channels == 5) ? 4 : m_channels;

    if(m_depth != 8)
	longjmp(jmp, 1);

    if(m_mode != PSD_RGB && m_mode != PSD_CMYK && m_mode != PSD_INDEXED && m_mode != PSD_GRAYSCALE)
	longjmp(jmp, 1);

    if(m_mode == PSD_RGB && (m_channels != 3 && m_channels != 4))
	longjmp(jmp, 1);

    if(m_mode == PSD_CMYK && m_channels != 4 && m_channels != 5)
	longjmp(jmp, 1);

    if(m_mode == PSD_INDEXED && m_channels != 1)
	longjmp(jmp, 1);

    u32 data_count;

    if(!m_frs.be_getlong(&data_count))
	longjmp(jmp, 1);

    if(data_count)
    {
	if(!m_frs.readK(m_pal, 256 * sizeof(RGB))) return SQERR_BADFILE;
    }

    if(!m_frs.be_getlong(&data_count))
	longjmp(jmp, 1);

    if(data_count)
	m_frs.seekg(data_count, ios::cur);

    if(!m_frs.be_getlong(&data_count))
	longjmp(jmp, 1);

    if(data_count)
	m_frs.seekg(data_count, ios::cur);

    if(!m_frs.be_getshort(&m_compression))
	longjmp(jmp, 1);

    if(m_compression != 1 && m_compression != 0)
	longjmp(jmp, 1);

    if(m_compression)
    {
	u16 b[h * m_channels];

	if(!m_frs.readK(b, 2 * h * m_channels)) longjmp(jmp, 1);
    }

    stringstream type;

    bpp = 24;
    switch(m_mode)
    {
	case PSD_RGB:
	    type << "RGB";
	    bpp = 24;
	break;

	case PSD_CMYK:
	    type << "CMYK";
	    bpp = (m_channels == 5) ? 32 : 24;
	break;
	
	case PSD_INDEXED:
	    type << "Color indexed";
	    bpp = 8;
	break;

	case PSD_GRAYSCALE:
	    type << "Grayscale";
	    bpp = 8;
	break;
    }

    RGBA m_last[h][w];

    for(u32 i = 0;i < h;i++)
    {
	memset(m_last[i], 255, w * sizeof(RGBA));
    }

    m_line = -1;

    u8 m_L[w];
    
    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << type.str() << "\n"
        << ((m_compression) ? "RLE" : "-") << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
	longjmp(jmp, 1);
    }
    
    memset(*image, 0, m_bytes);

    for(m_layer = 0;m_layer < passes;m_layer++)    
    {
	m_line = -1;

	for(u32 h2 = 0;h2 < h;h2++)
	{
	    RGBA 	*scan = *image + h2 * w;
	    
    u8 c, value, *p;
    u32 count = 0;
    
    m_line++;

    memcpy(scan, m_last[m_line], sizeof(RGBA) * w);

    if(m_compression)
    {
	while(count < w)
	{
	    if(!m_frs.readK(&c, 1)) longjmp(jmp, 1);

	    if(c == 128)
	    {} // do nothing
	    else if(c > 128)
	    {
		c ^= 0xff;
		c += 2;

		if(!m_frs.readK(&value, 1)) longjmp(jmp, 1);

		for(u32 i = count; i < count+c;i++)
		{
		    p = (u8*)(scan+i);
		    *(p+m_layer) = value;
		}

		count += c;
	    }
	    else if(c < 128)
	    {
		c++;

		for(u32 i = count; i < count+c;i++)
		{
		    if(!m_frs.readK(&value, 1)) longjmp(jmp, 1);

		    p = (u8*)(scan+i);
		    *(p+m_layer) = value;
		}

		count += c;
	    }
	}
    }
    else
    {
	if(!m_frs.readK(m_L, w)) longjmp(jmp, 1);

	for(u32 i = 0;i < w;i++)
	{
	    p = (u8*)(scan+i);
	    *(p+m_layer) = m_L[i];
	}
    }

    memcpy(m_last[m_line], scan, sizeof(RGBA) * w);

    if(m_layer == passes-1)
    {
	if(m_mode == PSD_CMYK)
	{
	    for(u32 i = 0;i < w;i++)
	    {
		scan[i].r = (scan[i].r * scan[i].a) >> 8;
		scan[i].g = (scan[i].g * scan[i].a) >> 8;
		scan[i].b = (scan[i].b * scan[i].a) >> 8;
	    
		if(m_channels == 4)
		    scan[i].a = 255;
	    }
	}
	else if(m_mode == PSD_INDEXED)
	{
	    u8 r;
	    const s32 z1 = 768/3;
	    const s32 z2 = z1 << 1;

	    for(u32 i = 0;i < w;i++)
	    {
		u8 *p = (u8*)m_pal;
		r = scan[i].r;

		(scan+i)->r = *(p+r);
		(scan+i)->g = *(p+r+z1);
		(scan+i)->b = *(p+r+z2);
		scan[i].a = 255;
	    }	    
	}
	else if(m_mode == PSD_GRAYSCALE)
	{
	    u8 v;

	    for(u32 i = 0;i < w;i++)
	    {
		v = scan[i].r;

		(scan+i)->r = v;
		(scan+i)->g = v;
		(scan+i)->b = v;
		scan[i].a = 255;
	    }	    
	}
    }
    }
    }

    m_frs.close();

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo; // maybe set to CompressionRLE ?
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

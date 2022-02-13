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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_pcx_defs.h"
#include "fmt_codec_pcx.h"

#include "error.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,128,128,128,4,4,4,171,39,77,152,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,78,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,139,147,138,138,19,144,161,162,162,2,100,130,69,160,12,21,4,3,170,6,166,11,110,14,196,100,37,81,168,165,96,91,193,206,8,0,0,88,48,23,89,192,219,238,61,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;
    pal_entr = 0;

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

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&pfh, sizeof(PCX_HEADER))) return SQERR_BADFILE;

    if(pfh.ID != 10 || pfh.Encoding != 1)
	return SQERR_BADFILE;

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
	if(!frs.readK(&test, 1)) return SQERR_BADFILE;

	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQERR_BADFILE;

	if(!frs.readK(pal, 768)) return SQERR_BADFILE;
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

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
    
    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
	<< finfo.image[currentImage].w << "x"
	<< finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
	<< ((pal_entr) ? "Color indexed":"RGB") << "\n"
	<< "-\n"
	<< bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
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
		return SQERR_BADFILE;

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
		    return SQERR_BADFILE;
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

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 	w, h, bpp;
    PCX_HEADER	m_pfh;
    short	m_TotalBytesLine;
    RGB		m_pal[256];
    s32		m_pal_entr;
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

    if(!m_frs.readK(&m_pfh, sizeof(PCX_HEADER))) longjmp(jmp, 1);

    if(m_pfh.ID != 10 || m_pfh.Encoding != 1)
	longjmp(jmp, 1);

    w = m_pfh.Xmax - m_pfh.Xmin + 1;
    h = m_pfh.Ymax - m_pfh.Ymin + 1;
    bpp = m_pfh.bpp * m_pfh.NPlanes;
    m_pal_entr = 0;

    if(m_pfh.bpp == 1)
    {
	m_pal_entr = 2;

	memset(m_pal, 0, sizeof(RGB));
	memset(m_pal+1, 255, sizeof(RGB));

    }
    else if(m_pfh.bpp <= 4)
    {
	m_pal_entr = 16;

	memcpy(m_pal, m_pfh.Palette, 48);
    }
    else if(m_pfh.bpp == 8 && m_pfh.NPlanes == 1)
    {
	m_pal_entr = 256;

	m_frs.seekg(-769, ios::end);

	s8 test;
	if(!m_frs.readK(&test, 1)) longjmp(jmp, 1);

	if(test != PCX_COLORMAP_SIGNATURE && test != PCX_COLORMAP_SIGNATURE_NEW)
	    return SQERR_BADFILE;

	if(!m_frs.readK(m_pal, 768)) longjmp(jmp, 1);
	
//	s32 i;
//	for(i=0;i<256;i++)
//	printf("%d %d %d\n",(finfo.image[currentImage].m_pal)[i].r,(finfo.image[currentImage].m_pal)[i].g,(finfo.image[currentImage].m_pal)[i].b);
    }

    m_frs.seekg(128, ios::beg);
/*    
    printf("ID: %d\nVersion: %d\nEncoding: %d\nbpp: %d\nNPlanes: %d\nBytesPerLine: %d\nPaletteInfo: %d\n",
    m_pfh.ID, m_pfh.Version, m_pfh.Encoding, m_pfh.bpp, m_pfh.NPlanes, m_pfh.BytesPerLine, m_pfh.PaletteInfo);
*/
    m_TotalBytesLine = m_pfh.NPlanes * m_pfh.BytesPerLine;

    m_bytes = w * h * sizeof(RGBA);
/*
    sprintf(dump, "%s\n%d\n%d\n%d\n%s\nRLE\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	1,
	m_bytes);
*/
    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << ((m_pal_entr)?"Color indexed":"RGB") << "\n"
        << "-" << "\n"
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
        RGBA 	*scan = *image + h2 * w;

	u16  i, j;
	u8 channel[4][w];
	u8 indexes[w];

	for(i = 0;i < 4;i++)
	    memset(channel[i], 255, w);

	switch(bpp)
	{
    	    case 1:
	    break;

	    case 4:
	    break;

	    case 8:
		if(!getrow(m_frs, indexes, m_pfh.BytesPerLine))
		    longjmp(jmp, 1);

		for(i = 0;i < w;i++)
		    memcpy(scan+i, m_pal+indexes[i], sizeof(RGB));
	    break;

	    case 16:
	    break;

	    case 24:
	    {
		for(j = 0;j < m_pfh.NPlanes;j++)
		{
		    if(!getrow(m_frs, channel[j], m_pfh.BytesPerLine))
			longjmp(jmp, 1);
		}
	    
		for(i = 0;i < w;i++)
		{
    		    scan[i].r = channel[0][i];
    		    scan[i].g = channel[1][i];
    		    scan[i].b = channel[2][i];
		}
	    }
	    break;

	    default:;
	}
    }

    m_frs.close();
    
    return SQERR_OK;
}

    
void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
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

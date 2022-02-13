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
#include "fmt_codec_bmp_defs.h"
#include "fmt_codec_bmp.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

/*
 *
 * This library works with the graphics-file formats used by the Microsoft Windows
 * operating system. 
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.1.2");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Windows Bitmap");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.bmp *.dib ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string(); // "BM" is too common to be a regexp :-)
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,0,255,4,4,4,55,45,89,24,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,99,96,96,8,5,1,6,32,48,20,20,20,20,5,49,2,149,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,146,139,138,147,147,10,144,161,162,162,164,228,228,132,42,2,100,192,213,128,24,48,93,112,115,32,38,43,129,44,11,20,132,218,10,118,70,0,0,73,168,23,77,189,109,216,200,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
	return SQERR_NOFILE;

    pal_entr = 0;    
    currentImage = -1;

    if(!frs.readK(&bfh, sizeof(BITMAPFILE_HEADER)))
	return SQERR_BADFILE;

    if(!frs.readK(&bih, sizeof(BITMAPINFO_HEADER)))
	return SQERR_BADFILE;

    if(bfh.Type != BMP_IDENTIFIER || bih.Size != 40)
    	return SQERR_BADFILE;

    if(bih.Compression != BI_RGB)
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

    RGBA		rgba;
    s32		i, j, scanShouldBe;

    if(bih.BitCount < 16)
    	pal_entr = 1 << bih.BitCount;
    else
	pal_entr = 0;

    finfo.image[currentImage].w = bih.Width;
    finfo.image[currentImage].h = bih.Height;
    finfo.image[currentImage].bpp = bih.BitCount;
    scanShouldBe = bih.Width;

    switch(finfo.image[currentImage].bpp)
    {
	case 1:
	{
	    s32 tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((finfo.image[currentImage].w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;
	
	default:
	    return SQERR_BADFILE;
    }

    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    filler = j;
	    break;
	}

    if(finfo.image[currentImage].bpp < 16)
    {
	/*  read palette  */
	for(i = 0;i < pal_entr;i++)
	{
	    if(!frs.readK(&rgba, sizeof(RGBA)))
		return SQERR_BADFILE;

	    (pal)[i].r = rgba.b;
	    (pal)[i].g = rgba.g;
	    (pal)[i].b = rgba.r;	
	}
    }
    
    /*  fseek to image bits  */
    frs.seekg(bfh.OffBits, ios_base::beg);

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.image[currentImage].needflip = true;
    finfo.images++;

    stringstream s;

    s	<< fmt_quickinfo() << "\n"
	<< finfo.image[currentImage].w << "x"
	<< finfo.image[currentImage].h << "\n"
	<< finfo.image[currentImage].bpp << "\n"
	<< ((pal_entr) ? "Color indexed":"RGB") << "\n"
	<< "-\n"
	<< bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u16 remain, scanShouldBe, j, counter = 0;
    u8 bt, dummy;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
    	case 1:
	{
		u8	index;
		remain=((finfo.image[currentImage].w)<=8)?(0):((finfo.image[currentImage].w)%8);
		scanShouldBe = finfo.image[currentImage].w;

		s32 tmp = scanShouldBe;
		scanShouldBe /= 8;
		scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
 
		// @todo get rid of miltiple 'if'
		for(j = 0;j < scanShouldBe;j++)
		{
			if(!frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (pal)+index, sizeof(RGB));
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	}
	break;

	case 4:
	{
		u8	index;
		remain = (finfo.image[currentImage].w)%2;

		s32 ck = (finfo.image[currentImage].w%2)?(finfo.image[currentImage].w + 1):(finfo.image[currentImage].w);
		ck /= 2;

		for(j = 0;j < ck-1;j++)
		{
			if(!frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		if(!frs.readK(&bt, 1))
		    return SQERR_BADFILE;

		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	}
	break;

	case 8:
	{
		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
			if(!frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			memcpy(scan+(counter++), (pal)+bt, 3);
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	}
	break;

	case 16:
	{
		u16 word;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
			if(!frs.readK(&word, 2))
			    return SQERR_BADFILE;

			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgb, sizeof(RGB)))
			return SQERR_BADFILE;

		    scan[counter].r = rgb.b;
		    scan[counter].g = rgb.g;
		    scan[counter].b = rgb.r;
		    counter++;
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgba, sizeof(RGBA)))
			return SQERR_BADFILE;

		    scan[j].r = rgba.b;
		    scan[j].g = rgba.g;
		    scan[j].b = rgba.r;
		}
	}
	break;

    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 		w, h, bpp;
    RGB			m_pal[256];
    s32			m_pal_entr;
    u16	m_FILLER;
    BITMAPFILE_HEADER	m_bfh;
    BITMAPINFO_HEADER	m_bih;
    s32 		m_bytes;
    jmp_buf		jmp;
    ifstreamK		m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
	return SQERR_NOFILE;

    if(setjmp(jmp))
    {	
	m_frs.close();
	return SQERR_BADFILE;
    }

    m_pal_entr = 0;    

    if(!m_frs.readK(&m_bfh, sizeof(BITMAPFILE_HEADER)))
	return SQERR_BADFILE;

    if(!m_frs.readK(&m_bih, sizeof(BITMAPINFO_HEADER)))
	return SQERR_BADFILE;

    if(m_bfh.Type != 0x4D42 || m_bih.Size != 40)
    	return SQERR_BADFILE;

    if(m_bih.Compression != BI_RGB)
	return SQERR_NOTSUPPORTED;

    RGBA		rgba;
    s32		i, j, scanShouldBe;

    if(m_bih.BitCount < 16)
    	m_pal_entr = 1 << m_bih.BitCount;
    else
	m_pal_entr = 0;

    w = m_bih.Width;
    h = m_bih.Height;
    bpp = m_bih.BitCount;
    scanShouldBe = m_bih.Width;

    switch(bpp)
    {
	case 1:
	{
	    s32 tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;
	
	default:
	    return SQERR_BADFILE;
    }

    m_FILLER = 0;
    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    m_FILLER = j;
	    break;
	}

    if(bpp < 16)
    {
	/*  read palette  */
	for(i = 0;i < m_pal_entr;i++)
	{
	    if(!m_frs.readK(&rgba, sizeof(RGBA)))
		return SQERR_BADFILE;

	    (m_pal)[i].r = rgba.b;
	    (m_pal)[i].g = rgba.g;
	    (m_pal)[i].b = rgba.r;	
	}
    }
    
    /*  fseek to image bits  */
    m_frs.seekg(m_bfh.OffBits, ios_base::beg);


    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s	<< fmt_quickinfo() << "\n"
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


    for(s32 s = 0;s < h;s++)
    {
	RGBA *scan = *image + w * s;
	
        u16 remain, scanShouldBe, j, counter = 0;
	u8 bt, dummy;

	switch(bpp)
	{
    	    case 1:
	    {
		u8	index;
		remain=((w)<=8)?(0):((w)%8);
		scanShouldBe = w;

		s32 tmp = scanShouldBe;
		scanShouldBe /= 8;
		scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
 
		// @todo get rid of miltiple 'if'
		for(j = 0;j < scanShouldBe;j++)
		{
			if(!m_frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (m_pal)+index, sizeof(RGB));
		}

		for(j = 0;j < m_FILLER;j++)
		{
		    if(!m_frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	    }
	    break;

	    case 4:
	    {
		u8	index;
		remain = (w)%2;

		s32 ck = (w%2)?(w + 1):(w);
		ck /= 2;

		for(j = 0;j < ck-1;j++)
		{
			if(!m_frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (m_pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		if(!m_frs.readK(&bt, 1))
		    return SQERR_BADFILE;

		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (m_pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (m_pal)+index, 3);
		}

		for(j = 0;j < m_FILLER;j++)
		{
		    if(!m_frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	    }
	    break;

	    case 8:
	    {
		for(j = 0;j < w;j++)
		{
			if(!m_frs.readK(&bt, 1))
			    return SQERR_BADFILE;

			memcpy(scan+(counter++), (m_pal)+bt, 3);
		}

		for(j = 0;j < m_FILLER;j++)
		{
		    if(!m_frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	    }
	    break;

	    case 16:
	    {
		u16 word;

		for(j = 0;j < w;j++)
		{
			if(!m_frs.readK(&word, 2))
			    return SQERR_BADFILE;

			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < m_FILLER;j++)
		{
		    if(!m_frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	    }
	    break;

	    case 24:
	    {
		RGB rgb;

		for(j = 0;j < w;j++)
		{
		    if(!m_frs.readK(&rgb, sizeof(RGB)))
			return SQERR_BADFILE;

		    scan[counter].r = rgb.b;
		    scan[counter].g = rgb.g;
		    scan[counter].b = rgb.r;
		    counter++;
		}

		for(j = 0;j < m_FILLER;j++)
		{
		    if(!m_frs.readK(&dummy, 1))
		        return SQERR_BADFILE;
		}
	    }
	    break;

	    case 32:
	    {
		RGBA rgba;

		for(j = 0;j < w;j++)
		{
		    if(!m_frs.readK(&rgba, sizeof(RGBA)))
			return SQERR_BADFILE;

		    scan[j].r = rgba.b;
		    scan[j].g = rgba.g;
		    scan[j].b = rgba.r;
		}
	    }
	    break;
	}
    }

    m_frs.close();

    fmt_utils::flip((s8*)*image, w * sizeof(RGBA), h);

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
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

/*                                                                         politely ignore all options */
s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    s32			m_FILLER;
    RGBA		*scan;
    BITMAPFILE_HEADER	m_bfh;
    BITMAPINFO_HEADER	m_bih;
    ofstreamK		fws;
    jmp_buf		jmp;

    if(!image || !w || !h || file.empty())
	return SQERR_NOTOK;

    fws.open(file.c_str(), ios::binary | ios::out);

    if(!fws.good())
	return SQERR_NOFILE;

    if(setjmp(jmp))
    {	
	fws.close();
	return SQERR_BADFILE;
    }

    fmt_utils::flip((s8*)image, w * sizeof(RGBA), h);

    m_bfh.Type = BMP_IDENTIFIER;
    m_bfh.Size = 0;
    m_bfh.Reserved1 = 0;
    m_bfh.OffBits = sizeof(BITMAPFILE_HEADER) + sizeof(BITMAPINFO_HEADER);

    m_bih.Size = 40;
    m_bih.Width = w;
    m_bih.Height = h;
    m_bih.Planes = 1;
    m_bih.BitCount = 24;
    m_bih.Compression = BI_RGB;
    m_bih.SizeImage = 0;
    m_bih.XPelsPerMeter = 0;
    m_bih.YPelsPerMeter = 0;
    m_bih.ClrUsed = 0;
    m_bih.ClrImportant = 0;

    if(!fws.writeK(&m_bfh, sizeof(BITMAPFILE_HEADER)))
	longjmp(jmp, 1);

    if(!fws.writeK(&m_bih, sizeof(BITMAPINFO_HEADER)))
	longjmp(jmp, 1);

    m_FILLER = (w < 4) ? (4-w) : w%4;

    s8 fillchar = '0';
    RGB rgb;
    
    for(s32 y = 0;y < h;y++)
    {
	scan = image + w * y;

	for(s32 x = 0;x < w;x++)
	{
	    rgb.r = scan[x].b;
	    rgb.g = scan[x].g;
	    rgb.b = scan[x].r;

	    if(!fws.writeK(&rgb, sizeof(RGB)))
		longjmp(jmp, 1);
	}
	
	if(m_FILLER)
	{
	    for(s32 s = 0;s < m_FILLER;s++)
		fws.writeK(&fillchar, 1);
	}
    }

    fws.close();

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

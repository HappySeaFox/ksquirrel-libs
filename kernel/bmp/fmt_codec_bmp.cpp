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

#include "fmt_codec_bmp_defs.h"
#include "fmt_codec_bmp.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,63,63,186,76,76,76,130,43,243,42,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,86,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,145,83,167,134,2,25,145,145,161,161,83,167,162,138,0,25,112,53,32,6,76,23,220,28,144,165,96,147,185,192,238,88,192,0,0,4,186,56,198,86,105,90,109,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
	return SQE_R_NOFILE;

    pal_entr = 0;    
    currentImage = -1;
    read_error = false;

    if(!frs.readK(&bfh, sizeof(BITMAPFILE_HEADER)))
	return SQE_R_BADFILE;

    if(!frs.readK(&bih, sizeof(BITMAPINFO_HEADER)))
	return SQE_R_BADFILE;

    if(bfh.Type != BMP_IDENTIFIER || bih.Size != 40)
    	return SQE_R_BADFILE;

    if(bih.Compression != BI_RGB)
	return SQE_R_NOTSUPPORTED;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    RGBA		rgba;
    s32		i, j, scanShouldBe;

    if(bih.BitCount < 16)
    	pal_entr = 1 << bih.BitCount;
    else
	pal_entr = 0;

    image.w = bih.Width;
    image.h = bih.Height;
    image.bpp = bih.BitCount;
    scanShouldBe = bih.Width;

    switch(image.bpp)
    {
	case 1:
	{
	    s32 tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((image.w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;
	
	default:
	    return SQE_R_BADFILE;
    }

    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    filler = j;
	    break;
	}

    if(image.bpp < 16)
    {
	/*  read palette  */
	for(i = 0;i < pal_entr;i++)
	{
	    if(!frs.readK(&rgba, sizeof(RGBA)))
		return SQE_R_BADFILE;

	    (pal)[i].r = rgba.b;
	    (pal)[i].g = rgba.g;
	    (pal)[i].b = rgba.r;	
	}
    }
    
    /*  fseek to image bits  */
    frs.seekg(bfh.OffBits, ios::beg);

    image.needflip = true;
    image.colorspace = (pal_entr ? "Color indexed":"RGB");
    image.compression = "-";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
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
			    return SQE_R_BADFILE;

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
		        return SQE_R_BADFILE;
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
			    return SQE_R_BADFILE;

			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (pal)+index, 3);
		}

		if(!frs.readK(&bt, 1))
		    return SQE_R_BADFILE;

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
		        return SQE_R_BADFILE;
		}
	}
	break;

	case 8:
	{
		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
			if(!frs.readK(&bt, 1))
			    return SQE_R_BADFILE;

			memcpy(scan+(counter++), (pal)+bt, 3);
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQE_R_BADFILE;
		}
	}
	break;

	case 16:
	{
		u16 word;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
			if(!frs.readK(&word, 2))
			    return SQE_R_BADFILE;

			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQE_R_BADFILE;
		}
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgb, sizeof(RGB)))
			return SQE_R_BADFILE;

		    scan[counter].r = rgb.b;
		    scan[counter].g = rgb.g;
		    scan[counter].b = rgb.r;
		    counter++;
		}

		for(j = 0;j < filler;j++)
		{
		    if(!frs.readK(&dummy, 1))
		        return SQE_R_BADFILE;
		}
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgba, sizeof(RGBA)))
			return SQE_R_BADFILE;

		    scan[j].r = rgba.b;
		    scan[j].g = rgba.g;
		    scan[j].b = rgba.r;
		}
	}
	break;

    }

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
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = true;
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

    m_FILLER = (image.w < 4) ? (4-image.w) : image.w%4;

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next()
{
    m_bfh.Type = BMP_IDENTIFIER;
    m_bfh.Size = 0;
    m_bfh.Reserved1 = 0;
    m_bfh.OffBits = sizeof(BITMAPFILE_HEADER) + sizeof(BITMAPINFO_HEADER);

    m_bih.Size = 40;
    m_bih.Width = writeimage.w;
    m_bih.Height = writeimage.h;
    m_bih.Planes = 1;
    m_bih.BitCount = 24;
    m_bih.Compression = BI_RGB;
    m_bih.SizeImage = 0;
    m_bih.XPelsPerMeter = 0;
    m_bih.YPelsPerMeter = 0;
    m_bih.ClrUsed = 0;
    m_bih.ClrImportant = 0;

    if(!fws.writeK(&m_bfh, sizeof(BITMAPFILE_HEADER)))
	return SQE_W_ERROR;

    if(!fws.writeK(&m_bih, sizeof(BITMAPINFO_HEADER)))
	return SQE_W_ERROR;

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    s8 fillchar = '0';
    RGB rgb;

    for(s32 x = 0;x < writeimage.w;x++)
    {
        rgb.r = scan[x].b;
        rgb.g = scan[x].g;
        rgb.b = scan[x].r;

	if(!fws.writeK(&rgb, sizeof(RGB)))
	    return SQE_W_ERROR;
    }

    if(m_FILLER)
    {
        for(s32 s = 0;s < m_FILLER;s++)
    	    fws.writeK(&fillchar, sizeof(s8));
    }

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("bmp");
}

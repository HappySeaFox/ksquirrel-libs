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
#include "fmt_codec_tga_defs.h"
#include "fmt_codec_tga.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

/*
 *
 * The TGA (Truevision Graphics Adapter) format
 * is used widely in paint, graphics, and imaging applications that
 * require the storage of image data containing up to 32 bits per
 * pixel. TGA is associated with the Truevision
 * product line of Targa, Vista, NuVista, and Targa 2000 graphics
 * adapters for the PC and Macintosh, all of which can capture
 * NTSC and/or PAL video image signals and store them in a digital frame buffer.
 * For this reason, TGA has also become popular in the world of
 * still-video editing.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.7.1");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("TarGA");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.tga ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,128,128,4,4,4,181,151,89,64,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,89,129,7,11,88,108,208,76,224,50,64,251,200,254,171,52,1,210,123,157,44,97,35,34,115,35,139,87,85,45,5,128,205,150,21,93,141,140,72,110,25,112,122,248,18,134,7,89,226,71,72,164,176,146,115,245,247,84,51,172,71,115,53,223,120,62,131,188,24,11,124,78,54,7,0,0,0,0,73,69,78,68,174,66,96,130,130");
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

    if(!frs.readK(&tfh, sizeof(TGA_FILEHEADER))) return SQERR_BADFILE;

    finfo.image[currentImage].w = tfh.ImageSpecW;
    finfo.image[currentImage].h = tfh.ImageSpecH;
    finfo.image[currentImage].bpp = tfh.ImageSpecDepth;
    pal_entr = 0;

    if(tfh.IDlength)
    {
	finfo.meta.push_back(fmt_metaentry());

	finfo.meta[0].group = "TGA image identification field";
	
	s8 data[tfh.IDlength];
	
	if(!frs.readK(data, tfh.IDlength)) return SQERR_BADFILE;

	finfo.meta[0].data = data;
    }

    if(tfh.ColorMapType)
    {
	pal_entr = tfh.ColorMapSpecLength;

//	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
//		return SQERR_NOMEMORY;

//	s8 sz = tfh.ColorMapSpecEntrySize;
	s32  i;
//	u16 word;
  
	for(i = 0;i < pal_entr;i++)
	{
		/*if(sz==24)*/ if(!frs.readK(pal+i, sizeof(RGB))) return SQERR_BADFILE;
/* alpha ingored  *//*else if(sz==32) { fread(finfo.pal+i, sizeof(RGB), 1, fptr); fgetc(fptr); }
		else if(sz==16)
		{
		    fread(&word, 2, 1, fptr);
		    (finfo.pal)[i].b = (word&0x1f) << 3;
		    (finfo.pal)[i].g = ((word&0x3e0) >> 5) << 3;
		    (finfo.pal)[i].r = ((word&0x7c00)>>10) << 3;
		}*/
		
	}
    }
//    else
//	pal = 0;

    if(tfh.ImageType == 0)
	return SQERR_BADFILE;

    stringstream comp, type;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.image[currentImage].needflip = true;
    finfo.images++;
    finfo.image[currentImage].hasalpha = (finfo.image[currentImage].bpp == 32);

    switch(tfh.ImageType)
    {
	case 1:
	    comp << "-";
	    type << "Color indexed";
	break;

	case 2:
	    comp << "-";
	    type << ((finfo.image[currentImage].bpp == 32) ? "RGBA":"RGB");
	break;

	case 3:
	    comp << "-";
	    type << "Monochrome";
	break;

	case 9:
	    comp << "RLE";
	    type << "Color indexed";
	break;

	case 10:
	    comp << "RLE";
	    type << ((finfo.image[currentImage].bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    comp << "RLE";
	    type << "Monochrome";
	break;
    }

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << type.str() << "\n"
        << comp.str() << "\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

//    printf("tfh.ImageType: %d, pal_len: %d\n", tfh.ImageType, tfh.ColorMapSpecLength);

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 j, counter = 0;
    RGB rgb;
    RGBA rgba;

    memset(scan, 255, finfo.image[currentImage].w * 4);

    switch(tfh.ImageType)
    {
    	case 0:
	break;

	case 1:
	{
	}
	break;

	case 2:
	{
	    if(tfh.ImageSpecDepth==24)
	    {
		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgb, sizeof(RGB))) return SQERR_BADFILE;

		    (scan+counter)->r = rgb.b;
		    (scan+counter)->g = rgb.g;
		    (scan+counter)->b = rgb.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

		    (scan+counter)->r = rgba.b;
		    (scan+counter)->g = rgba.g;
		    (scan+counter)->b = rgba.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==16)
	    {
		u16 word;

		for(j = 0;j < finfo.image[currentImage].w;j++)
		{
		    if(!frs.readK(&word, 2)) return SQERR_BADFILE;

		    scan[counter].b = (word&0x1f) << 3;
		    scan[counter].g = ((word&0x3e0) >> 5) << 3;
		    scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}
	    }
	}
	break;

	case 3:
	break;

	// RLE + color mapped
	case 9:
	break;

	// RLE + true color
	case 10:
	{
	    u8	bt, count;
	    ushort	counter = 0, word;
	    RGBA	rgba;
	    
	    for(;;)
	    {
		if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

		count = (bt&127) + 1;

    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(finfo.image[currentImage].bpp)
		    {
			case 16:
    			    if(!frs.readK(&word, 2)) return SQERR_BADFILE;

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;

			case 24:
    			    if(!frs.readK(&rgb, sizeof(RGB))) return SQERR_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;

			case 32:
    			    if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(finfo.image[currentImage].bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				if(!frs.readK(&word, 2)) return SQERR_BADFILE;

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				if(!frs.readK(&rgb, sizeof(RGB))) return SQERR_BADFILE;

				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

				(scan+counter)->r = rgba.b;
    				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;
				if(counter >= finfo.image[currentImage].w-1) goto lts;
			    }
			break;
		    }
		}
	    }
	}
	lts:
	break;

	// RLE + B&W
	case 11:
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 		w, h, bpp;
    TGA_FILEHEADER 	m_tfh;
    RGB 		m_pal[256];
    s32 		m_pal_entr;
    RGB 		rgb;
    RGBA 		rgba;
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

    if(!m_frs.readK(&m_tfh, sizeof(TGA_FILEHEADER))) longjmp(jmp, 1);

    w = m_tfh.ImageSpecW;
    h = m_tfh.ImageSpecH;
    bpp = m_tfh.ImageSpecDepth;
    m_pal_entr = 0;

    if(m_tfh.ColorMapType)
    {
	m_pal_entr = m_tfh.ColorMapSpecLength;

//	if((m_pal = (RGB*)calloc(m_pal_entr, sizeof(RGB))) == 0)
//		return SQERR_NOMEMORY;

//	s8 sz = m_tfh.ColorMapSpecEntrySize;
	s32  i;
//	u16 word;
  
	for(i = 0;i < m_pal_entr;i++)
	{
		/*if(sz==24)*/ if(!m_frs.readK(m_pal+i, sizeof(RGB))) longjmp(jmp, 1);
/* alpha ingored  *//*else if(sz==32) { fread(finfo.m_pal+i, sizeof(RGB), 1, m_fptr); fgetc(m_fptr); }
		else if(sz==16)
		{
		    fread(&word, 2, 1, m_fptr);
		    (finfo.m_pal)[i].b = (word&0x1f) << 3;
		    (finfo.m_pal)[i].g = ((word&0x3e0) >> 5) << 3;
		    (finfo.m_pal)[i].r = ((word&0x7c00)>>10) << 3;
		}*/
		
	}
    }
//    else
//	m_pal = 0;

    if(m_tfh.ImageType == 0)
	return SQERR_BADFILE;

    m_bytes = w * h * sizeof(RGBA);

    stringstream comp, type;

    switch(m_tfh.ImageType)
    {
	case 1:
	    comp << "-";
	    type << "Color indexed";
	break;

	case 2:
	    comp << "-";
	    type << ((bpp == 32) ? "RGBA":"RGB");
	break;

	case 3:
	    comp << "-";
	    type << "Monochrome";
	break;

	case 9:
	    comp << "RLE";
	    type << "Color indexed";
	break;

	case 10:
	    comp << "RLE";
	    type << ((bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    comp << "RLE";
	    type << "Monochrome";
	break;
    }

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << type.str() << "\n"
        << comp.str() << "\n"
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

    s32 j, counter = 0;

    switch(m_tfh.ImageType)
    {
    	case 0:
	break;

	case 1:
	break;

	case 2:
	{
	    if(m_tfh.ImageSpecDepth==24)
	    {
		for(j = 0;j < w;j++)
		{
		    if(!m_frs.readK(&rgb, sizeof(RGB))) longjmp(jmp, 1);

		    (scan+counter)->r = rgb.b;
		    (scan+counter)->g = rgb.g;
		    (scan+counter)->b = rgb.r;
		    counter++;
		}
	    }
	    else if(m_tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < w;j++)
		{
		    if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

		    (scan+counter)->r = rgba.b;
		    (scan+counter)->g = rgba.g;
		    (scan+counter)->b = rgba.r;
		    counter++;
		}
	    }
	    else if(m_tfh.ImageSpecDepth==16)
	    {
		u16 word;

		for(j = 0;j < w;j++)
		{
		    if(!m_frs.readK(&word, sizeof(u16))) longjmp(jmp, 1);

		    scan[counter].b = (word&0x1f) << 3;
		    scan[counter].g = ((word&0x3e0) >> 5) << 3;
		    scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}
	    }
	}
	break;

	case 3:
	break;

	// RLE + color mapped
	case 9:
	break;

	// RLE + true color
	case 10:
	{
	    u8	bt, count;
	    ushort	counter = 0, word;
	    
	    for(;;)
	    {
		if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);
		count = (bt & 127) + 1;
		
    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(bpp)
		    {
			case 16:
    			    if(!m_frs.readK(&word, 2)) longjmp(jmp, 1);

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= w-1) goto lts;
			    }
			break;

			case 24:
    			    if(!m_frs.readK(&rgb, sizeof(RGB))) longjmp(jmp, 1);

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgb.b;
				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;

			case 32:
    			    if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				if(!m_frs.readK(&word, 2)) longjmp(jmp, 1);

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				if(!m_frs.readK(&rgb, sizeof(RGB))) longjmp(jmp, 1);

				(scan+counter)->r = rgb.b;
				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= w-1) goto lts;
			    }
			break;
		    }
		}
	    }
	}
	lts:
	break;

	// RLE + B&W
	case 11:
	break;

    }
    }

    fmt_utils::flip((s8*)*image, w * sizeof(RGBA), h);

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
    opt->compression_scheme = CompressionRLE;
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

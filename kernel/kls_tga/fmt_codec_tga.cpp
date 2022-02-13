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

#include "fmt_codec_tga_defs.h"
#include "fmt_codec_tga.h"

#include "../xpm/codec_tga.xpm"

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

void fmt_codec::options(codec_options *o)
{
    o->version = "0.7.2";
    o->name = "TarGA";
    o->filter = "*.tga ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_tga;
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

    if(!frs.readK(&tfh, sizeof(TGA_FILEHEADER))) return SQE_R_BADFILE;

    image.w = tfh.ImageSpecW;
    image.h = tfh.ImageSpecH;
    image.bpp = tfh.ImageSpecDepth;
    pal_entr = 0;

    if(tfh.IDlength)
    {
	s8 data[tfh.IDlength];

	if(!frs.readK(data, tfh.IDlength)) return SQE_R_BADFILE;

	fmt_metaentry mt;

	mt.group = "TGA image identification field";
	
	mt.data = data;

	addmeta(mt);
    }

    if(tfh.ColorMapType)
    {
	pal_entr = tfh.ColorMapSpecLength;

//	if((pal = (RGB*)calloc(pal_entr, sizeof(RGB))) == 0)
//		return SQE_R_NOMEMORY;

//	s8 sz = tfh.ColorMapSpecEntrySize;
	s32  i;
//	u16 word;
  
	for(i = 0;i < pal_entr;i++)
	{
		/*if(sz==24)*/ if(!frs.readK(pal+i, sizeof(RGB))) return SQE_R_BADFILE;
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
	return SQE_R_BADFILE;

    std::string comp, type;

    fliph = (bool)(tfh.ImageSpecDescriptor & 0x10);
    image.needflip = !(bool)(tfh.ImageSpecDescriptor & 0x20);
    image.hasalpha = (image.bpp == 32);

    switch(tfh.ImageType)
    {
	case 1:
	    comp = "-";
	    type = "Color indexed";
	break;

	case 2:
	    comp = "-";
	    type = (( image.bpp == 32) ? "RGBA":"RGB");
	break;

	case 3:
	    comp = "-";
	    type = "Monochrome";
	break;

	case 9:
	    comp = "RLE";
	    type = "Color indexed";
	break;

	case 10:
	    comp = "RLE";
	    type = (( image.bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    comp = "RLE";
	    type = "Monochrome";
	break;
    }

    image.compression = comp;
    image.colorspace = type;

//    printf("tfh.ImageType: %d, pal_len: %d\n", tfh.ImageType, tfh.ColorMapSpecLength);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    s32 j, counter = 0;
    RGB rgb;
    RGBA rgba;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

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
		for(j = 0;j < im->w;j++)
		{
		    if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

		    (scan+counter)->r = rgb.b;
		    (scan+counter)->g = rgb.g;
		    (scan+counter)->b = rgb.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==32)
	    {
		for(j = 0;j < im->w;j++)
		{
		    if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		    (scan+counter)->r = rgba.b;
		    (scan+counter)->g = rgba.g;
		    (scan+counter)->b = rgba.r;
		    counter++;
		}
	    }
	    else if(tfh.ImageSpecDepth==16)
	    {
		u16 word;

		for(j = 0;j < im->w;j++)
		{
		    if(!frs.readK(&word, 2)) return SQE_R_BADFILE;

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
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		count = (bt&127) + 1;

    	        // RLE packet
    		if(bt >= 128)
		{
		    switch(im->bpp)
		    {
			case 16:
    			    if(!frs.readK(&word, 2)) return SQE_R_BADFILE;

			    rgb.b = (word&0x1f) << 3;
			    rgb.g = ((word&0x3e0) >> 5) << 3;
			    rgb.r = ((word&0x7c00)>>10) << 3;

			    for(j = 0;j < count;j++)
			    {
				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= im->w-1) goto lts;
			    }
			break;

			case 24:
    			    if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= im->w-1) goto lts;
			    }
			break;

			case 32:
    			    if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

			    for(j = 0;j < count;j++)
			    {
				(scan+counter)->r = rgba.b;
				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;

				if(counter >= im->w-1) goto lts;
			    }
			break;
		    }
		}
		else // Raw packet
		{
		    switch(im->bpp)
		    {
			case 16:

			    for(j = 0;j < count;j++)
			    {
    				if(!frs.readK(&word, 2)) return SQE_R_BADFILE;

				rgb.b = (word&0x1f) << 3;
				rgb.g = ((word&0x3e0) >> 5) << 3;
				rgb.r = ((word&0x7c00)>>10) << 3;

				memcpy(scan+(counter++), &rgb, sizeof(RGB));
				if(counter >= im->w-1) goto lts;
			    }
			break;

			case 24:
			    for(j = 0;j < count;j++)
			    {
				if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

				(scan+counter)->r = rgb.b;
    				(scan+counter)->g = rgb.g;
				(scan+counter)->b = rgb.r;
				counter++;

				if(counter >= im->w-1) goto lts;
			    }
			break;

			case 32:
			    for(j = 0;j < count;j++)
			    {
				if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

				(scan+counter)->r = rgba.b;
    				(scan+counter)->g = rgba.g;
				(scan+counter)->b = rgba.r;
				counter++;
				if(counter >= im->w-1) goto lts;
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
    
    if(fliph)
    {
	RGBA t;
	s32 ww = im->w;

	for(j = 0;j < ww / 2;j++)
	{
	    t = *(scan+j);
	    *(scan+j) = *(scan+ww-j-1);
	    *(scan+ww-j-1) = t;
	}
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionRLE;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = true;
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

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string();
}

#include "fmt_codec_cd_func.h"

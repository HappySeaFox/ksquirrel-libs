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

#include "fmt_codec_tga_defs.h"
#include "fmt_codec_tga.h"

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
    return std::string("0.7.2");
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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,0,128,128,76,76,76,122,6,193,82,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,89,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,83,67,167,134,2,25,83,103,70,206,156,58,21,194,8,133,49,166,66,165,166,66,68,96,186,224,230,128,44,5,155,204,5,118,199,2,6,0,118,194,58,182,15,252,196,205,0,0,0,0,73,69,78,68,174,66,96,130");
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

    if(!frs.readK(&tfh, sizeof(TGA_FILEHEADER))) return SQE_R_BADFILE;

    finfo.image[currentImage].w = tfh.ImageSpecW;
    finfo.image[currentImage].h = tfh.ImageSpecH;
    finfo.image[currentImage].bpp = tfh.ImageSpecDepth;
    pal_entr = 0;

    if(tfh.IDlength)
    {
	finfo.meta.push_back(fmt_metaentry());

	finfo.meta[0].group = "TGA image identification field";
	
	s8 data[tfh.IDlength];
	
	if(!frs.readK(data, tfh.IDlength)) return SQE_R_BADFILE;

	finfo.meta[0].data = data;
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
    finfo.image[currentImage].needflip = !(bool)(tfh.ImageSpecDescriptor & 0x20);
    finfo.images++;
    finfo.image[currentImage].hasalpha = (finfo.image[currentImage].bpp == 32);

    switch(tfh.ImageType)
    {
	case 1:
	    comp = "-";
	    type = "Color indexed";
	break;

	case 2:
	    comp = "-";
	    type = ((finfo.image[currentImage].bpp == 32) ? "RGBA":"RGB");
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
	    type = ((finfo.image[currentImage].bpp == 32) ? "RGBA":"RGB");
	break;

	case 11:
	    comp = "RLE";
	    type = "Monochrome";
	break;
    }

    finfo.image[currentImage].compression = comp;
    finfo.image[currentImage].colorspace = type;

//    printf("tfh.ImageType: %d, pal_len: %d\n", tfh.ImageType, tfh.ColorMapSpecLength);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
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
		    if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

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

		for(j = 0;j < finfo.image[currentImage].w;j++)
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
		    switch(finfo.image[currentImage].bpp)
		    {
			case 16:
    			    if(!frs.readK(&word, 2)) return SQE_R_BADFILE;

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
    			    if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

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
    			    if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

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
    				if(!frs.readK(&word, 2)) return SQE_R_BADFILE;

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
				if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

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
				if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

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
    
    if(fliph)
    {
	RGBA t;
	s32 ww = finfo.image[currentImage].w;

	for(j = 0;j < ww / 2;j++)
	{
	    t = *(scan+j);
	    *(scan+j) = *(scan+ww-j-1);
	    *(scan+ww-j-1) = t;
	}
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
    opt->compression_scheme = CompressionRLE;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = true;
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

bool fmt_codec::fmt_readable() const
{
    return true;
}
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

#include "fmt_codec_ras_defs.h"
#include "fmt_codec_ras.h"

/*
 *
 * The Sun Raster image file format is the native bitmap format of the Sun
 * Microsystems UNIX platforms using the SunOS operating system. This format is
 * capable of storing black-and-white, gray-scale, and color bitmapped data of any
 * pixel depth. The use of color maps and a simple Run-Length data compression
 * are also supported. Typically, most images found on a SunOS system are Sun
 * Raster images, and this format is supported by most UNIX imaging applications.
 *
 */

bool fmt_readdata(ifstreamK &, u8 *, u32, bool);

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.6.3");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("SUN Raster");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.ras ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("\x0059\x00A6\x006A\x0095");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,78,78,78,174,174,174,202,202,202,70,70,70,254,254,254,178,178,178,102,254,206,242,242,242,222,222,222,2,2,2,116,218,129,101,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,102,73,68,65,84,120,218,99,16,4,129,4,6,6,6,97,99,99,99,195,73,32,70,104,104,168,73,39,80,200,25,200,48,20,20,128,136,24,27,3,25,46,80,192,32,94,14,6,37,12,82,171,202,171,150,151,175,90,194,32,85,190,188,106,121,21,68,4,72,175,42,1,137,172,90,85,94,14,150,90,94,85,5,20,129,235,130,155,3,178,212,216,216,129,129,129,5,196,117,20,96,0,0,147,141,40,240,190,118,236,206,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    rle = false;
    isRGB = false;

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

    if(!frs.be_getlong(&rfh.ras_magic)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_width)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_height)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_depth)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_length)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_type)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_maptype)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&rfh.ras_maplength)) return SQE_R_BADFILE;

    if(rfh.ras_magic != RAS_MAGIC) return SQE_R_BADFILE;

    if(rfh.ras_type != RAS_OLD && rfh.ras_type != RAS_STANDARD && rfh.ras_type != RAS_BYTE_ENCODED && rfh.ras_type != RAS_RGB &&
	    rfh.ras_type != RAS_TIFF && rfh.ras_type != RAS_IFF &&  rfh.ras_type != RAS_EXPERIMENTAL)
	return SQE_R_BADFILE;
    else if(rfh.ras_type == RAS_EXPERIMENTAL)
	return SQE_R_NOTSUPPORTED;

    finfo.image[currentImage].w = rfh.ras_width;
    finfo.image[currentImage].h = rfh.ras_height;
    finfo.image[currentImage].bpp = rfh.ras_depth;

    switch(rfh.ras_maptype)
    {
	case RMT_NONE :
	{
		if (rfh.ras_depth < 24)
		{
		    s32 numcolors = 1 << rfh.ras_depth, i;

		    for (i = 0; i < numcolors; i++)
		    {
			pal[i].r = (255 * i) / (numcolors - 1);
			pal[i].g = (255 * i) / (numcolors - 1);
			pal[i].b = (255 * i) / (numcolors - 1);
		    }
		}

	break;
	}

	case RMT_EQUAL_RGB:
	{
		s8 *g, *b;

		s32 numcolors = 1 << rfh.ras_depth;

		s8 r[3 * numcolors];

		g = r + numcolors;
		b = g + numcolors;

		if(!frs.readK(r, 3 * numcolors)) return SQE_R_BADFILE;

		for(s32 i = 0; i < numcolors; i++)
		{
			pal[i].r = r[i];
			pal[i].g = g[i];
			pal[i].b = b[i];
		}
	break;
	}

	case RMT_RAW:
	{
		s8 colormap[rfh.ras_maplength];

		if(!frs.readK(colormap, rfh.ras_maplength)) return SQE_R_BADFILE;
	break;
	}
    }

    switch(rfh.ras_type)
    {
	case RAS_OLD:
	case RAS_STANDARD:
	case RAS_TIFF:
	case RAS_IFF:
	break;

	case RAS_BYTE_ENCODED:
	    rle = true;
	break;

	case RAS_RGB:
	    isRGB = true;
	break;
    }
    
    if(rfh.ras_depth == 1)
	linelength = (short)((rfh.ras_width / 8) + (rfh.ras_width % 8 ? 1 : 0));
    else
	linelength = (short)rfh.ras_width;

    fill = (linelength % 2) ? 1 : 0;

    buf = new u8 [rfh.ras_width * sizeof(RGB)];

    if(!buf)
	return SQE_R_NOMEMORY;

    finfo.images++;
    finfo.image[currentImage].compression = ((isRGB) ? "-":"RLE");
    finfo.image[currentImage].colorspace = "RGB";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}
    
s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u32 i;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
    	case 1:
	break;

	case 8:
		if(!fmt_readdata(frs, buf, linelength, rle))
		    return SQE_R_BADFILE;

		for(i = 0;i < rfh.ras_width;i++)
		    memcpy(scan+i, &pal[i], sizeof(RGB));

		if(fill)
		{
		    if(!fmt_readdata(frs, &fillchar, fill, rle))
			return SQE_R_BADFILE;
		}
    	break;

	case 24:
	{
	    u8 *b = buf;
	    
	    if(!fmt_readdata(frs, buf, rfh.ras_width * 3, rle))
		return SQE_R_BADFILE;

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].r = *b;
    	    	    scan[i].g = *(b+1);
		    scan[i].b = *(b+2);
		    b += 3;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 2);
		    scan[i].g = *(b + 1);
		    scan[i].b = *b;
		    b += 3;
		}
		
	    if(fill)
	    {
	        if(!fmt_readdata(frs, &fillchar, fill, rle))
		    return SQE_R_BADFILE;
	    }
	}	
	break;

	case 32:
	{
	    u8 *b = buf;
	    
	    if(!fmt_readdata(frs, buf, rfh.ras_width * 4, rle))
		return SQE_R_BADFILE;

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].a = *b;
	    	    scan[i].r = *(b+1);
    	    	    scan[i].g = *(b+2);
		    scan[i].b = *(b+3);
		    b += 4;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 3);
		    scan[i].g = *(b + 2);
		    scan[i].b = *(b + 1);
		    scan[i].a = *b;
		    b += 4;
		}
		
	    if(fill)
	    {
	        if(!fmt_readdata(frs, &fillchar, fill, rle))
		    return SQE_R_BADFILE;
	    }

	}
	break;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    if(buf)
	delete [] buf;

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = false;
}

s32 fmt_codec::fmt_write_init(std::string file, const fmt_image &image, const fmt_writeoptions &opt)
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


bool fmt_readdata(ifstreamK &ff, u8 *_buf, u32 length, bool rle)
{
    u8 repchar, remaining = 0;

    if(rle)
    {
	while(length--)
	{
		if (remaining)
		{
    		    remaining--;
		    *(_buf++)= repchar;
		}
		else
		{
			if(!ff.readK(&repchar, 1)) return false;

			if(repchar == RESC)
			{
				if(!ff.readK(&remaining, 1)) return false;

				if (remaining == 0)
					*(_buf++) = RESC;
				else
				{
					if(!ff.readK(&repchar, 1)) return false;
					*(_buf++) = repchar;
				}
			}
			else
				*(_buf++) = repchar;
		}
	}
    }
    else
    {
	if(!ff.readK(_buf, length)) return false;
	
    }

    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

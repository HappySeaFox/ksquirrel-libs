/*  This file is part of SQuirrel (http://ksquirrel.sf.net) libraries

    Copyright (c) 2004 Dmitry Baryshev <ckult@yandex.ru>

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
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_sgi_defs.h"
#include "fmt_codec_sgi.h"

/*
 *
 * The SGI image file format is actually part of the SGI image library found on
 * all Silicon Graphics machines. SGI image files may store black-and-white (.BW
 * extension), color RGB (.RGB extension),
 * or color RGB with alpha channel data (.RGBA extension)
 * images. SGI image files may also have the generic extension .SGI as well.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.9.4");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("SGI Format");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.rgb *.rgba *.bw");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("\001\332.[\001\002]");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,255,0,255,76,76,76,28,120,106,198,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,95,73,68,65,84,120,218,61,142,177,17,128,48,8,69,169,173,156,129,50,181,171,80,80,187,128,99,184,0,119,254,46,103,101,50,165,64,162,239,40,222,113,240,129,122,176,18,209,205,204,101,11,17,145,178,31,83,122,29,194,236,242,156,3,106,72,46,106,38,16,21,23,133,1,22,29,53,47,23,152,33,69,116,206,124,91,127,78,28,205,228,37,255,168,244,2,95,50,58,10,146,58,39,129,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
	return SQE_R_NOFILE;

    currentImage = -1;
    starttab = 0;
    lengthtab = 0;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;	    

    if(!frs.be_getshort(&sfh.Magik)) return SQE_R_BADFILE;
    if(!frs.readK(&sfh.StorageFormat, 1)) return SQE_R_BADFILE;
    if(!frs.readK(&sfh.bpc, 1)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&sfh.Dimensions)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&sfh.x)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&sfh.y)) return SQE_R_BADFILE;
    if(!frs.be_getshort(&sfh.z)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&sfh.pixmin)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&sfh.pixmax)) return SQE_R_BADFILE;
    if(!frs.be_getlong(&sfh.dummy)) return SQE_R_BADFILE;

    if(!frs.readK(sfh.name, sizeof(sfh.name))) return SQE_R_BADFILE;

    if(!frs.be_getlong(&sfh.ColormapID)) return SQE_R_BADFILE;

    if(!frs.readK(&sfh.dummy2, sizeof(sfh.dummy2))) return SQE_R_BADFILE;

    image.w = sfh.x;
    image.h = sfh.y;
    image.bpp = sfh.bpc * sfh.z * 8;

    if(image.bpp == 32) image.hasalpha = true;

    if(sfh.Magik != 474 || (sfh.StorageFormat != 0 && sfh.StorageFormat != 1) || (sfh.Dimensions != 1 && sfh.Dimensions != 2 && sfh.Dimensions != 3) || (sfh.bpc != 1 && sfh.bpc != 2))
	return SQE_R_BADFILE;

    if(sfh.bpc == 2 || sfh.ColormapID > 0)
	return SQE_R_NOTSUPPORTED;

    if(sfh.StorageFormat == 1)
    {
	s32 sz = sfh.y * sfh.z, i;
        lengthtab = (u32*)calloc(sz, sizeof(ulong));
	starttab = (u32*)calloc(sz, sizeof(ulong));
    
        if(!lengthtab)
    	    return SQE_R_NOMEMORY;

        if(!starttab)
	{
	    free(lengthtab);
	    return SQE_R_NOMEMORY;
	}

	frs.seekg(512, ios::beg);

	for(i = 0;i < sz;i++)
	    if(!frs.be_getlong(&starttab[i]))
		return SQE_R_BADFILE;

	for(i = 0;i < sz;i++)
	    if(!frs.be_getlong(&lengthtab[i]))
		return SQE_R_BADFILE;
    }

    rle_row = 0;

    if(strlen(sfh.name))
    {
	fmt_metaentry mt;
	mt.group = "SGI Image Name";
	mt.data = sfh.name;
	finfo.meta.push_back(mt);
    }

    image.needflip = true;
    image.compression = (sfh.StorageFormat ? "RLE" : "-");
    image.colorspace = fmt_utils::colorSpaceByBpp(image.bpp);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    const s32 sz = sfh.x;
    s32 i = 0, j = 0;
    s32 len;
    fstream::pos_type pos;

    memset(scan, 255, finfo.image[currentImage].w * 4);

    // channel[0] == channel RED, channel[1] = channel GREEN...
    s8	channel[4][sz];
    s8	bt;

    memset(channel[3], 255, sz);

    switch(sfh.z)
    {
    	case 1:
	{
	    if(sfh.StorageFormat)
	    {
		    j = 0;

		    frs.seekg(starttab[rle_row], ios::beg);
		    len = lengthtab[rle_row];

		    for(;;)
		    {
			s8 count;
		    
    			if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!frs.readK(&channel[0][j], 1)) return SQE_R_BADFILE; 

				    j++;

				    if(!len--) goto ex1;
				}
			else
			{
			    if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

			    if(!len--) goto ex1;

			    while(count--)
				channel[0][j++] = bt;
			}
		    }
		    ex1:
		    len = len; // some stuff: get rid of compile warning

		rle_row++;
	    }
	    else
	    {
		if(!frs.readK(channel[0], sz)) return SQE_R_BADFILE;
	    }

	    memcpy(channel[1], channel[0], sz);
	    memcpy(channel[2], channel[0], sz);
	}
	break;


	case 3:
	case 4:
	{
	    if(sfh.StorageFormat)
	    {
		for(i = 0;i < sfh.z;i++)
		{
		    j = 0;

		    frs.seekg(starttab[rle_row + i*finfo.image[currentImage].h], ios::beg);
		    len = lengthtab[rle_row + i*finfo.image[currentImage].h];

		    for(;;)
		    {
			s8 count;
		    
    			if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!frs.readK(&channel[i][j], 1)) return SQE_R_BADFILE; 
				    j++;
				    if(!len--) goto ex;
				}
			else
			{
			    if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

			    if(!len--) goto ex;

			    while(count--)
				channel[i][j++] = bt;
			}
		    }
		    ex:
		    len = len; // some stuff: get rid of compile warning
		}
		rle_row++;
	    }
	    else
	    {
		if(!frs.readK(channel[0], sz)) return SQE_R_BADFILE;

		pos = frs.tellg();
		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[1], sz)) return SQE_R_BADFILE;

		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[2], sz)) return SQE_R_BADFILE;

		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[3], sz)) return SQE_R_BADFILE;

		frs.seekg(pos);
	    }

	}
	break;
    }

    for(i = 0;i < sz;i++)
    {
        scan[i].r = channel[0][i];
        scan[i].g = channel[1][i];
        scan[i].b = channel[2][i];
        scan[i].a = channel[3][i];
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();
    
    if(starttab)
	free(starttab);
	
    if(lengthtab)
	free(lengthtab);

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

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

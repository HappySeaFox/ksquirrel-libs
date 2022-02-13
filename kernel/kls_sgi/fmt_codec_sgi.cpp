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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_sgi_defs.h"
#include "fmt_codec_sgi.h"

#include "../xpm/codec_sgi.xpm"

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

void fmt_codec::options(codec_options *o)
{
    o->version = "0.9.4";
    o->name = "SGI Format";
    o->filter = "*.rgb *.rgba *.bw";
    o->config = "";
    o->mime = "\001\332.[\001\002]";
    o->pixmap = codec_sgi;
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
    starttab = NULL;
    lengthtab = NULL;
    channel[0] = channel[1] = channel[2] = channel[3] = NULL;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
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

    for(s32 i = 0;i < 4;i++)
    {
        channel[i] = new s8 [sfh.x];

        if(!channel[i])
            return SQE_R_NOMEMORY;
    }

    if(sfh.StorageFormat == 1)
    {
	s32 sz = sfh.y * sfh.z, i;
        lengthtab = new u32 [sz];
	starttab  = new u32 [sz];
    
        if(!lengthtab || !starttab)
	    return SQE_R_NOMEMORY;

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

	mt.group = "Image Name";
	mt.data = sfh.name;

	addmeta(mt);
    }

    image.needflip = true;
    image.compression = (sfh.StorageFormat ? "RLE" : "-");
    image.colorspace = fmt_utils::colorSpaceByBpp(image.bpp);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    const s32 sz = sfh.x;
    s32 i = 0, j = 0;
    s32 len;
    fstream::pos_type pos;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

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

		    frs.seekg(starttab[rle_row + i*im->h], ios::beg);
		    len = lengthtab[rle_row + i*im->h];

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
		frs.seekg(im->w * (im->h - 1), ios::cur);
		if(!frs.readK(channel[1], sz)) return SQE_R_BADFILE;

		frs.seekg(im->w * (im->h - 1), ios::cur);
		if(!frs.readK(channel[2], sz)) return SQE_R_BADFILE;

		frs.seekg(im->w * (im->h - 1), ios::cur);
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

void fmt_codec::read_close()
{
    frs.close();

    delete [] starttab;
    starttab = NULL;

    delete [] lengthtab;
    lengthtab = NULL;

    for(s32 i = 0;i < 4;i++)
    {
        delete [] channel[i];
        channel[i] = NULL;
    }

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
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

s32 fmt_codec::write_scanline(RGBA * /*scan*/)
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

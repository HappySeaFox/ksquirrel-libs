/*  This file is part of ksquirrel (http://ksquirrel.sf.net)

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
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_ico_defs.h"
#include "fmt_codec_ico.h"

/*
 *
 * An icon-resource file contains image data for icons used by Windows
 * applications. The file consists of an icon directory identifying the number
 * and types of icon images in the file, plus one or more icon images. The
 * default filename extension for an icon-resource file is .ICO.
 *
*/

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.4.2");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Windows icons");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.ico *.cur ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,202,202,202,70,70,70,162,254,162,254,254,254,178,178,178,174,174,174,222,222,222,78,78,78,242,242,242,2,2,110,128,61,19,185,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,93,73,68,65,84,120,218,99,232,0,129,0,6,6,6,69,65,65,65,161,50,16,195,197,197,69,104,122,0,132,33,209,161,0,97,8,10,54,48,48,52,41,65,0,131,214,42,48,88,196,208,181,120,149,241,170,197,86,32,198,226,85,86,86,139,23,49,104,1,25,171,64,12,184,8,66,13,76,23,220,156,70,65,16,0,90,193,1,226,106,52,48,0,0,32,131,45,4,210,173,140,82,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    pal_entr = 0;
    bAND = 0;

    if(!frs.readK(&ifh, sizeof(ICO_HEADER)))
	return SQE_R_BADFILE;

    if(ifh.idType != 1 && ifh.idType != 2)
	return SQE_R_BADFILE;
	
//    printf("ICO: count = %d\n", ifh.idCount);

    ide = (ICO_DIRENTRY*)calloc(ifh.idCount, sizeof(ICO_DIRENTRY));

    if(!ide)
	return SQE_R_NOMEMORY;

    if(!frs.readK(ide, sizeof(ICO_DIRENTRY) * ifh.idCount))
	return SQE_R_BADFILE;

//    for(s32 i = 0;i < ifh.idCount;i++)
//    printf("colorcount: %d, bitcount: %d, bytes: %d\n", ide[i].bColorCount, ide[i].wBitCount, ide[i].dwBytes);

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage == ifh.idCount)
	return SQE_NOTOK;

    fmt_image image;

    RGBA	rgba;
    s32		i;
    fstream::pos_type	pos;

//    printf("ressize: %d\n", ide.dwBytes);

    image.w = ide[currentImage].bWidth;
    image.h = ide[currentImage].bHeight;
//    finfo.images = ifh.idCount;
    
/*
    if(finfo.image[currentImage].w != 16 && finfo.image[currentImage].w != 32 && finfo.image[currentImage].w != 64)
	return SQE_R_BADFILE;

    if(finfo.image[currentImage].h != 16 && finfo.image[currentImage].h != 32 && finfo.image[currentImage].h != 64)
	return SQE_R_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQE_R_BADFILE;
*/

    frs.seekg(ide[currentImage].dwImageOffset, ios::beg);

    if(!frs.readK(&bih, sizeof(BITMAPINFO_HEADER)))
	return SQE_R_BADFILE;

    image.bpp = bih.BitCount;
//    printf("bitcount #2: %d\n", bih.BitCount);
//    printf("pal_entr: %d\n", pal_entr);

    if(image.bpp < 16)
    {
	pal_entr = 1 << image.bpp;

	for(i = 0;i < pal_entr;i++)
	{
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		pal[i].r = rgba.b;
		pal[i].g = rgba.g;
		pal[i].b = rgba.r;
	}
    }
    else
    {
	pal_entr = 0;
    }

    pos = frs.tellg();

//    printf("Calculating ...\n");
    s32 count = image.w * image.h;
//    printf("count: %d\n", count);
    s32 count2 = (image.bpp < 16) ? (count / (8 / image.bpp)) : (count * (image.bpp / 8));
//    printf("count2: %d\n", count2);
    s32 count3 = count / 8;
//    printf("count3: %d\n", count3);

    frs.seekg(/*ide[currentImage].dwBytes - sizeof(BITMAPINFO_HEADER) - */count2, ios::cur);

    bAND = (u8 *)realloc(bAND, count * sizeof(u8));

    if(!bAND)
        return SQE_R_NOMEMORY;

    u8 realAND[count3];

    if(!frs.readK(realAND, count3)) return SQE_R_BADFILE;

/*    
    for(i = 0;i < count3;i++)
	printf("REALAND %d\n", realAND[i]);
*/
    s32 r = 0;

    for(i = 0;i < count3;i++)
    {
	for(s32 z = 0,f = 128;z < 8;z++,f >>= 1)
	{
	    bAND[r] = (realAND[i] & f) ? 1 : 0;
	//    printf("%d,", bAND[r]);
	    r++;
	}
	//printf("\n");
    }//printf("r: %d\n", r);
/*
    for(s32 i = 0;i < image.h;i++)
    {
	for(s32 j = 0;j < image.w;j++)
	{
	    printf("%2d", bAND[i * finfo.image[currentImage].w + j]);
	}
	printf("\n");
    }
*/
    frs.seekg(pos);

/*
    for(i = 0;i < pal_entr;i++)
	printf("%d %d %d\n",(pal)[i].r,(pal)[i].g,(pal)[i].b);
*/

    image.needflip = true;
    image.hasalpha = true;

/*	
    snprintf(finfo.image[currentImage].dump, sizeof(finfo.image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo.image[currentImage].w,
	finfo.image[currentImage].h,
	finfo.image[currentImage].bpp,
	"RGB",
	bytes);
*/
    image.compression = "-";
    image.colorspace = ((pal_entr) ? "Color indexed":"RGB");

    finfo.image.push_back(image);

    pixel = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}
    
s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 i, j, count;
    u8 bt, ind;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
    	case 1:
	    j = finfo.image[currentImage].w / 8;
	    count = 0;

	    for(i = 0;i < j;i++)
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

//		printf("*** Read byte %d\n", bt);

		for(s32 z = 0, f = 128;z < 8;z++,f >>= 1)
		{
		    ind = (bt & f) ? 1 : 0;
//		    printf("ind: %d, %d\n", ind, (bt & f));

		    memcpy(scan+count, pal+ind, sizeof(RGB));

//		    (scan+count)->a = (bAND[pixel]) ? 0 : 255;

//		    printf("pixel: %d\n", pixel);
		    count++;
		    pixel++;
		}
	    }
	break;

	case 4:
	    j = 0;
	    do
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		ind = bt >> 4;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
		ind = bt&15;
		memcpy(scan+j, pal+ind, sizeof(RGB));
//		(scan+j)->a = (bAND[pixel]) ? 0 : 255;
		j++;
		pixel++;
	    }while(j < finfo.image[currentImage].w);

	break;

	case 8:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!frs.readK(&bt, 1)) return SQE_R_BADFILE;

		memcpy(scan+i, pal+bt, sizeof(RGB));
//		(scan+i)->a = (bAND[pixel]) ? 0 : 255;
		pixel++;
	    }
	break;
	
	case 24:
	{
	    RGB rgb;
	    
	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

		(scan+i)->r = rgb.b;
		(scan+i)->g = rgb.g;
		(scan+i)->b = rgb.r;
		pixel++;
	    }
	}
	break;
	
	case 32:
	{
	    RGBA rgba;
	    
	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		(scan+i)->r = rgba.b;
		(scan+i)->g = rgba.g;
		(scan+i)->b = rgba.r;
		(scan+i)->a = rgba.a;
		pixel++;
	    }
	}
	break;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    if(bAND)
	free(bAND);

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

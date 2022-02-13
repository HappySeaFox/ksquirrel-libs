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


#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_ico_defs.h"
#include "fmt_codec_ico.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,0,0,128,0,255,0,4,4,4,78,45,246,135,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,74,73,68,65,84,120,218,99,96,96,72,3,1,6,32,72,20,20,20,20,3,51,148,148,148,196,210,160,12,160,144,49,20,48,152,184,128,129,51,131,73,136,107,168,75,104,8,136,17,226,226,234,226,138,157,1,83,3,211,5,55,7,98,178,146,24,212,82,176,173,96,103,36,0,0,118,239,27,119,25,223,110,163,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;

    pal_entr = 0;
    bAND = 0;

    if(!frs.readK(&ifh, sizeof(ICO_HEADER)))
	return SQERR_BADFILE;

    if(ifh.idType != 1 && ifh.idType != 2)
	return SQERR_BADFILE;
	
//    printf("ICO: count = %d\n", ifh.idCount);

    ide = (ICO_DIRENTRY*)calloc(ifh.idCount, sizeof(ICO_DIRENTRY));

    if(!ide)
	return SQERR_NOMEMORY;

    if(!frs.readK(ide, sizeof(ICO_DIRENTRY) * ifh.idCount))
	return SQERR_BADFILE;

//    for(s32 i = 0;i < ifh.idCount;i++)
//    printf("colorcount: %d, bitcount: %d, bytes: %d\n", ide[i].bColorCount, ide[i].wBitCount, ide[i].dwBytes);

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage == ifh.idCount)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    RGBA	rgba;
    s32		i;
    fstream::pos_type	pos;

//    printf("ressize: %d\n", ide.dwBytes);

    finfo.image[currentImage].w = ide[currentImage].bWidth;
    finfo.image[currentImage].h = ide[currentImage].bHeight;
//    finfo.images = ifh.idCount;
    
/*
    if(finfo.image[currentImage].w != 16 && finfo.image[currentImage].w != 32 && finfo.image[currentImage].w != 64)
	return SQERR_BADFILE;

    if(finfo.image[currentImage].h != 16 && finfo.image[currentImage].h != 32 && finfo.image[currentImage].h != 64)
	return SQERR_BADFILE;

    if(ide.bColorCount != 2 && ide.bColorCount != 8 && ide.bColorCount != 16)
	return SQERR_BADFILE;
*/

    frs.seekg(ide[currentImage].dwImageOffset, ios::beg);

    if(!frs.readK(&bih, sizeof(BITMAPINFO_HEADER)))
	return SQERR_BADFILE;

    finfo.image[currentImage].bpp = bih.BitCount;
//    printf("bitcount #2: %d\n", bih.BitCount);
//    printf("pal_entr: %d\n", pal_entr);

    if(finfo.image[currentImage].bpp < 16)
    {
	pal_entr = 1 << finfo.image[currentImage].bpp;

	for(i = 0;i < pal_entr;i++)
	{
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

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
    s32 count = finfo.image[currentImage].w * finfo.image[currentImage].h;
//    printf("count: %d\n", count);
    s32 count2 = (finfo.image[currentImage].bpp < 16) ? (count / (8 / finfo.image[currentImage].bpp)) : (count * (finfo.image[currentImage].bpp / 8));
//    printf("count2: %d\n", count2);
    s32 count3 = count / 8;
//    printf("count3: %d\n", count3);

    frs.seekg(/*ide[currentImage].dwBytes - sizeof(BITMAPINFO_HEADER) - */count2, ios::cur);

    bAND = (u8 *)realloc(bAND, count * sizeof(u8));

    if(!bAND)
        return SQERR_NOMEMORY;

    u8 realAND[count3];

    if(!frs.readK(realAND, count3)) return SQERR_BADFILE;

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
    for(s32 i = 0;i < finfo.image[currentImage].h;i++)
    {
	for(s32 j = 0;j < finfo.image[currentImage].w;j++)
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

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.image[currentImage].needflip = true;
    finfo.image[currentImage].hasalpha = true;
    finfo.images++;
/*	
    snprintf(finfo.image[currentImage].dump, sizeof(finfo.image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo.image[currentImage].w,
	finfo.image[currentImage].h,
	finfo.image[currentImage].bpp,
	"RGB",
	bytes);
*/
    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << ((pal_entr) ? "Color indexed":"RGB") << "\n"
        << "-\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

    pixel = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
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
		if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

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
		if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

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
		if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

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
		if(!frs.readK(&rgb, sizeof(RGB))) return SQERR_BADFILE;

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
		if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

		(scan+i)->r = rgba.b;
		(scan+i)->g = rgba.g;
		(scan+i)->b = rgba.r;
		(scan+i)->a = rgba.a;
		pixel++;
	    }
	}
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    ICO_HEADER		m_ifh;
    ICO_DIRENTRY	m_ide;
    BITMAPINFO_HEADER	m_bih;
    s32			m_pixel = 0;
    RGB 		m_pal[256];
    s32 		m_pal_entr;
    fstream::pos_type	pos;
    RGBA		rgba;
    s32 		m_bytes, r, count, count2, count3;
    jmp_buf		jmp;
    s32 		w, h, bpp;

    ifstreamK           m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
            return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	m_frs.close();

	return SQERR_BADFILE;
    }

    if(!m_frs.readK(&m_ifh, sizeof(ICO_HEADER))) longjmp(jmp, 1);

    if(m_ifh.idType != 1 && m_ifh.idType != 2)
	longjmp(jmp, 1);

    if(!m_frs.readK(&m_ide, sizeof(ICO_DIRENTRY))) longjmp(jmp, 1);

    w = m_ide.bWidth;
    h = m_ide.bHeight;

    m_frs.seekg(m_ide.dwImageOffset, ios::beg);
    if(!m_frs.readK(&m_bih, sizeof(BITMAPINFO_HEADER))) longjmp(jmp, 1);

    bpp = m_bih.BitCount;

    if(bpp < 16)
    {
	m_pal_entr = 1 << bpp;

	for(s32 i = 0;i < m_pal_entr;i++)
	{
		if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

		m_pal[i].r = rgba.b;
		m_pal[i].g = rgba.g;
		m_pal[i].b = rgba.r;
	}
    }
    else
    {
	m_pal_entr = 0;
    }

    pos = m_frs.tellg();

    count = w * h;
//    s32 count2 = count / (8 / bpp);
    count2 = (bpp < 16) ? (count / (8 / bpp)) : (count * (bpp / 8));

    count3 = count / 8;

    m_frs.seekg(count2, ios::cur);

    u8 m_bAND[count];

    if(!m_bAND)
        longjmp(jmp, 1);

    u8 realAND[count3];

    if(!m_frs.readK(realAND, count3)) longjmp(jmp, 1);

    r = 0;
    for(s32 i = 0;i < count3;i++)
    {
	for(s32 z = 0,f = 128;z < 8;z++,f >>= 1)
	{
	    m_bAND[r] = (realAND[i] & f) ? 1 : 0;
	    r++;
	}
    }

    m_frs.seekg(pos);

    m_bytes = w * h * sizeof(RGBA);
/*
    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	m_ifh.idCount,
	m_bytes);
*/
    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << ((m_pal_entr)?"Color indexed":"RGB") << "\n"
        << "-" << "\n"
        << m_ifh.idCount << "\n"
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

        s32 i, j, count;
	u8 bt, ind;

	switch(bpp)
	{
    	    case 1:
		j = w / 8;
		count = 0;

		for(i = 0;i < j;i++)
		{
		    if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

//			printf("*** Read byte %d\n", bt);

		    for(s32 z = 0, f = 128;z < 8;z++,f >>= 1)
		    {
			ind = (bt & f) ? 1 : 0;
//		    	printf("ind: %d, %d\n", ind, (bt & f));

			memcpy(scan+count, m_pal+ind, sizeof(RGB));

//		    	(scan+count)->a = (m_bAND[m_pixel]) ? 0 : 255;
			count++;
			m_pixel++;
		    }
		}
	    break;

	    case 4:
		j = 0;
		do
		{
		    if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

		    ind = bt >> 4;
		    memcpy(scan+j, m_pal+ind, sizeof(RGB));
//			(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    j++;
		    m_pixel++;
		    ind = bt&15;
		    memcpy(scan+j, m_pal+ind, sizeof(RGB));
//			(scan+j)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    j++;
		    m_pixel++;
		}while(j < w);
	    break;

	    case 8:
		for(i = 0;i < w;i++)
		{
		    if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

		    memcpy(scan+i, m_pal+bt, sizeof(RGB));
//			(scan+i)->a = (m_bAND[m_pixel]) ? 0 : 255;
		    m_pixel++;
		}
	    break;

	    case 24:
	    {
		RGB rgb;

		for(i = 0;i < w;i++)
		{
		    if(!m_frs.readK(&rgb, sizeof(RGB))) longjmp(jmp, 1);

		    (scan+i)->r = rgb.b;
		    (scan+i)->g = rgb.g;
		    (scan+i)->b = rgb.r;
		    m_pixel++;
		}
	    }
	    break;

	    case 32:
	    {
		RGBA rgba;

		for(i = 0;i < w;i++)
		{
		    if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

		    (scan+i)->r = rgba.b;
		    (scan+i)->g = rgba.g;
		    (scan+i)->b = rgba.r;
		    (scan+i)->a = rgba.a;
		    m_pixel++;
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
}

s32 fmt_codec::fmt_writeimage(std::string , RGBA *, s32 , s32 , const fmt_writeoptions &)
//s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

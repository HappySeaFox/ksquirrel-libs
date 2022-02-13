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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_sgi_defs.h"
#include "fmt_codec_sgi.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,0,224,4,4,4,219,147,181,29,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,61,142,193,17,128,64,8,3,105,129,7,13,100,232,192,171,0,83,128,62,174,255,86,4,68,247,181,147,129,128,136,236,66,146,91,85,173,5,128,237,145,140,142,65,22,155,83,86,128,112,164,56,131,244,74,60,194,75,24,193,22,248,204,124,91,127,207,219,12,155,163,125,181,223,184,30,91,52,23,81,90,195,36,47,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
	return SQERR_NOFILE;

    currentImage = -1;
    starttab = 0;
    lengthtab = 0;

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

    if(!frs.be_getshort(&sfh.Magik)) return SQERR_BADFILE;
    if(!frs.readK(&sfh.StorageFormat, 1)) return SQERR_BADFILE;
    if(!frs.readK(&sfh.bpc, 1)) return SQERR_BADFILE;
    if(!frs.be_getshort(&sfh.Dimensions)) return SQERR_BADFILE;
    if(!frs.be_getshort(&sfh.x)) return SQERR_BADFILE;
    if(!frs.be_getshort(&sfh.y)) return SQERR_BADFILE;
    if(!frs.be_getshort(&sfh.z)) return SQERR_BADFILE;
    if(!frs.be_getlong(&sfh.pixmin)) return SQERR_BADFILE;
    if(!frs.be_getlong(&sfh.pixmax)) return SQERR_BADFILE;
    if(!frs.be_getlong(&sfh.dummy)) return SQERR_BADFILE;

    if(!frs.readK(sfh.name, sizeof(sfh.name))) return SQERR_BADFILE;

    if(!frs.be_getlong(&sfh.ColormapID)) return SQERR_BADFILE;

    if(!frs.readK(&sfh.dummy2, sizeof(sfh.dummy2))) return SQERR_BADFILE;

    finfo.image[currentImage].w = sfh.x;
    finfo.image[currentImage].h = sfh.y;
    finfo.image[currentImage].bpp = sfh.bpc * sfh.z * 8;

    if(finfo.image[currentImage].bpp == 32) finfo.image[currentImage].hasalpha = true;

    if(sfh.Magik != 474 || (sfh.StorageFormat != 0 && sfh.StorageFormat != 1) || (sfh.Dimensions != 1 && sfh.Dimensions != 2 && sfh.Dimensions != 3) || (sfh.bpc != 1 && sfh.bpc != 2))
	return SQERR_BADFILE;

    if(sfh.bpc == 2 || sfh.ColormapID > 0)
	return SQERR_NOTSUPPORTED;

    if(sfh.StorageFormat == 1)
    {
	s32 sz = sfh.y * sfh.z, i;
        lengthtab = (u32*)calloc(sz, sizeof(ulong));
	starttab = (u32*)calloc(sz, sizeof(ulong));
    
        if(!lengthtab)
    	    return SQERR_NOMEMORY;

        if(!starttab)
	{
	    free(lengthtab);
	    return SQERR_NOMEMORY;
	}

	frs.seekg(512, ios::beg);

	for(i = 0;i < sz;i++)
	    if(!frs.be_getlong(&starttab[i]))
		return SQERR_BADFILE;

	for(i = 0;i < sz;i++)
	    if(!frs.be_getlong(&lengthtab[i]))
		return SQERR_BADFILE;
    }

    rle_row = 0;

    if(strlen(sfh.name))
    {
	finfo.meta[0].group = "SGI Image Name";
	finfo.meta[0].data = sfh.name;
    }

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.image[currentImage].needflip = true;
    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << ((finfo.image[currentImage].bpp == 32) ? "RGBA":"RGB") << "\n"
        << "RLE\n"
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
		    
    			if(!frs.readK(&bt, 1)) return SQERR_BADFILE;
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!frs.readK(&channel[0][j], 1)) return SQERR_BADFILE; 

				    j++;

				    if(!len--) goto ex1;
				}
			else
			{
			    if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

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
		if(!frs.readK(channel[0], sz)) return SQERR_BADFILE;
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
		    
    			if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!frs.readK(&channel[i][j], 1)) return SQERR_BADFILE; 
				    j++;
				    if(!len--) goto ex;
				}
			else
			{
			    if(!frs.readK(&bt, 1)) return SQERR_BADFILE;

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
		if(!frs.readK(channel[0], sz)) return SQERR_BADFILE;

		pos = frs.tellg();
		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[1], sz)) return SQERR_BADFILE;

		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[2], sz)) return SQERR_BADFILE;

		frs.seekg(finfo.image[currentImage].w * (finfo.image[currentImage].h - 1), ios::cur);
		if(!frs.readK(channel[3], sz)) return SQERR_BADFILE;

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

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 	w, h, bpp;
    SGI_HEADER	m_sfh;
    s32 	m_rle_row;
    s32 	m_bytes;
    jmp_buf	jmp;
    ifstreamK	m_frs;
    u32 *m_lengthtab = 0, *m_starttab = 0;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();

	if(m_starttab) free(m_starttab);
	if(m_lengthtab) free(m_lengthtab);

        return SQERR_BADFILE;
    }

    if(!m_frs.be_getshort(&m_sfh.Magik)) longjmp(jmp, 1);
    if(!m_frs.readK(&m_sfh.StorageFormat, 1)) longjmp(jmp, 1);
    if(!m_frs.readK(&m_sfh.bpc, 1)) longjmp(jmp, 1);
    if(!m_frs.be_getshort(&m_sfh.Dimensions)) longjmp(jmp, 1);
    if(!m_frs.be_getshort(&m_sfh.x)) longjmp(jmp, 1);
    if(!m_frs.be_getshort(&m_sfh.y)) longjmp(jmp, 1);
    if(!m_frs.be_getshort(&m_sfh.z)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_sfh.pixmin)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_sfh.pixmax)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_sfh.dummy)) longjmp(jmp, 1);

    if(!m_frs.readK(m_sfh.name, sizeof(m_sfh.name))) longjmp(jmp, 1);

    if(!m_frs.be_getlong(&m_sfh.ColormapID)) longjmp(jmp, 1);

    if(!m_frs.readK(&m_sfh.dummy2, sizeof(m_sfh.dummy2))) longjmp(jmp, 1);

    w = m_sfh.x;
    h = m_sfh.y;
    bpp = m_sfh.bpc * m_sfh.z * 8;

    if(m_sfh.Magik != 474 || (m_sfh.StorageFormat != 0 && m_sfh.StorageFormat != 1) || (m_sfh.Dimensions != 1 && m_sfh.Dimensions != 2 && m_sfh.Dimensions != 3) || (m_sfh.bpc != 1 && m_sfh.bpc != 2))
	longjmp(jmp, 1);

    if(m_sfh.bpc == 2 || m_sfh.ColormapID > 0)
	longjmp(jmp, 1);


    if(m_sfh.StorageFormat == 1)
    {
	s32 sz = m_sfh.y * m_sfh.z, i;
        m_lengthtab = (u32*)calloc(sz, sizeof(ulong));
	m_starttab = (u32*)calloc(sz, sizeof(ulong));
    
        if(!m_lengthtab)
    	    longjmp(jmp, 1);

        if(!m_starttab)
	    longjmp(jmp, 1);

	m_frs.seekg(512, ios::beg);

	for(i = 0;i < sz;i++)
	    if(!m_frs.be_getlong(&m_starttab[i]))
		longjmp(jmp, 1);

	for(i = 0;i < sz;i++)
	    if(!m_frs.be_getlong(&m_lengthtab[i]))
		longjmp(jmp, 1);
    }

    m_rle_row = 0;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << ((bpp == 32) ? "RGBA":"RGB") << "\n"
        << "RLE" << "\n"
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
	
	const s32 sz = m_sfh.x;
	s32 i = 0, j = 0;
        s32 len;
	fstream::pos_type pos;

	// channel[0] == channel RED, channel[1] = channel GREEN...
	s8	channel[4][sz];
	s8	bt;

	memset(channel[3], 255, sz);

	switch(m_sfh.z)
	{
    	    case 1:
	    {
		if(m_sfh.StorageFormat)
		{
		    j = 0;

		    m_frs.seekg(m_starttab[m_rle_row], ios::beg);
		    len = m_lengthtab[m_rle_row];

		    for(;;)
		    {
			s8 count;
		    
    			if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!m_frs.readK(&channel[0][j], 1)) longjmp(jmp, 1); 

				    j++;

				    if(!len--) goto ex1;
				}
			else
			{
			    if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

			    if(!len--) goto ex1;

			    while(count--)
				channel[0][j++] = bt;
			}
		    }
		    ex1:
		    len = len; // some stuff: get rid of compile warning

		m_rle_row++;
		}
		else
		{
		    if(!m_frs.readK(channel[0], sz)) longjmp(jmp, 1);
		}

		memcpy(channel[1], channel[0], sz);
		memcpy(channel[2], channel[0], sz);
	    }
	    break;


	    case 3:
	    case 4:
	    {
		if(m_sfh.StorageFormat)
		{
		    for(i = 0;i < m_sfh.z;i++)
		    {
			j = 0;

			m_frs.seekg(m_starttab[m_rle_row + i*h], ios::beg);
			len = m_lengthtab[m_rle_row + i*h];

			for(;;)
			{
			    s8 count;
		    
    			    if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

			    count = bt&0x7f;

			    if(!count) break;
		    
			    if(bt & 0x80)
				while(count--)
				{
				    if(!m_frs.readK(&channel[i][j], 1)) longjmp(jmp, 1); 
				    j++;
				    if(!len--) goto ex2;
				}
			    else
			    {
				if(!m_frs.readK(&bt, 1)) longjmp(jmp, 1);

				if(!len--) goto ex2;

				while(count--)
				    channel[i][j++] = bt;
			    }
			}
			ex2:
			len = len; // some stuff: get rid of compile warning
		    }
		    m_rle_row++;
		}
	        else
		{
		    if(!m_frs.readK(channel[0], sz)) longjmp(jmp, 1);

		    pos = m_frs.tellg();
		    m_frs.seekg(w * (h - 1), ios::cur);
		    if(!m_frs.readK(channel[1], sz)) longjmp(jmp, 1);

		    m_frs.seekg(w * (h - 1), ios::cur);
		    if(!m_frs.readK(channel[2], sz)) longjmp(jmp, 1);

		    m_frs.seekg(w * (h - 1), ios::cur);
		    if(!m_frs.readK(channel[3], sz)) longjmp(jmp, 1);

		    m_frs.seekg(pos);
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
    }

    m_frs.close();

    free(m_starttab);
    free(m_lengthtab);

    fmt_utils::flip((s8*)*image, w * sizeof(RGBA), h);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

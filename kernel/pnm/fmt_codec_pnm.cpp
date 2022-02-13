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
#include "fmt_codec_pnm_defs.h"
#include "fmt_codec_pnm.h"

#include "error.h"

/*
 *
 * PBM, PGM,
 * PNM, and PPM are
 * intermediate formats used in the conversion of many little known
 * formats via pbmplus, the Portable Bitmap Utilities. These
 * formats are mainly available under UNIX and
 * on Intel-based PCs.
 *
 */

static RGB palmono[2] = {RGB(255,255,255), RGB(0,0,0)};

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
    return std::string("Portable aNy Map");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.ppm *.pgm *.pbm *.pnm ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("P[123456]");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,80,80,4,4,4,254,242,52,76,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,189,2,15,22,176,216,160,153,128,102,128,246,145,253,87,41,1,154,123,157,44,48,0,88,27,4,175,136,104,10,73,93,45,17,93,13,198,76,110,12,154,185,123,136,25,221,153,9,89,201,180,35,53,243,111,157,158,106,166,246,209,188,154,111,60,31,79,50,23,69,141,112,85,89,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    fptr = fopen(file.c_str(), "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;

    currentImage = -1;

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

    s8		str[256];
    s32		w, h;
    u32		maxcolor;

    if(!sq_fgets(str, 255, fptr)) return SQERR_BADFILE;

    pnm = str[1] - 48;

    if(pnm < 1 || pnm > 6)
	return SQERR_BADFILE;

    while(true)
    {
	if(!sq_fgets(str, 255, fptr)) return SQERR_BADFILE;

        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    finfo.image[currentImage].w = w;
    finfo.image[currentImage].h = h;

    switch(pnm)
    {
	case 1:
	case 4:
	    finfo.image[currentImage].bpp = 1;
	break;

	case 2:
	case 5:
	    finfo.image[currentImage].bpp = 8;
	break;

	case 3:
	case 6:
	    finfo.image[currentImage].bpp = 8;
	break;
    }

    if(pnm != 4 && pnm != 1)
    {
	fscanf(fptr, "%d", &maxcolor);

	if(sq_ferror(fptr)) return SQERR_BADFILE;

	if((pnm == 5 || pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(pnm == 2 || pnm == 3)
	{
	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	else
	{
	    u8 dummy;
	    if(!sq_fgetc(fptr, &dummy)) return SQERR_BADFILE;
	}

	if(maxcolor <= 9)
	    strcpy(format, "%1d");
	else if(maxcolor >= 9 && maxcolor <= 99)
	    strcpy(format, "%2d");
	else if(maxcolor > 99 && maxcolor <= 999)
	    strcpy(format, "%3d");
	else if(maxcolor > 999 && maxcolor <= 9999)
	    strcpy(format, "%4d");

	koeff = 255.0 / maxcolor;
    }
    else if(pnm == 1)
    {
	strcpy(format, "%1d");
	koeff = 1.0;
    }
    
//    printf("maxcolor: %d, format: %s, koeff: %.1f\n\n", maxcolor, format, koeff);

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
    
    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << ((pnm == 1 || pnm == 4) ? "Monochrome":"Color indexed") << "\n"
        << "-\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGB		rgb;
    s8	bt;
    s32		i;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(pnm)
    {
	case 1:
        {
	    s32 d;

	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		d = (s32)(d * koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }

	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	break;

	case 2:
	{
	    s32 d;

	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		d = (s32)(d * koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	}
	break;

	case 3:
    	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		fscanf(fptr, format, (s32*)&rgb.r);
		fscanf(fptr, format, (s32*)&rgb.g);
		fscanf(fptr, format, (s32*)&rgb.b);
		if(sq_ferror(fptr)) return SQERR_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }

	    if(!skip_flood(fptr))
		return SQERR_BADFILE;
	break;

	case 6:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
//		(scan+i)->r = rgb.
    	    }
	break;

	case 5:
	{
	    s32 pos;
	    pos = ftell(fptr);
//	    printf("POS1: %d, ", pos);

	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!sq_fread(&bt,1,1,fptr)) return SQERR_BADFILE;

		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	}
	break;
	
	case 4:
	{
	    s32 index;//, remain = finfo.image[currentImage].w % 8;

	    for(i = 0;;)
	    {
		if(!sq_fread(&bt,1,1,fptr)) return SQERR_BADFILE;

		index = (bt&128)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&64)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&32)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&16)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&8)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&4)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&2)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
		index = (bt&1);
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo.image[currentImage].w) break;
	    }
	}
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 	w, h, bpp;
    s32		m_pnm;
    FILE 	*m_fptr;
    s8		m_format[10];
    double	m_koeff;
    s32 	m_bytes;
    s8		str[256];
    u32 maxcolor;
    jmp_buf	jmp;

    m_fptr = fopen(file.c_str(), "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	fclose(m_fptr);
	return SQERR_BADFILE;
    }

    if(!sq_fgets(str, 255, m_fptr)) longjmp(jmp, 1);

    m_pnm = str[1] - 48;

    if(m_pnm < 1 || m_pnm > 6)
	longjmp(jmp, 1);

    while(true)
    {
	if(!sq_fgets(str, 255, m_fptr)) longjmp(jmp, 1);

        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    w = w;
    h = h;

    bpp = 0;
    switch(m_pnm)
    {
	case 1:
	case 4:
	    bpp = 1;
	break;

	case 2:
	case 5:
	    bpp = 8;
	break;

	case 3:
	case 6:
	    bpp = 8;
	break;
    }

    if(m_pnm != 4 && m_pnm != 1)
    {
	fscanf(m_fptr, "%d", &maxcolor);
	if(sq_ferror(m_fptr)) longjmp(jmp, 1);

	if((m_pnm == 5 || m_pnm == 6) && maxcolor > 255)
	    return SQERR_BADFILE;

	if(m_pnm == 2 || m_pnm == 3)
	{
	    if(!skip_flood(m_fptr))
		longjmp(jmp, 1);
	}
	else
	{
	    fgetc(m_fptr);
	    if(sq_ferror(m_fptr)) longjmp(jmp, 1);
	}
	
	if(maxcolor <= 9)
	    strcpy(m_format, "%1d");
	else if(maxcolor >= 9 && maxcolor <= 99)
	    strcpy(m_format, "%2d");
	else if(maxcolor > 99 && maxcolor <= 999)
	    strcpy(m_format, "%3d");
	else if(maxcolor > 999 && maxcolor <= 9999)
	    strcpy(m_format, "%4d");

	m_koeff = 255.0 / maxcolor;
    }
    else if(m_pnm == 1)
    {
	strcpy(m_format, "%1d");
	m_koeff = 1.0;
    }
    else m_koeff = 1.0;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << ((m_pnm == 1 || m_pnm == 4) ? "Monochrome":"Color indexed") << "\n"
        << "-" << "\n"
        << 1 << "\n"
	<< m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    for(s32 h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	RGB		rgb;
	s8	bt;
	s32		i;

	switch(m_pnm)
	{
	    case 1:
    	    {
		s32 d;

		for(i = 0;i < w;i++)
		{
		    fscanf(m_fptr, m_format, &d);

		    d = (s32)(d * m_koeff);

		    memcpy(scan+i, palmono+d, sizeof(RGB));
    		}
	    
		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    }
	    break;

	    case 2:
	    {
		s32 d;

		for(i = 0;i < w;i++)
		{
		    fscanf(m_fptr, m_format, &d);
		    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

		    d = (s32)(d * m_koeff);

		    memset(scan+i, d, sizeof(RGB));
		}
	    
		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    }
	    break;

	    case 3:
    		for(i = 0;i < w;i++)
		{
		    fscanf(m_fptr, m_format, (s32*)&rgb.r);
		    fscanf(m_fptr, m_format, (s32*)&rgb.g);
		    fscanf(m_fptr, m_format, (s32*)&rgb.b);
		    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

		    memcpy(scan+i, &rgb, 3);
		}

		if(!skip_flood(m_fptr))
		    longjmp(jmp, 1);
	    break;

	    case 6:
		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&rgb,sizeof(RGB),1,m_fptr)) longjmp(jmp, 1);

		    memcpy(scan+i, &rgb, sizeof(RGB));
    		}
	    break;

	    case 5:
	    {
		s32 pos;
		pos = ftell(m_fptr);
//		printf("POS1: %d, ", pos);

		for(i = 0;i < w;i++)
		{
		    if(!sq_fread(&bt,1,1,m_fptr)) longjmp(jmp, 1);

		    rgb.r = rgb.g = rgb.b = bt;
		    memcpy(scan+i, &rgb, 3);
		}
	    }
	    break;

	    case 4:
	    {
		s32 index;//, remain = finfo.image[currentImage].w % 8;

		for(i = 0;;)
		{
		    if(!sq_fread(&bt,1,1,m_fptr)) longjmp(jmp, 1);

		    index = (bt&128)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&64)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&32)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&16)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&8)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&4)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&2)?1:0;
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		    index = (bt&1);
		    memcpy(scan+i, palmono+index, 3);i++; if(i >= w) break;
		}
	    }
	    break;
	}
    }
    
    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    fclose(fptr);

    finfo.meta.clear();
    finfo.image.clear();
}

bool skip_flood(FILE *f)
{
    s32 pos;
    u8 b;

    while(true)
    {
	pos = ftell(f);
	if(!sq_fread(&b, 1, 1, f)) return false;

	if(!isspace(b))
	{
	    if(b == '#')
	    {
		while(true)
		{
		    if(!sq_fgetc(f, &b)) return false;

		    if(b == '\n')
			break;
		}
	    }

	    break;
	}
    }

    fsetpos(f, (fpos_t*)&pos);
    
    return true;
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
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

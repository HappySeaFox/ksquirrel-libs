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

#include "fmt_codec_pnm_defs.h"
#include "fmt_codec_pnm.h"

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
    return std::string("0.6.4");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Portable aNy Map");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.pnm *.pgm *.pbm *.ppm ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("P[123456]");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,255,80,80,76,76,76,49,99,172,94,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,89,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,161,51,35,167,78,157,25,193,176,116,102,100,232,212,169,161,17,96,145,80,136,8,92,10,202,128,235,130,155,3,178,20,108,50,23,216,29,11,24,0,51,86,57,126,169,74,133,164,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    fptr = fopen(file.c_str(), "rb");
	        
    if(!fptr)
	return SQE_R_NOFILE;

    currentImage = -1;

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

    s8		str[256];
    s32		w, h;
    u32		maxcolor;

    if(!sq_fgets(str, 255, fptr)) return SQE_R_BADFILE;

    pnm = str[1] - 48;

    if(pnm < 1 || pnm > 6)
	return SQE_R_BADFILE;

    while(true)
    {
	if(!sq_fgets(str, 255, fptr)) return SQE_R_BADFILE;

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

	if(sq_ferror(fptr)) return SQE_R_BADFILE;

	if((pnm == 5 || pnm == 6) && maxcolor > 255)
	    return SQE_R_BADFILE;

	if(pnm == 2 || pnm == 3)
	{
	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	else
	{
	    u8 dummy;
	    if(!sq_fgetc(fptr, &dummy)) return SQE_R_BADFILE;
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

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = ((pnm == 1 || pnm == 4) ? "Monochrome":"Color indexed");

    return SQE_OK;
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
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		d = (s32)(d * koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }

	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	break;

	case 2:
	{
	    s32 d;

	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		d = (s32)(d * koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	break;

	case 3:
    	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		fscanf(fptr, format, (s32*)&rgb.r);
		fscanf(fptr, format, (s32*)&rgb.g);
		fscanf(fptr, format, (s32*)&rgb.b);
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }

	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	break;

	case 6:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
	    {
		if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQE_R_BADFILE;

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
		if(!sq_fread(&bt,1,1,fptr)) return SQE_R_BADFILE;

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
		if(!sq_fread(&bt,1,1,fptr)) return SQE_R_BADFILE;

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

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

void fmt_codec::fmt_read_close()
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
    opt->passes = 1;
    opt->needflip = false;
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
    fws << "P6" << endl << writeimage.w << " " << writeimage.h << endl << 255 << endl;

    return fws.good() ? SQE_OK : SQE_W_ERROR;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    for(s32 i = 0;i < writeimage.w;i++)
    {
	if(!fws.writeK(scan+i, sizeof(RGB)))
	    return SQE_W_ERROR;
    }

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("pnm");
}

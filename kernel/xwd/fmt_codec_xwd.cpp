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
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_xwd_defs.h"
#include "fmt_codec_xwd.h"

using namespace fmt_utils;

/*
 *
 * The XWD (X Window Dump) format is used specifically to store screen
 * dumps
 *
 * Created by the X Window System. Under X11, screen dumps are created by the
 * xwd client. Using xwd, the window or background is selected to
 * dump and an XWD file is produced containing an image of the window. If you
 * issue the following command:
 * 
 *   % xwd -root > output.xwd
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.4.3");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X Window Dump");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.xwd ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,40,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,137,12,83,76,76,76,65,217,160,88,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,83,167,206,156,58,53,20,204,0,130,153,17,12,43,35,161,140,165,83,103,70,70,34,24,64,53,112,93,112,115,64,150,130,77,230,2,187,99,1,3,0,143,46,58,174,81,21,167,92,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    pal = 0;

    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    XWDFileHeader xfh;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    XWDColor	color;
    s8 	str[256];
    s32		i, ncolors;

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&xfh, sizeof(XWDFileHeader))) return SQE_R_BADFILE;

    xfh.file_version = fmt_utils::konvertLong(xfh.file_version);

    if(xfh.file_version != XWD_FILE_VERSION)
	return SQE_R_BADFILE;

    frs.get(str, 255, '\n');
    
    frs.clear();

    frs.seekg(fmt_utils::konvertLong(xfh.header_size), ios::beg);

    pal_entr = ncolors = fmt_utils::konvertLong(xfh.ncolors);

    pal = new RGB [ncolors];

    if(!pal)
	return SQE_R_NOMEMORY;

    for(i = 0;i < ncolors;i++)
    {
	if(!frs.readK(&color, sizeof(XWDColor))) return SQE_R_BADFILE;

	pal[i].r = (s8)fmt_utils::konvertWord(color.red);
	pal[i].g = (s8)fmt_utils::konvertWord(color.green);
	pal[i].b = (s8)fmt_utils::konvertWord(color.blue);
    }

    finfo.image[currentImage].w = fmt_utils::konvertLong(xfh.pixmap_width);
    finfo.image[currentImage].h = fmt_utils::konvertLong(xfh.pixmap_height);
    finfo.image[currentImage].bpp = fmt_utils::konvertLong(xfh.bits_per_pixel);//fmt_utils::konvertLong(xfh.pixmap_depth);

    finfo.images++;

    finfo.meta.push_back(fmt_metaentry());

    finfo.meta[0].group = "XWD Window Name";
    finfo.meta[0].data = str;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = "RGB";

    filler = fmt_utils::konvertLong(xfh.bytes_per_line) - finfo.image[currentImage].w * finfo.image[currentImage].bpp / 8;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 	i;
    RGBA	rgba;
    RGB		rgb;
    u8 d;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
        case 24:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
    	    {
    		if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }
	    
	    for(s32 s = 0;s < filler;s++)
		if(!frs.readK(&d, 1))
		    return SQE_R_BADFILE;
	break;

        case 32:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
    	    {
    		if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

		scan[i].r = rgba.b;
		scan[i].g = rgba.g;
		scan[i].b = rgba.r;
	    }
	    
	    for(s32 s = 0;s < filler;s++)
		if(!frs.readK(&d, 1))
		    return SQE_R_BADFILE;
	break;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    if(pal)
	delete [] pal;

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

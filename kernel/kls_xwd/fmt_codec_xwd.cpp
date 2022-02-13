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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_xwd_defs.h"
#include "fmt_codec_xwd.h"

#include "../xpm/codec_xwd.xpm"

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
 * $ xwd -root > output.xwd
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.4.3";
    o->name = "X Window Dump";
    o->filter = "*.xwd ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-xwd";
    o->pixmap = codec_xwd;
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
    pal = NULL;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    XWDFileHeader xfh;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    XWDColor	color;
    s8 	str[256];
    s32		i, ncolors;

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

    image.w = fmt_utils::konvertLong(xfh.pixmap_width);
    image.h = fmt_utils::konvertLong(xfh.pixmap_height);
    image.bpp = fmt_utils::konvertLong(xfh.bits_per_pixel);//fmt_utils::konvertLong(xfh.pixmap_depth);

    if(image.bpp != 24 && image.bpp != 32)
        return SQE_R_NOTSUPPORTED;

    fmt_metaentry mt;

    mt.group = "Window Name";
    mt.data = str;

    addmeta(mt);

    image.compression = "-";
    image.colorspace = "RGB";

    filler = fmt_utils::konvertLong(xfh.bytes_per_line) - image.w * image.bpp / 8;

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    s32 	i;
    RGBA	rgba;
    RGB		rgb;
    u8          d;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(im->bpp)
    {
        case 24:
	    for(i = 0;i < im->w;i++)
    	    {
    		if(!frs.readK(&rgb, sizeof(RGB))) return SQE_R_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }
	    
	    for(s32 s = 0;s < filler;s++)
		if(!frs.readK(&d, 1))
		    return SQE_R_BADFILE;
	break;

        case 32:
	    for(i = 0;i < im->w;i++)
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

void fmt_codec::read_close()
{
    frs.close();

    delete [] pal;
    pal = NULL;

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
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

s32 fmt_codec::write_scanline(RGBA *scan)
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

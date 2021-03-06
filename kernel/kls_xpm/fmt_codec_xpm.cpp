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
#include <map>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_xpm.h"
#include "fmt_codec_xpm_defs.h"

#include "xpm_utils.h"

#include "../xpm/codec_xpm.xpm"

/*
 * 
 * The XPM (X PixMap) format is the current de facto standard for storing X Window
 * pixmap data to a disk file. This format is supported by many image editors,
 * graphics window managers, and image file converters. 
 *
 *
 * XPM is capable of storing black-and-white, gray-scale, or color image data.
 * Hotspot information for cursor bitmaps may also be stored. Although small
 * collections of data, such as icons, are typically associated with XPM files,
 * there is no limit to the size of an image or the number of colors that may be
 * stored in an XPM file.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    fillmap();
}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.6.4";
    o->name = "X11 Pixmap";
    o->filter = "*.xpm ";
    o->config = "";
    o->mime = "/\\* XPM \\*/\n";
    o->mimetype = "image/x-xpm";
    o->pixmap = codec_xpm;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &fl)
{
    frs.open(fl.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    file.clear();

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    s32		i;
    s8	str[256];

    s32 ret;

    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQE_R_BADFILE; }
    if(!frs.getS(str, 256)) return SQE_R_BADFILE;
    if(strncmp(str, "static", 6) != 0) return SQE_R_BADFILE;
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQE_R_BADFILE; }
    if(!frs.getS(str, 256)) return SQE_R_BADFILE;
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQE_R_BADFILE; }

    sscanf(str, "\"%d %d %d %d", &image.w, &image.h, &numcolors, (int*)&cpp);

    if(!numcolors)
	return SQE_R_BADFILE;

    s8 name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < numcolors;i++)
    {
	if(!frs.getS(str, 256)) return SQE_R_BADFILE;

	if(*str != '\"')
	{
	    numcolors = i;
	    break;
	}

	strcpy(name, "");

	found = str;
	found++;

	strncpy(name, found, cpp);
	name[cpp] = 0;

	sscanf(found+cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

	file[name] = hex2rgb(color);
    }

    if(!numcolors)
	return SQE_R_BADFILE;

    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQE_R_BADFILE; }

    image.bpp = 24;
    image.hasalpha = true;
    image.passes = 1;
    image.compression = "-";
    image.colorspace = "Indexed RGBA";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    const s32	bpl = im->w * (cpp+2);
    s32		i, j;
    s8 	line[bpl], key[KEY_LENGTH];
    
    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));

    switch(im->bpp)
    {
	case 24:
	{
	    RGBA  rgba;
	    bool f;

	    i = j = 0;
	    if(!frs.getS(line, sizeof(line))) return SQE_R_BADFILE;

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < im->w;j++)
	    {
		strncpy(key, line+i, cpp);
		i += cpp;

		std::map<std::string, RGBA>::const_iterator it = file.find(key);

		f = (it != file.end());

		if(!f)
		{
		    cerr << "XPM decoder: WARNING: color \"" << key << "\" not found, assuming transparent instead" << endl;
		    memset(&rgba, 0, sizeof(RGBA));
		}
		else
		    rgba = (*it).second;

		memcpy(scan+j, &rgba, sizeof(RGBA));
	    }
	}
	break;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    file.clear();
}

void fmt_codec::fillmap()
{
    s8 	name[80];
    s32 r, g, b, a;

    std::ifstream rgb_fstream;

    rgb_fstream.open(SQ_RGBMAP, ios::in);

    if(!rgb_fstream.good())
    {
	std::cerr << "libkls_xpm.so: rgbmap not found" << std::endl;
	return;
    }

    typedef std::pair<std::string, RGBA> xpm_pair;

    while(rgb_fstream.good())
    {
	rgb_fstream >> name >> r >> g >> b >> a;

	named.insert(xpm_pair(name, RGBA(r,g,b,a)));
    }

    rgb_fstream.close();
}

#include "fmt_codec_cd_func.h"

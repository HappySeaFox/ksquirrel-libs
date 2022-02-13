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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_xpm.h"
#include "fmt_codec_xpm_defs.h"

#include "xpm_utils.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.6.4");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X11 Pixmap");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.xpm ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("/\\* XPM \\*/\n");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,2,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,95,95,95,76,76,76,221,167,222,130,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,165,83,167,134,206,140,156,25,1,98,204,140,12,157,26,193,176,50,18,36,18,9,17,129,75,129,25,112,93,112,115,64,150,130,77,230,2,187,99,1,3,0,58,98,57,134,0,178,12,219,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &fl)
{
    frs.open(fl.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    file.clear();

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
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
//    printf("%d %d %d %d\n\n",finfo.image[currentImage].w,finfo.image[currentImage].h,numcolors,cpp);

    if(!numcolors)
	return SQE_R_BADFILE;

    s8 name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < numcolors;i++)
    {
	if(!frs.getS(str, 256)) return SQE_R_BADFILE;

	if(*str != '\"')
	{
	    cerr << "libSQ_read_xpm: file corrupted" << endl;
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

//	if(!i)printf("%s\n",color);

//	memcpy(Xmap[i].name, name, cpp);
//	Xmap[i].name[cpp] = 0;
//	Xmap[i].rgba = hex2rgb(color);
//	RGBA r = hex2rgb(color);
//	printf("%s <=> %d,%d,%d,%d\n", name, r.r, r.g, r.b, r.a);
	file[name] = hex2rgb(color);
    }

    if(!numcolors)
	return SQE_R_BADFILE;

//    cout << "111111111111";
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQE_R_BADFILE; }
//    cout << "333333333333333\n";

/*
    QuickSort(Xmap, 0, numcolors-1);

    for(i = 0;i < numcolors;i++)
    {
	printf("\"%s\"  %d %d %d %d\n",Xmap[i].name,Xmap[i].rgba.r,Xmap[i].rgba.g,Xmap[i].rgba.b,Xmap[i].rgba.a);
    }
*/

    image.bpp = 24;
    image.hasalpha = true;
    image.passes = 1;
    image.compression = "-";
    image.colorspace = "Indexed RGBA";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    const s32	bpl = im->w * (cpp+2);
    s32		i, j;
    s8 	line[bpl], key[KEY_LENGTH];
    
//    printf("bpl: %d\n", bpl);

    memset(scan, 255, im->w * sizeof(RGBA));
    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));
/*    
    static s32 ee = 0;
    printf("line %d\n", ee);
    ee++;
*/
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

		//trgba = BinSearch(Xmap, 0, numcolors-1, key);
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

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    file.clear();
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

void fmt_codec::fillmap()
{
    s8 	name[80];
    s32 	r, g, b, a;

    ifstream rgb_fstream;

    rgb_fstream.open(SQ_RGBMAP, ios::in);

    if(!rgb_fstream.good())
    {
	cerr << "libSQ_read_xpm: rgbmap not found" << endl;
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

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

#include "fmt_codec_cd_func.h"

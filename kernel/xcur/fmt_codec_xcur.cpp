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

#include "fmt_codec_xcur_defs.h"
#include "fmt_codec_xcur.h"

/*
 *
 * Library to support X cursors.
 * 
 * You can test it with nice cursors from http://kde-apps.org
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.4.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X Cursors");
}

std::string fmt_codec::fmt_filter()
{
    return std::string();
}

std::string fmt_codec::fmt_mime()
{
    return std::string("Xcur");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,78,78,78,174,174,174,202,202,202,70,70,70,254,254,254,254,254,2,178,178,178,242,242,242,222,222,222,2,2,2,88,22,29,181,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,92,73,68,65,84,120,218,99,16,4,129,2,6,6,6,97,99,99,99,195,73,32,70,104,104,168,73,39,80,200,25,200,48,20,20,128,136,24,27,3,25,46,80,192,32,150,6,6,41,12,82,89,232,12,177,101,105,171,178,178,178,150,0,69,178,210,128,12,176,212,42,160,88,10,66,151,8,204,28,144,165,198,198,14,12,12,140,32,174,163,0,3,0,118,140,35,242,255,128,119,200,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    currentToc = -1;

    if(!frs.readK(&xcur_h, sizeof(XCUR_HEADER))) return SQE_R_BADFILE;

    tocs = new XCUR_CHUNK_DESC [xcur_h.ntoc];

    if(!frs.readK(tocs, sizeof(XCUR_CHUNK_DESC) * xcur_h.ntoc)) return SQE_R_BADFILE;

    lastToc = false;

    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(lastToc)
    {
	finfo.animated = (currentToc > 0);
	return SQE_NOTOK;
    }

    do
    {
	 currentToc++;
    }
    while(tocs[currentToc].type != XCUR_CHUNK_TYPE_IMAGE && currentToc < (s32)xcur_h.ntoc);

    if(currentToc == (s32)xcur_h.ntoc-1)
	lastToc = true;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;    

    frs.seekg(tocs[currentToc].pos, ios::beg);

    if(!frs.readK(&xcur_chunk, sizeof(XCUR_CHUNK_HEADER))) return SQE_R_BADFILE;
    if(!frs.readK(&xcur_im, sizeof(XCUR_CHUNK_IMAGE))) return SQE_R_BADFILE;
    
    finfo.image[currentImage].w = xcur_im.width;
    finfo.image[currentImage].h = xcur_im.height;
    finfo.image[currentImage].bpp = 32;
    finfo.image[currentImage].delay = xcur_im.delay;
    finfo.image[currentImage].hasalpha = true;

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = "ARGB";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGBA rgba;

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
    {
	if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

	(scan+i)->r = rgba.b;
	(scan+i)->g = rgba.g;
	(scan+i)->b = rgba.r;
	(scan+i)->a = rgba.a;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    if(tocs)
	delete [] tocs;
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
    opt->needflip = false;
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

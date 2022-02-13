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
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_xcur_defs.h"
#include "fmt_codec_xcur.h"

#include "../xpm/codec_xcur.xpm"

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

void fmt_codec::options(codec_options *o)
{
    o->version = "0.4.0";
    o->name = "X Cursors";
    o->filter = "";
    o->config = "";
    o->mime = "Xcur";
    o->mimetype = "image/x-xcursor";
    o->pixmap = codec_xcur;
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
    currentToc = -1;

    if(!frs.readK(&xcur_h, sizeof(XCUR_HEADER))) return SQE_R_BADFILE;

    tocs = new XCUR_CHUNK_DESC [xcur_h.ntoc];
    
    if(!tocs)
        return SQE_R_NOMEMORY;

    if(!frs.readK(tocs, sizeof(XCUR_CHUNK_DESC) * xcur_h.ntoc)) return SQE_R_BADFILE;

    lastToc = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
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

    fmt_image image;

    frs.seekg(tocs[currentToc].pos, ios::beg);

    if(!frs.readK(&xcur_chunk, sizeof(XCUR_CHUNK_HEADER))) return SQE_R_BADFILE;
    if(!frs.readK(&xcur_im, sizeof(XCUR_CHUNK_IMAGE))) return SQE_R_BADFILE;

    image.w = xcur_im.width;
    image.h = xcur_im.height;
    image.bpp = 32;
    image.delay = xcur_im.delay;
    image.hasalpha = true;
    image.compression = "-";
    image.colorspace = "ARGB";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    RGBA rgba;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(s32 i = 0;i < im->w;i++)
    {
	if(!frs.readK(&rgba, sizeof(RGBA))) return SQE_R_BADFILE;

	(scan+i)->r = rgba.b;
	(scan+i)->g = rgba.g;
	(scan+i)->b = rgba.r;
	(scan+i)->a = rgba.a;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    delete [] tocs;
    tocs = NULL;

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

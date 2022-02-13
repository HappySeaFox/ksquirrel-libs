/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include "fmt_types.h"
#include "fileio.h"

#include "fmt_codec_svg_defs.h"
#include "fmt_codec_svg.h"

#include "svg2mem.h"
#include "error.h"

/*
 *
 * SVG is a language for describing two-dimensional graphics in XML [XML10].
 * SVG allows for three types of graphic objects: vector graphic shapes (e.g., paths consisting
 * of straight lines and curves), images and text. Graphical objects can be grouped, styled,
 * transformed and composited into previously rendered objects. The feature set includes
 * nested transformations, clipping paths, alpha masks, filter effects and template objects.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.1.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Scalable Vector Graphics");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.svg ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,40,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,216,104,0,76,76,76,176,176,176,200,200,200,194,127,201,12,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,93,73,68,65,84,120,218,99,232,0,129,9,12,12,12,93,171,86,173,90,33,4,98,24,27,27,175,112,12,128,50,58,26,32,140,85,171,128,140,182,52,8,96,104,47,7,131,50,134,118,99,115,115,115,99,115,32,195,188,28,200,130,137,152,27,3,25,229,64,9,243,98,176,72,113,185,49,88,4,170,11,110,14,200,82,176,201,28,96,119,52,48,0,0,112,70,49,22,48,158,177,236,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    fin = fopen(file.c_str(), "r");

    if(!fin)
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    if(render_to_mem(fin, &buf, &image.w, &image.h))
	return SQE_R_BADFILE;

    image.bpp = 32;
    image.hasalpha = true;

    line = -1;

    image.compression = "-";
    image.colorspace = "Vectorized";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;
    fmt_image *im = image(currentImage);

    RGBA *nbuf = (RGBA *)(buf + line * im->w * sizeof(RGBA));

    for(s32 i = 0;i < im->w;i++)
    {
	(scan+i)->r = (nbuf+i)->b;
	(scan+i)->g = (nbuf+i)->g;
	(scan+i)->b = (nbuf+i)->r;
	(scan+i)->a = (nbuf+i)->a;
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    if(fin)
	fclose(fin);

    finfo.meta.clear();
    finfo.image.clear();

    if(buf)
	free(buf);
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->passes = 1;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
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

#include "fmt_codec_cd_func.h"

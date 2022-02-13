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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_svg_defs.h"
#include "fmt_codec_svg.h"

#include "svg2mem.h"
#include "ksquirrel-libs/error.h"

#include "../xpm/codec_svg.xpm"

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

void fmt_codec::fill_default_settings()
{
    settings_value val;

    // scale factor in percents
    val.type = settings_value::v_int;
    val.iVal = 100;

    m_settings["scale"] = val;
}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.2";
    o->name = "Scalable Vector Graphics";
    o->filter = "*.svg *.svgz ";
    o->config = std::string(SVG_UI); // SVG_UI comes from Makefile.am
    o->mime = "";
    o->mimetype = "image/svg+xml";
    o->pixmap = codec_svg;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    fin = fopen(file.c_str(), "r");

    if(!fin)
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    fmt_settings::iterator it = m_settings.find("scale");

    // percents / 100
    double scale = (it == m_settings.end() || (*it).second.type != settings_value::v_int)
             ? 1.0 : (double)((*it).second.iVal) / 100.0;

    if(render_to_mem(fin, &buf, &image.w, &image.h, scale))
	return SQE_R_BADFILE;

    image.bpp = 32;
    image.hasalpha = true;

    line = -1;

    image.compression = "-";
    image.colorspace = "Vectorized";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    line++;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

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

void fmt_codec::read_close()
{
    if(fin)
	fclose(fin);

    finfo.meta.clear();
    finfo.image.clear();

    if(buf)
	free(buf);
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
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

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
{
    cerr << "libSQ_codec_svg: using cairo, libpixman, libsvg and libsgv-cairo from CVS (25.05.2005)" << endl;
}

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,216,104,0,76,76,76,113,150,101,53,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,94,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,145,145,83,35,129,140,200,153,64,22,76,36,50,20,200,152,25,9,100,78,5,139,76,157,57,21,44,2,213,5,55,7,100,41,216,100,46,176,59,22,48,0,0,210,219,55,226,98,232,155,195,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(std::string file)
{
    fin = fopen(file.c_str(), "r");

    if(!fin)
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

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

    if(render_to_mem(fin, &buf, &finfo.image[currentImage].w, &finfo.image[currentImage].h))
	return SQE_R_BADFILE;

    finfo.image[currentImage].bpp = 32;
    finfo.image[currentImage].hasalpha = true;

    line = -1;

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = "Vectorized";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;

    RGBA *nbuf = (RGBA *)(buf + line * finfo.image[currentImage].w * sizeof(RGBA));

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
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
}

s32 fmt_codec::fmt_write_init(std::string file, const fmt_image &image, const fmt_writeoptions &opt)
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

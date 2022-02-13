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
#include "fmt_utils.h"

#include "fmt_codec_mdl_defs.h"
#include "fmt_codec_mdl.h"

#include "error.h"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.2.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("HalfLife model");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.mdl ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,149,10,209,76,76,76,191,74,134,86,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,85,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,145,83,167,134,78,5,49,66,67,167,78,157,10,21,193,100,132,78,13,157,129,208,5,55,7,100,41,216,100,46,176,59,22,48,0,0,142,246,58,242,100,233,81,22,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;
    finfo.images = 0;

    s32 id, ver;

    if(!frs.readK(&id, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&ver, sizeof(s32))) return SQE_R_BADFILE;

    if(id != 0x54534449 || ver != 10)
	return SQE_R_BADFILE;

    frs.seekg(172, ios_base::cur);

    if(!frs.readK(&numtex, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&texoff, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&texdataoff, sizeof(s32))) return SQE_R_BADFILE;

    if(!numtex || !texoff || !texdataoff)
	return SQE_R_BADFILE;

    frs.seekg(texoff, ios_base::beg);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage == numtex)
        return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    if(currentImage)
	frs.seekg(opos);

    if(!frs.readK(tex.name, sizeof(tex.name))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.flags, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.width, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.height, sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&tex.offset, sizeof(s32))) return SQE_R_BADFILE;
    
    opos = frs.tellg();

    if(!tex.offset)
        return SQE_R_BADFILE;

    frs.seekg(tex.offset, ios_base::beg);
    
    if(!frs.good())
	return SQE_R_BADFILE;

    finfo.image[currentImage].w = tex.width;
    finfo.image[currentImage].h = tex.height;

    fstream::pos_type pos = frs.tellg();

    frs.seekg(tex.width * tex.height, ios_base::cur);

    if(!frs.readK(pal, sizeof(RGB) * 256)) return SQE_R_BADFILE;

    frs.seekg(pos);

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = fmt_utils::colorSpaceByBpp(8);

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    u8 index;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    for(s32 x = 0;x < finfo.image[currentImage].w;x++)
    {
	if(!frs.readK(&index, sizeof(u8))) return SQE_R_BADFILE;

	memcpy(scan+x, pal+index, sizeof(RGB));
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();
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

s32 fmt_codec::fmt_write_scanline(RGBA * /*scan*/)
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

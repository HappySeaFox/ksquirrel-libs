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

#include "fmt_codec_hdr_defs.h"
#include "fmt_codec_hdr.h"

#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "../xpm/codec_hdr.xpm"

/*
 *
 * Radiance HDR image
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.0";
    o->name = "Radiance HDR image";
    o->filter = "*.hdr ";
    o->config = "";
    o->mime  = "#.RADIANCE";
    o->mimetype = "image/x-hdr";
    o->pixmap = codec_hdr;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    scanline = 0;

    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    if(!getHdrHead()) return SQE_R_BADFILE;

    if(strcmp(hdr.sig, "#?RADIANCE")) return SQE_R_BADFILE;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

	fmt_image image;

    image.w = hdr.width;
    image.h = hdr.height;
    image.bpp = 32;

    scanline = new u8 [hdr.width * sizeof(RGBA)];
    
    if(!scanline) return SQE_R_NOMEMORY;

    image.compression = "RGBE";
    image.colorspace = fmt_utils::colorSpaceByBpp(32);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    u32 r, g, b, e, i, j;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(!read_scan(scanline, im->w))
	return SQE_R_BADFILE;

    for(j = 0, i = 0; j < (u32)im->w * 4; j += 4, i++)
    {
        float t;

        e = scanline[j + 3];
        r = scanline[j + 0];
        g = scanline[j + 1];
        b = scanline[j + 2];

        //t = (float)pow(2.f, ((ILint)e) - 128);
        if (e != 0)
	    e = (e - 1) << 23;

        t = *(float *)&e;

        (scan + i)->r = u8((r / 255.0f) * t);
        (scan + i)->g = u8((g / 255.0f) * t);
        (scan + i)->b = u8((b / 255.0f) * t);
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    delete [] scanline;
    scanline = 0;
}

//
// These two methods were taken from DevIL
// http://imagelib.org
//

bool fmt_codec::read_scan(u8 *scanline, const s32 w)
{
	u8 *runner, c;
	u32 r, g, b, e, read, shift;

	if(!frs.readK(&c, sizeof(u8))) return false; r = c;
	if(!frs.readK(&c, sizeof(u8))) return false; g = c;
	if(!frs.readK(&c, sizeof(u8))) return false; b = c;
	if(!frs.readK(&c, sizeof(u8))) return false; e = c;

	//check if the scanline is in the new format
	//if so, e, r, g, g are stored separated and are
	//rle-compressed independently.
	if (r == 2 && g == 2)
	{
		u32 length = (b << 8) | e;
		u32 j, t, k;

		if (length > (u32)w)
			length = w; //fix broken files

		for(k = 0; k < 4; ++k)
		{
			runner = scanline + k;
			j = 0;

			while (j < length)
			{
				if(!frs.readK(&c, sizeof(u8))) return false;

				t = c;

				if(t > 128)
				{ //Run?
					if(!frs.readK(&c, sizeof(u8))) return false;

					t &= 127;

					//copy current byte
					while (t > 0 && j < length)
					{
						*runner = c;
						runner += 4;
						--t;
						++j;
					}
				}
				else
				{ //No Run.
					//read new bytes
					while (t > 0 && j < length)
					{
						if(!frs.readK(&c, sizeof(u8))) return false;

						*runner = c;
						runner += 4;
						--t;
						++j;
					}
				}
			}
		}

		return true; //done decoding a scanline in separated format
	}

	//if we come here, we are dealing with old-style scanlines
	shift = 0;
	read = 0;
	runner = scanline;

	while(read < (u32)w)
	{
		if (read != 0)
		{
		    if(!frs.readK(&c, sizeof(u8))) return false; r = c;
		    if(!frs.readK(&c, sizeof(u8))) return false; g = c;
		    if(!frs.readK(&c, sizeof(u8))) return false; b = c;
		    if(!frs.readK(&c, sizeof(u8))) return false; e = c;
		}

		//if all three mantissas are 1, then this is a rle
		//count dword
		if (r == 1 && g == 1 && b == 1)
		{
			u32 length = e;
			u32 j;

			for (j = length << shift; j > 0; --j)
			{
				memcpy(runner, runner - 4, 4);
				runner += 4;
			}
			//if more than one rle count dword is read
			//consecutively, they are higher order bytes
			//of the first read value. shift keeps track of
			//that.
			shift += 8;
			read += length;
		}
		else
		{
			runner[0] = r;
			runner[1] = g;
			runner[2] = b;
			runner[3] = e;

			shift = 0;
			runner += 4;
			++read;
		}
	}

    return true;
}

bool fmt_codec::getHdrHead()
{
	bool done = false;
	s8 a, b;
	s8 x[2], y[2];
	s8 buff[80];
	u32 count = 0;

	if(!frs.readK(hdr.sig, sizeof(hdr.sig)-1)) return false;

	hdr.sig[10] = '\0';

	//skip lines until an empty line is found.
	//this marks the end of header information,
	//the next line contains the image's dimension.

	//TODO: read header contents into variables
	//(EXPOSURE, orientation, xyz correction, ...)

	if(!frs.readK(&a, sizeof(s8)))
		return false;

	while(!done)
	{
		if(!frs.readK(&b, sizeof(s8))) return false;

		if (b == '\n' && a == '\n')
			done = true;
		else
			a = b;
	}

	//read dimensions (note that this assumes a somewhat valid image)
	if(!frs.readK(&a, sizeof(s8))) return false;

	while (a != '\n')
	{
		buff[count] = a;

		if(!frs.readK(&a, sizeof(s8))) return false;

		++count;
	}

	buff[count] = '\0';

	sscanf(buff, "%s %d %s %d", x, &hdr.width, y, &hdr.height);

	return true;
}

#include "fmt_codec_cd_func.h"

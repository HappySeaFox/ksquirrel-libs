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
#include <exception>

#include "fmt_types.h"
#include "fileio.h"

#include <ImfStandardAttributes.h>
#include <ImathBox.h>
#include <ImfInputFile.h>
#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfConvert.h>

#include "fmt_codec_openexr_defs.h"
#include "fmt_codec_openexr.h"

#include "error.h"

RGBA RgbaToRGBA(struct Rgba);

/*
 *
 *   A high-dynamic-range image format from Industrial Light & Magic
 *   for use in digital visual effects production.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.2.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("OpenEXR");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.exr ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,23,1,0,78,78,78,254,182,2,202,202,202,70,70,70,254,254,254,178,178,178,174,174,174,242,242,242,222,222,222,2,2,2,216,221,231,174,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,88,73,68,65,84,120,218,99,16,4,129,4,6,6,6,97,99,99,99,195,233,32,70,104,104,168,97,103,2,148,33,40,0,97,24,27,3,25,34,46,16,192,32,164,4,6,42,12,66,171,86,105,105,105,173,82,97,88,180,72,9,200,208,2,50,86,45,82,90,132,38,178,10,194,128,235,130,155,3,178,20,108,50,35,216,29,2,12,0,240,94,28,48,28,222,87,31,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &fl)
{
    frs.open(fl.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    frs.close();

    currentImage = -1;
    read_error = false;

    pixels = NULL;
    file = fl;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    fmt_image image;

    s32 width, height;

    RgbaInputFile *in = NULL;

    pixels = new Array2D<Rgba>;

    try
    {
	in = new RgbaInputFile(file.c_str());

	Imath::Box2i dw = in->dataWindow();

	width  = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;

	pixels->resizeErase(height, width);

	in->setFrameBuffer(&(*pixels)[0][0] - dw.min.x - dw.min.y * width, 1, width);

	in->readPixels(dw.min.y, dw.max.y);
    }
    catch(const exception &e)
    {
	cerr << e.what() << endl;

	delete in;

	return SQE_R_BADFILE;
    }

    switch(in->compression())
    {
	case Imf::NO_COMPRESSION:
	    image.compression = "-";
	break;

	case Imf::RLE_COMPRESSION:
	    image.compression = "RLE";
	break;

	case Imf::ZIPS_COMPRESSION:
	    image.compression = "ZIPS";
	break;

	case Imf::ZIP_COMPRESSION:
	    image.compression = "ZIP";
	break;

	case Imf::PIZ_COMPRESSION:
	    image.compression = "PIZ";
	break;

	case Imf::PXR24_COMPRESSION:
	    image.compression = "PXR24";
	break;

	case Imf::NUM_COMPRESSION_METHODS:
	    image.compression = "Different methods";
	break;
	
	default:
	    image.compression = "Unknown";
    }

    image.colorspace = "RGBA";
    image.bpp = 32;
    image.w = width;
    image.h = height;

    finfo.image.push_back(image);

    line = -1;

    delete in;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGBA rgba;
    fmt_image *im = image(currentImage);
    
    line++;

    memset(scan, 255, im->w * sizeof(RGBA));

    for(s32 x = 0; x < im->w; x++)
    {
	rgba = RgbaToRGBA((*pixels)[line][x]);
	memcpy(scan+x, &rgba, sizeof(RGBA));
    }

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    finfo.meta.clear();
    finfo.image.clear();

    delete pixels;
    pixels = NULL;
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

    fws.close();

    out = NULL;
    hs = NULL;

    out = new RgbaOutputFile(file.c_str(), image.w, image.h, WRITE_RGBA);

    if(!out)
	return SQE_W_NOMEMORY;

    hs = new Rgba [image.w];

    if(!hs)
	return SQE_W_NOMEMORY;

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
    for(s32 i = 0;i < writeimage.w;i++)
    {
	hs[i].r = scan[i].r;
	hs[i].g = scan[i].g;
	hs[i].b = scan[i].b;
	hs[i].a = scan[i].a;
    }

    out->setFrameBuffer(hs, 1, 0);

    out->writePixels(1);

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    delete out;
    out = NULL;

    delete hs;
    hs = NULL;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

/* 
 * utility function
 *
 * this does a conversion from the ILM Half (equal to Nvidia Half)
 * format into the normal 32 bit pixel format. Process is from the
 * ILM code.
 *
 */

RGBA RgbaToRGBA(struct Rgba imagePixel)
{
	float r,g,b,a;
	
	//  1) Compensate for fogging by subtracting defog
	//     from the raw pixel values.
	// Response: We work with defog of 0.0, so this is a no-op        

	//  2) Multiply the defogged pixel values by
	//     2^(exposure + 2.47393).
	// Response: We work with exposure of 0.0.
	// (2^2.47393) is 5.55555 
	r = imagePixel.r * 5.55555;
	g = imagePixel.g * 5.55555;
	b = imagePixel.b * 5.55555;
	a = imagePixel.a * 5.55555;

	//  3) Values, which are now 1.0, are called "middle gray".
	//     If defog and exposure are both set to 0.0, then
	//     middle gray corresponds to a raw pixel value of 0.18.
	//     In step 6, middle gray values will be mapped to an
	//     intensity 3.5 f-stops below the display's maximum
	//     intensity.
	// Response: no apparent content.

	//  4) Apply a knee function.  The knee function has two
	//     parameters, kneeLow and kneeHigh.  Pixel values
	//     below 2^kneeLow are not changed by the knee
	//     function.  Pixel values above kneeLow are lowered
	//     according to a logarithmic curve, such that the
	//     value 2^kneeHigh is mapped to 2^3.5 (in step 6,
	//     this value will be mapped to the the display's
	//     maximum intensity).
	// Response: kneeLow = 0.0 (2^0.0 => 1); kneeHigh = 5.0 (2^5 =>32)
    if (r > 1.0)
		r = 1.0 + Imath::Math<float>::log ((r-1.0) * 0.184874 + 1) / 0.184874;
    if (g > 1.0)
		g = 1.0 + Imath::Math<float>::log ((g-1.0) * 0.184874 + 1) / 0.184874;
    if (b > 1.0)
		b = 1.0 + Imath::Math<float>::log ((b-1.0) * 0.184874 + 1) / 0.184874;
    if (a > 1.0)
		a = 1.0 + Imath::Math<float>::log ((a-1.0) * 0.184874 + 1) / 0.184874;
//
//  5) Gamma-correct the pixel values, assuming that the
//     screen's gamma is 0.4545 (or 1/2.2).
    r = Imath::Math<float>::pow (r, 0.4545);
    g = Imath::Math<float>::pow (g, 0.4545);
    b = Imath::Math<float>::pow (b, 0.4545);
    a = Imath::Math<float>::pow (a, 0.4545);

//  6) Scale the values such that pixels middle gray
//     pixels are mapped to 84.66 (or 3.5 f-stops below
//     the display's maximum intensity).
//
//  7) Clamp the values to [0, 255].
	return RGBA( s8 (Imath::clamp ( r * 84.66f, 0.f, 255.f ) ),
				  s8 (Imath::clamp ( g * 84.66f, 0.f, 255.f ) ),
				  s8 (Imath::clamp ( b * 84.66f, 0.f, 255.f ) ),
				  s8 (Imath::clamp ( a * 84.66f, 0.f, 255.f ) ) );
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("exr");
}

#include "fmt_codec_cd_func.h"

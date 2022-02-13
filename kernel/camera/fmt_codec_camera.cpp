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
#include "fmt_utils.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_camera_defs.h"
#include "fmt_codec_camera.h"

using namespace fmt_utils;

extern "C" int sqcall(int, char **);


/*
 *
 * Supports all photo formats, which are supported by dcraw.c.
 *
 * Visit http://www.cybercom.net/~dcoffin/dcraw/ for further information.
 *
 * dcraw.c revision 1.220 supports
 * the cameras listed below
 * 
 * Adobe Digital Negative (DNG) 
 * Canon PowerShot 600 
 * Canon PowerShot A5 
 * Canon PowerShot A5 Zoom 
 * Canon PowerShot A50 
 * Canon PowerShot Pro70 
 * Canon PowerShot Pro90 IS 
 * Canon PowerShot G1 
 * Canon PowerShot G2 
 * Canon PowerShot G3 
 * Canon PowerShot G5 
 * Canon PowerShot G6 
 * Canon PowerShot S30 
 * Canon PowerShot S40 
 * Canon PowerShot S45 
 * Canon PowerShot S50 
 * Canon PowerShot S60 
 * Canon PowerShot S70 
 * Canon PowerShot Pro1 
 * Canon EOS D30 
 * Canon EOS D60 
 * Canon EOS 10D 
 * Canon EOS 20D 
 * Canon EOS 300D 
 * Canon EOS 350D 
 * Canon EOS Digital Rebel 
 * Canon EOS Digital Rebel XT 
 * Canon EOS Kiss Digital 
 * Canon EOS D2000C 
 * Canon EOS-1D 
 * Canon EOS-1DS 
 * Canon EOS-1D Mark II 
 * Canon EOS-1Ds Mark II 
 * Casio QV-2000UX 
 * Casio QV-3000EX 
 * Casio QV-3500EX 
 * Casio QV-4000 
 * Casio QV-5700 
 * Casio QV-R51 
 * Casio EX-Z50 
 * Casio EX-Z55 
 * Casio Exlim Pro 600 
 * Casio Exlim Pro 700 
 * Contax N Digital 
 * Creative PC-CAM 600 
 * Epson R-D1 
 * Fuji FinePix E550 
 * Fuji FinePix F700 
 * Fuji FinePix F710 
 * Fuji FinePix F800 
 * Fuji FinePix F810 
 * Fuji FinePix S2Pro 
 * Fuji FinePix S3Pro 
 * Fuji FinePix S20Pro 
 * Fuji FinePix S5000 
 * Fuji FinePix S5100/S5500 
 * Fuji FinePix S7000 
 * Imacon Ixpress 16-megapixel 
 * Imacon Ixpress 22-megapixel 
 * Kodak DC20 (see Oliver Hartman's page) 
 * Kodak DC25 (see Jun-ichiro Itoh's page) 
 * Kodak DC40 
 * Kodak DC50 
 * Kodak DC120 (also try kdc2tiff) 
 * Kodak DCS315C 
 * Kodak DCS330C 
 * Kodak DCS420 
 * Kodak DCS460 
 * Kodak DCS460A 
 * Kodak DCS520C 
 * Kodak DCS560C 
 * Kodak DCS620C 
 * Kodak DCS620X 
 * Kodak DCS660C 
 * Kodak DCS660M 
 * Kodak DCS720X 
 * Kodak DCS760C 
 * Kodak DCS760M 
 * Kodak EOSDCS1 
 * Kodak EOSDCS3B 
 * Kodak NC2000F 
 * Kodak ProBack 
 * Kodak PB645C 
 * Kodak PB645H 
 * Kodak PB645M 
 * Kodak DCS Pro 14n 
 * Kodak DCS Pro 14nx 
 * Kodak DCS Pro SLR/c 
 * Kodak DCS Pro SLR/n 
 * Konica KD-400Z 
 * Konica KD-510Z 
 * Leaf Valeo 11 
 * Leaf Valeo 22 
 * Leaf Volare 
 * Leica Digilux 2 
 * Logitech Fotoman Pixtura 
 * Minolta DiMAGE 5 
 * Minolta DiMAGE 7 
 * Minolta DiMAGE 7i 
 * Minolta DiMAGE 7Hi 
 * Minolta DiMAGE A1 
 * Minolta DiMAGE A2 
 * Minolta DiMAGE A200 
 * Minolta DiMAGE G400 
 * Minolta DiMAGE G500 
 * Minolta DiMAGE G600 
 * Minolta DiMAGE Z2 
 * Minolta Alpha/Dynax/Maxxum 7 
 * Minolta Alpha/Dynax/Maxxum 7D 
 * Nikon D1 
 * Nikon D1H 
 * Nikon D1X 
 * Nikon D100 
 * Nikon D2H 
 * Nikon D2X 
 * Nikon D70 
 * Nikon E700 ("DIAG RAW" hack) 
 * Nikon E800 ("DIAG RAW" hack) 
 * Nikon E900 ("DIAG RAW" hack) 
 * Nikon E950 ("DIAG RAW" hack) 
 * Nikon E990 ("DIAG RAW" hack) 
 * Nikon E995 ("DIAG RAW" hack) 
 * Nikon E2100 ("DIAG RAW" hack) 
 * Nikon E2500 ("DIAG RAW" hack) 
 * Nikon E3700 ("DIAG RAW" hack) 
 * Nikon E4300 ("DIAG RAW" hack) 
 * Nikon E4500 ("DIAG RAW" hack) 
 * Nikon E5000 
 * Nikon E5400 
 * Nikon E5700 
 * Nikon E8400 
 * Nikon E8700 
 * Nikon E8800 
 * Olympus C5050Z 
 * Olympus C5060WZ 
 * Olympus C70Z,C7000Z 
 * Olympus C8080WZ 
 * Olympus E-1 
 * Olympus E-10 
 * Olympus E-20 
 * Olympus E-300 
 * Panasonic DMC-LC1 
 * Pentax *ist D 
 * Pentax *ist DS 
 * Pentax Optio S 
 * Pentax Optio S4 
 * Pentax Optio 33WR 
 * Phase One LightPhase 
 * Phase One H10 
 * Phase One H20 
 * Phase One H25 
 * Rollei d530flex 
 * Sigma SD9 
 * Sigma SD10 
 * Sinar 12582980-byte 
 * Sony DSC-F828 
 * Sony DSC-V3 
 * STV680 VGA
 * 
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    cerr << "libSQ_codec_camera: using dcraw revision 1.220" << endl;
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.9.0");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Photos from different cameras");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.crw *.kdc ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,255,0,164,76,76,76,0,116,226,101,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,161,145,83,103,70,78,5,50,34,35,67,35,67,67,65,140,153,145,145,112,17,48,3,72,65,24,48,93,112,115,64,150,130,77,230,2,187,99,1,3,0,208,195,55,158,32,175,83,131,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    frs.close();

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    ref = fmt_utils::adjustTempName(file, ".rawrgb");
    orig = file;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    const char * argv[5] = 
    {
	"dcraw",
	"-2",
	"-o",
	ref.c_str(),
	orig.c_str()
    };

    sqcall(5, (char **)argv);

    frs.open(ref.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    if(!frs.readK(&finfo.image[currentImage].w,   sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&finfo.image[currentImage].h,   sizeof(s32))) return SQE_R_BADFILE;
    if(!frs.readK(&finfo.image[currentImage].bpp, sizeof(s32))) return SQE_R_BADFILE;

    if(finfo.image[currentImage].bpp != 24 && finfo.image[currentImage].bpp != 32)
	return SQE_R_BADFILE;

    finfo.images++;
    finfo.image[currentImage].compression = "?"; 
    finfo.image[currentImage].colorspace = "?";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    RGB rgb;
    RGBA rgba;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    if(finfo.image[currentImage].bpp == 32)
        for(s32 i = 0;i < finfo.image[currentImage].w;i++)
        {
            frs.readK(&rgba, sizeof(RGBA));
            memcpy(scan+i, &rgba, sizeof(RGBA));
        }
    else
	for(s32 i = 0;i < finfo.image[currentImage].w;i++)
	{
            frs.readK(&rgb, sizeof(RGB));
            memcpy(scan+i, &rgb, sizeof(RGB));
	}

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    unlink(ref.c_str());
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

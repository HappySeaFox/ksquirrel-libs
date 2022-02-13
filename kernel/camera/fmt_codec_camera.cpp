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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_camera_defs.h"
#include "fmt_codec_camera.h"

#include "error.h"

extern "C" int sqcall(int, char **);

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,65,0,0,0,192,192,192,255,255,255,255,0,164,255,11,167,4,4,4,235,25,237,53,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,92,73,68,65,84,120,218,69,142,193,13,128,48,12,3,189,2,208,9,242,224,143,92,6,168,146,13,202,0,168,143,238,63,2,73,91,137,123,157,172,200,14,128,30,192,217,69,228,12,105,36,207,190,196,163,109,129,100,206,99,5,137,166,185,218,133,195,178,102,101,136,155,206,132,12,241,155,187,14,177,65,249,123,102,51,99,172,201,90,29,111,188,31,119,30,23,212,80,21,8,48,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    frs.close();

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    ref = fmt_utils::adjustTempName(file, ".rawrgb");
    orig = file;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage)
        return SQERR_NOTOK;

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
        return SQERR_NOFILE;

    if(!frs.readK(&finfo.image[currentImage].w,   sizeof(s32))) return SQERR_BADFILE;
    if(!frs.readK(&finfo.image[currentImage].h,   sizeof(s32))) return SQERR_BADFILE;
    if(!frs.readK(&finfo.image[currentImage].bpp, sizeof(s32))) return SQERR_BADFILE;

    if(finfo.image[currentImage].bpp != 24 && finfo.image[currentImage].bpp != 32)
	return SQERR_BADFILE;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "?" << "\n"
        << "?" << "\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
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

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32                 w, h, bpp;
    s32                 m_bytes;
    ifstreamK           m_frs;

    std::string m_ref = fmt_utils::adjustTempName(file, ".rawrgb");
    std::string m_orig = file;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    m_frs.close();

    const char * argv[5] = 
    {
	"dcraw",
	"-2",
	"-o",
	m_ref.c_str(),
	m_orig.c_str()
    };

    sqcall(5, (char **)argv);

    m_frs.open(m_ref.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(!m_frs.readK(&w,   sizeof(s32))) return SQERR_BADFILE;
    if(!m_frs.readK(&h,   sizeof(s32))) return SQERR_BADFILE;
    if(!m_frs.readK(&bpp, sizeof(s32))) return SQERR_BADFILE;

    if(bpp != 24 && bpp != 32)
	return SQERR_BADFILE;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "?" << "\n"
        << "?" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA    *scan = *image + h2 * w;

	RGB rgb;
	RGBA rgba;

	if(bpp == 32)
    	    for(s32 i = 0;i < w;i++)
    	    {
    		m_frs.readK(&rgba, sizeof(RGBA));
        	memcpy(scan+i, &rgba, sizeof(RGBA));
    	    }
	else
	    for(s32 i = 0;i < w;i++)
	    {
        	m_frs.readK(&rgb, sizeof(RGB));
        	memcpy(scan+i, &rgb, sizeof(RGB));
	    }
    }

    m_frs.close();

    unlink(m_ref.c_str());

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string, RGBA *, s32, s32, const fmt_writeoptions &)
{
    return SQERR_NOTSUPPORTED;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

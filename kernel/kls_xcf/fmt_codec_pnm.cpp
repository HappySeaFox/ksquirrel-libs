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

// we will use fork()
#if defined CODEC_DJVU  \
        || defined CODEC_CAMERA \
        || defined CODEC_DXF    \
        || defined CODEC_XCF    \
        || defined CODEC_LBM    \
        || defined CODEC_NETPBM
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>
#endif

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_pnm_defs.h"
#include "fmt_codec_pnm.h"

#if defined CODEC_CAMERA
#include "../xpm/codec_camera.xpm"
#elif defined CODEC_DJVU
#include "../xpm/codec_djvu.xpm"
#elif defined CODEC_XCF
#include "../xpm/codec_xcf.xpm"
#elif defined CODEC_DXF
#include "../xpm/codec_dxf.xpm"
#elif defined CODEC_LBM
#include "../xpm/codec_lbm.xpm"
#elif defined CODEC_NEO
#include "../xpm/codec_neo.xpm"
#elif defined CODEC_FITS
#include "../xpm/codec_fits.xpm"
#elif defined CODEC_LEAF
#include "../xpm/codec_leaf.xpm"
#elif defined CODEC_PI1
#include "../xpm/codec_pi1.xpm"
#elif defined CODEC_XIM
#include "../xpm/codec_xim.xpm"
#elif defined CODEC_UTAH
#include "../xpm/codec_utah.xpm"
#elif defined CODEC_PICT
#include "../xpm/codec_pict.xpm"
#elif defined CODEC_IFF
#include "../xpm/codec_iff.xpm"
#elif defined CODEC_MAC
#include "../xpm/codec_mac.xpm"
#else
#include "../xpm/codec_pnm.xpm"
#endif

/*
 *
 * PBM, PGM,
 * PNM, and PPM are
 * intermediate formats used in the conversion of many little known
 * formats via pbmplus, the Portable Bitmap Utilities. These
 * formats are mainly available under UNIX and
 * on Intel-based PCs.
 *
 */

static RGB palmono[2] = {RGB(255,255,255), RGB(0,0,0)};

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{
#ifdef CODEC_DXF
    std::string tmmp = tmp + ".ppm";
    unlink(tmmp.c_str());
#endif
}

void fmt_codec::options(codec_options *o)
{
#if defined CODEC_CAMERA
    o->version = "1.0.0";
    o->name = "Photos from different cameras";
    o->filter = "*.arw *.bay *.bmq *.cr2 *.crw *.cs1 *.dc2 *.dcr *.dng *.erf *.fff *.hdr *.ia *.k25 *.kc2 *.kdc *.mdc *.mos *.mrw *.nef *.orf *.pef *.pxn *.raf *.raw *.rdc *.sr2 *.srf *.sti *.x3f ";
    o->config = std::string(CAMERA_UI);
    o->mime = "";
    o->pixmap = codec_camera;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_DJVU
    o->version = "1.0.0";
    o->name = "DjVu Document";
    o->filter = "*.djvu *.djv *.iw4 *.iw44 ";
    o->config = std::string(DJVU_UI);
    o->mime = "";
    o->pixmap = codec_djvu;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_DXF
    o->version = "1.0.0";
    o->name = "AutoCAD/QCAD Drawing";
    o->filter = "*.dxf ";
    o->config = std::string(DXF_UI);
    o->mime = "";
    o->pixmap = codec_dxf;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_XCF
    o->version = "1.0.0";
    o->name = "GIMP XCF";
    o->filter = "*.xcf ";
    o->config = std::string(XCF_UI);
    o->mime = "";
    o->pixmap = codec_xcf;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_LBM
    o->version = "1.0.0";
    o->name = "PC Deluxe Paint II LBM";
    o->filter = "*.lbm ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_lbm;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_NEO
    o->version = "1.0.0";
    o->name = "Neochrome NEO";
    o->filter = "*.neo ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_neo;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_FITS
    o->version = "1.0.0";
    o->name = "FITS";
    o->filter = "*.fits ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_fits;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_LEAF
    o->version = "1.0.0";
    o->name = "ILEAF Image";
    o->filter = "*.leaf ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_leaf;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_PI1
    o->version = "1.0.0";
    o->name = "Degas PI1";
    o->filter = "*.pi1 ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_pi1;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_XIM
    o->version = "1.0.0";
    o->name = "X IMage";
    o->filter = "*.xim ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_xim;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_UTAH
    o->version = "1.0.0";
    o->name = "UTAH RLE";
    o->filter = "*.rle ";
    o->config = "";
    o->mime = "\x0052\x00CC";
    o->pixmap = codec_utah;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_PICT
    o->version = "1.0.0";
    o->name = "Macintosh PICT";
    o->filter = "*.pict ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_pict;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_IFF
    o->version = "1.0.0";
    o->name = "Interchange File Format";
    o->filter = "*.iff *.ilbm *.lbm ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_iff;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#elif defined CODEC_MAC
    o->version = "1.0.0";
    o->name = "Macintosh Paint";
    o->filter = "*.mac ";
    o->config = "";
    o->mime = "";
    o->pixmap = codec_mac;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = true;
#else
    o->version = "0.6.4";
    o->name = "Portable aNy Map";
    o->filter = "*.pnm *.pgm *.pbm *.ppm ";
    o->config = "";
    o->mime = "P[123456]";
    o->pixmap = codec_pnm;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = true;
    o->writeanimated = false;
    o->needtempfile = false;
#endif
}

#if defined CODEC_CAMERA
void fmt_codec::fill_default_settings()
{
    settings_value val;

    // scale factor in percents
    val.type = settings_value::v_bool;

    val.bVal = true;
    m_settings["half_size"] = val;
    m_settings["automatic_white"] = val;
    m_settings["camera_white"] = val;

    val.bVal = false;
    m_settings["dontstretch"] = val;
    m_settings["camera_date"] = val;
    m_settings["document_mode"] = val;
    m_settings["interpolate_rggb"] = val;
    m_settings["icc_cam"] = val;

    val.type = settings_value::v_int;
    val.iVal = 0;
    m_settings["highlights"] = val;
    m_settings["different"] = val;
    m_settings["flipping"] = val; // 0 means camera settings

    val.iVal = 0; // 1,2,3 are accepted numbers. 1 means no interpolation
    m_settings["quick"] = val;
    val.iVal = -1; // don't use black pixel
    m_settings["black"] = val;
    val.iVal = 99; // values 100...1000 are accepted. 99 means no threshold
    m_settings["threshold"] = val;

    val.type = settings_value::v_double;
    val.dVal = 1.0;
    m_settings["brightness"] = val;

    val.type = settings_value::v_string;
    val.sVal = "";
    m_settings["icc_file"] = val;
}
#elif defined CODEC_DJVU
void fmt_codec::fill_default_settings()
{
    settings_value val;

    // page number
    val.type = settings_value::v_int;

    val.iVal = 1;
    m_settings["page"] = val;

    val.iVal = 2;
    m_settings["scaledown"] = val;
}
#elif defined CODEC_DXF
void fmt_codec::fill_default_settings()
{
    settings_value val;

    // page number
    val.type = settings_value::v_int;

    val.iVal = 0;
    m_settings["width"] = val;
    val.iVal = 0;
    m_settings["height"] = val;
}
#elif defined CODEC_XCF
void fmt_codec::fill_default_settings()
{
    settings_value val;

    // background color
    val.type = settings_value::v_string;
    val.sVal = "#ffffff";
    m_settings["background"] = val;

    val.type = settings_value::v_bool;
    val.bVal = false;
    m_settings["autocrop"] = val;
}
#endif

s32 fmt_codec::read_init(const std::string &file)
{
    fptr = 0;

#if defined CODEC_CAMERA
    std::vector<std::string> params;
    int status;

    bool half_size,
    dontstretch,
    camera_date,
    automatic_white,
    camera_white,
    document_mode,
    interpolate_rggb,
    icc_cam;

    int quick,
    threshold,
    black,
    different,
    highlights,
    flipping;

    double brightness;

    std::string icc_file;

    fmt_settings::iterator it = m_settings.find("half_size");
    half_size = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    true : (*it).second.bVal;

    it = m_settings.find("icc_cam");
    icc_cam = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    it = m_settings.find("dontstretch");
    dontstretch = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    it = m_settings.find("camera_date");
    camera_date = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    it = m_settings.find("automatic_white");
    automatic_white = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    true : (*it).second.bVal;

    it = m_settings.find("camera_white");
    camera_white = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    true : (*it).second.bVal;

    it = m_settings.find("document_mode");
    document_mode = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    it = m_settings.find("interpolate_rggb");
    interpolate_rggb = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    it = m_settings.find("quick");
    quick = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    1 : (*it).second.iVal;

    if(quick < 0 || quick > 3)
        quick = 0;

    it = m_settings.find("highlights");
    highlights = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    0 : (*it).second.iVal;

    if(highlights < 0 || highlights > 9)
        highlights = 0; // revert to default value

    it = m_settings.find("flipping");
    flipping = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    0 : (*it).second.iVal;

    if(flipping != 0 && flipping != 1 && flipping != 3 && flipping != 5 && flipping != 6)
        flipping = 0; // revert to default value

    it = m_settings.find("black");
    black = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    -1 : (*it).second.iVal;

    if(black < -1 || black > 255)
        black = -1;

    it = m_settings.find("different");
    different = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    0 : (*it).second.iVal;

    if(different < 0 || different > 99)
        different = 0; // revert to default

    it = m_settings.find("threshold");
    threshold = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    100 : (*it).second.iVal;

    if(threshold < 99 || threshold > 1000)
        threshold = 99; // revert to default

    it = m_settings.find("brightness");
    brightness = (it == m_settings.end() || (*it).second.type != settings_value::v_double) ?
                    1.0 : (*it).second.dVal;

    if(brightness < 0 || brightness > 255)
        brightness = 1.0;

    it = m_settings.find("icc_file");
    icc_file = (it == m_settings.end() || (*it).second.type != settings_value::v_string) ?
                    "" : (*it).second.sVal;

    if(half_size) params.push_back("-h");
    if(dontstretch) params.push_back("-j");
    if(camera_date) params.push_back("-z");
    if(automatic_white) params.push_back("-a");
    if(camera_white) params.push_back("-w");
    if(document_mode) params.push_back("-d");
    if(interpolate_rggb) params.push_back("-f");

    char ss[32];

    if(quick)
    {
        if(quick == 1)
            quick = 0;

        sprintf(ss, "%d", quick);
        params.push_back("-q");
        params.push_back(ss);
    }

    if(threshold != 99)
    {
        sprintf(ss, "%d", threshold);
        params.push_back("-n");
        params.push_back(ss);
    }

    if(black >= 0)
    {
        sprintf(ss, "%d", black);
        params.push_back("-k");
        params.push_back(ss);
    }

    sprintf(ss, "%d", different);
    params.push_back("-s");
    params.push_back(ss);

    sprintf(ss, "%.1f", brightness);
    params.push_back("-b");
    params.push_back(ss);

    sprintf(ss, "%d", highlights);
    params.push_back("-H");
    params.push_back(ss);

    if(flipping)
    {
        if(flipping == 1)
            flipping = 0; // 0, 3, 5, 6 are accepted

        sprintf(ss, "%d", flipping);
        params.push_back("-t");
        params.push_back(ss);
    }

#ifndef NO_LCMS
    if(icc_cam)
    {
        params.push_back("-p");
        params.push_back("embed");
    }
    else if(icc_file.length())
    {
        params.push_back("-p");
        params.push_back(icc_file);
    }
#endif

    const s32 argc = 5 + params.size();

    const char *argv[argc];
    argv[0] = KLDCRAW;

    for(int i = 1;i < argc-4;i++)
        argv[i] = params[i-1].c_str();

    argv[argc-4] = "-y";
    argv[argc-3] = tmp.c_str();
    argv[argc-2] = file.c_str();
    argv[argc-1] = (char *)0;

    for(int i = 0;i < argc;i++)
        printf("CAMERA %s\n", argv[i]);

    if(!fork())
    {
        execvp(KLDCRAW, (char *const *)argv);
        exit(1);
    }

    ::wait(&status); // TODO check for errors

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#elif defined CODEC_DJVU

    fmt_settings::iterator it = m_settings.find("scaledown");

    // get aspect
    s32 aspect = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    1 : (*it).second.iVal;

    // correct
    if(aspect < 1 || aspect > 12)
        aspect = 2;

    it = m_settings.find("page");

    // get page number
    s32 ipage = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    1 : (*it).second.iVal;

    // correct
    if(ipage < 0 || ipage > 1000)
        ipage = 1;

    int status;

    s8 subsample[20];
    s8 pagesp[20];

    snprintf(subsample, 20, "-subsample=%d", aspect);
    snprintf(pagesp,    20, "-page=%d", ipage);

    if(!fork())
    {
        execlp(DJVU, DJVU, "-format=ppm", subsample, pagesp, file.c_str(), tmp.c_str(), (char *)0);
        exit(1);
    }

    ::wait(&status);

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#elif defined CODEC_DXF

    std::string tmmp = tmp + ".ppm";
    printf("TMP: %s\n", tmmp.c_str());
    fmt_settings::iterator it = m_settings.find("width");

    // get aspect
    s32 width = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    0 : (*it).second.iVal;

    // correct
    if(width < 0 || width > 10000)
        width = 0;

    it = m_settings.find("height");

    // get page number
    s32 height = (it == m_settings.end() || (*it).second.type != settings_value::v_int) ?
                    0 : (*it).second.iVal;

    // correct
    if(height < 0 || height > 10000)
        height = 0;

    s32 status;

    s8 swidth[20], sheight[20];

    const int argc = 8;
    const char *argv[argc];
    const char *x = "-x", *y = "-y";

    int i = 3;
    argv[0] = VEC2WEB;
    argv[1] = file.c_str();
    argv[2] = tmmp.c_str();

    if(width)
    {
        snprintf(swidth,  20, "%d", width);
        argv[i++] = x;
        argv[i++] = swidth;
    }

    if(height)
    {
        snprintf(sheight,  20, "%d", height);
        argv[i++] = y;
        argv[i++] = sheight;
    }

    argv[i] = (char *)0;

    if(!fork())
    {
        execvp(VEC2WEB, (char* const*)argv);
        exit(1);
    }

    ::wait(&status);

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmmp.c_str(), "rb");

#elif defined CODEC_XCF

    const s32 argc = 9;
    int status;

    fmt_settings::iterator it = m_settings.find("background");

    // background for transparent images
    std::string bkgr = (it == m_settings.end() || (*it).second.type != settings_value::v_string) ?
                    "#ffffff" : (*it).second.sVal;

    it = m_settings.find("autocrop");

    // autocrop ?
    bool autocrop = (it == m_settings.end() || (*it).second.type != settings_value::v_bool) ?
                    false : (*it).second.bVal;

    const char *argv[argc];
    argv[0] = KLXCF2PNM;

    std::string bg = "-b";
    bg += bkgr;
    argv[1] = bg.c_str();

    int i = 2;

    if(autocrop)
    {
        argv[i++] = "-C";
    }

    argv[i++] = "-T";
    argv[i++] = "-c";
    argv[i++] = "-o";
    argv[i++] = tmp.c_str();
    argv[i++] = file.c_str();
    argv[i++] = (char *)0;

    if(!fork())
    {
        execvp(KLXCF2PNM, (char *const *)argv);
        exit(1);
    }

    ::wait(&status); // TODO check for errors

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#elif defined CODEC_NETPBM

    int status;

    if(!fork())
    {
        execlp(NETPBM_S, NETPBM_S, file.c_str(), tmp.c_str(), (char *)0);
        exit(1);
    }

    ::wait(&status);

    if(WIFEXITED(status))
        if(WEXITSTATUS(status))
            return SQE_R_BADFILE;
        else;
    else
        return SQE_R_BADFILE;

    fptr = fopen(tmp.c_str(), "rb");

#else

    fptr = fopen(file.c_str(), "rb");

#endif

    if(!fptr)
	return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    s8		str[256];
    s32		w, h;
    u32		maxcolor;

    if(!sq_fgets(str, 255, fptr)) return SQE_R_BADFILE;

    pnm = str[1] - 48;

    if(pnm < 1 || pnm > 6)
	return SQE_R_BADFILE;

    while(true)
    {
	if(!sq_fgets(str, 255, fptr)) return SQE_R_BADFILE;

        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);

    image.w = w;
    image.h = h;

    switch(pnm)
    {
	case 1:
	case 4:
	    image.bpp = 1;
	break;

	case 2:
	case 5:
	    image.bpp = 8;
	break;

	case 3:
	case 6:
	    image.bpp = 8;
	break;
    }

    if(pnm != 4 && pnm != 1)
    {
	fscanf(fptr, "%d", &maxcolor);

	if(sq_ferror(fptr)) return SQE_R_BADFILE;

	if((pnm == 5 || pnm == 6) && maxcolor > 255)
	    return SQE_R_BADFILE;

	if(pnm == 2 || pnm == 3)
	{
	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	else
	{
	    u8 dummy;
	    if(!sq_fgetc(fptr, &dummy)) return SQE_R_BADFILE;
	}

	if(maxcolor <= 9)
	    strcpy(format, "%1d");
	else if(maxcolor >= 9 && maxcolor <= 99)
	    strcpy(format, "%2d");
	else if(maxcolor > 99 && maxcolor <= 999)
	    strcpy(format, "%3d");
	else if(maxcolor > 999 && maxcolor <= 9999)
	    strcpy(format, "%4d");

	koeff = 255.0 / maxcolor;
    }
    else if(pnm == 1)
    {
	strcpy(format, "%1d");
	koeff = 1.0;
    }
    
//    printf("maxcolor: %d, format: %s, koeff: %.1f\n\n", maxcolor, format, koeff);

    image.compression = "-";
    image.colorspace = ((pnm == 1 || pnm == 4) ? "Monochrome":"Color indexed");

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    RGB		rgb;
    u8	        bt;
    s32		i;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    switch(pnm)
    {
	case 1:
        {
	    s32 d;

	    for(i = 0;i < im->w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		d = (s32)(d * koeff);

		memcpy(scan+i, palmono+d, sizeof(RGB));
    	    }

	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	break;

	case 2:
	{
	    s32 d;

	    for(i = 0;i < im->w;i++)
	    {
		fscanf(fptr, format, &d);
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		d = (s32)(d * koeff);

		memset(scan+i, d, sizeof(RGB));
	    }
	    
	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	}
	break;

	case 3:
    	    for(i = 0;i < im->w;i++)
	    {
		fscanf(fptr, format, (s32*)&rgb.r);
		fscanf(fptr, format, (s32*)&rgb.g);
		fscanf(fptr, format, (s32*)&rgb.b);
		if(sq_ferror(fptr)) return SQE_R_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }

	    if(!skip_flood(fptr))
		return SQE_R_BADFILE;
	break;

	case 6:
	    for(i = 0;i < im->w;i++)
	    {
		if(!sq_fread(&rgb, sizeof(RGB), 1, fptr)) return SQE_R_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
//		(scan+i)->r = rgb.
    	    }
	break;

	case 5:
	{
	    for(i = 0;i < im->w;i++)
	    {
		if(!sq_fread(&bt, 1, 1, fptr)) return SQE_R_BADFILE;

		memset(scan+i, int(bt*koeff), sizeof(RGB));
	    }
	}
	break;
	
	case 4:
	{
	    s32 index;//, remain = im->w % 8;

	    for(i = 0;;)
	    {
		if(!sq_fread(&bt,1,1,fptr)) return SQE_R_BADFILE;

		index = (bt&128)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&64)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&32)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&16)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&8)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&4)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&2)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
		index = (bt&1);
		memcpy(scan+i, palmono+index, 3);i++; if(i >= im->w) break;
	    }
	}
	break;
    }

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

void fmt_codec::read_close()
{
    if(fptr)
        fclose(fptr);

    finfo.meta.clear();
    finfo.image.clear();
}

bool skip_flood(FILE *f)
{
    s32 pos;
    u8 b;

    while(true)
    {
	pos = ftell(f);
	if(!sq_fread(&b, 1, 1, f)) return false;

	if(!isspace(b))
	{
	    if(b == '#')
	    {
		while(true)
		{
		    if(!sq_fgetc(f, &b)) return false;

		    if(b == '\n')
			break;
		}
	    }

	    break;
	}
    }

    fsetpos(f, (fpos_t*)&pos);
    
    return true;
}

void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->passes = 1;
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
    fws << "P6" << endl << writeimage.w << " " << writeimage.h << endl << 255 << endl;

    return fws.good() ? SQE_OK : SQE_W_ERROR;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    for(s32 i = 0;i < writeimage.w;i++)
    {
	if(!fws.writeK(scan+i, sizeof(RGB)))
	    return SQE_W_ERROR;
    }

    return SQE_OK;
}

void fmt_codec::write_close()
{
    fws.close();
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("pnm");
}

#include "fmt_codec_cd_func.h"

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
#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"

#include "fmt_codec_xbm_defs.h"
#include "fmt_codec_xbm.h"

#include "../xpm/codec_xbm.xpm"

/*
 *
 * XBM is a native file format of the X Window System and is used for
 * storing cursor and icon bitmaps that are used in the X GUI.
 * XBMfiles are quite different in that they are actually C language source
 * files that are created to be read by a C compiler rather than a 
 * graphical display program.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.7.0";
    o->name = "X BitMap";
    o->filter = "*.xbm ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-xbm";
    o->pixmap = codec_xbm;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQE_R_NOFILE;
		    
    currentImage = -1;
    
    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    long	_tmp;
    s8	str[256], *ptr;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    if(!skip_comments(fptr))
	return SQE_R_BADFILE;

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQE_R_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQE_R_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQE_R_BADFILE;

    image.w = (long)atoi(ptr+6);

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQE_R_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQE_R_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQE_R_BADFILE;

    image.h = (long)atoi(ptr+7);

    while(sq_fgets(str, sizeof(str)-1, fptr))
    {
        if(sq_ferror(fptr)) return SQE_R_BADFILE;

	if(strncmp(str, "#define ", 8) != 0)
	    break;
    }

    if(str[0] == '\n') if(!sq_fgets(str, sizeof(str)-1, fptr)) return SQE_R_BADFILE;

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	return SQE_R_BADFILE;
	
    if((strstr(str, "unsigned") && (strstr(str, "char"))) || strstr(str, "char"))
	version = 11;
    else if(strstr(str, "short"))
	version = 10;
    else
	return SQE_R_NOTSUPPORTED;

    _tmp = lscan = image.w;

    image.bpp = 1;

    lscan /= 8;
    lscan = lscan + ((_tmp%8)?1:0);

    memset(pal, 255, sizeof(RGB));
    memset(pal+1, 0, sizeof(RGB));

    image.compression = "-";
    image.colorspace = fmt_utils::colorSpaceByBpp(1);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    s8 index,  c;
    s32 counter = 0, remain = (im->w <= 8) ? im->w : (im->w % 8), j;
    u32 bt;

    for(j = 0;j < lscan;j++)
    {
	fscanf(fptr, "%x%c", &bt, &c);
	if(sq_ferror(fptr)) return SQE_R_BADFILE;

	// @todo make faster
	if(j==lscan-1 && (remain-0)<=0 && remain)break; index = (bt & 1);           memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-1)<=0 && remain)break; index = (bt & 2) ? 1 : 0;   memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-2)<=0 && remain)break; index = (bt & 4) ? 1 : 0;   memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-3)<=0 && remain)break; index = (bt & 8) ? 1 : 0;   memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-4)<=0 && remain)break; index = (bt & 16) ? 1 : 0;  memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-5)<=0 && remain)break; index = (bt & 32) ? 1 : 0;  memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-6)<=0 && remain)break; index = (bt & 64) ? 1 : 0;  memcpy(scan+counter, pal+(s32)index, 3); counter++;
	if(j==lscan-1 && (remain-7)<=0 && remain)break; index = (bt & 128) ? 1 : 0; memcpy(scan+counter, pal+(s32)index, 3); counter++;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    fclose(fptr);

    finfo.meta.clear();
    finfo.image.clear();
}

/*  skip a single line C-like comment  */
bool skip_comments(FILE *fp)
{
    s8 str[513];
    s32 pos;

    do
    {
        pos = ftell(fp);
        if(!sq_fgets(str, 512, fp)) return false;

        if(!strstr(str, "/*"))
            break;
    }while(true);

    fsetpos(fp, (fpos_t*)&pos);
    
    return true;
}

#include "fmt_codec_cd_func.h"

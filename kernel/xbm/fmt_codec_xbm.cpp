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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_xbm_defs.h"
#include "fmt_codec_xbm.h"

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

std::string fmt_codec::fmt_version()
{
    return std::string("0.7.0");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X BitMap");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.xbm ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,202,202,202,70,70,70,222,222,222,178,178,178,174,174,174,254,254,254,254,254,202,78,78,78,242,242,242,2,2,202,40,206,154,3,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,98,73,68,65,84,120,218,99,232,0,1,7,6,6,6,69,65,65,65,33,83,16,35,45,45,77,104,178,3,132,33,209,161,0,97,8,10,54,48,48,52,41,65,0,131,214,42,48,88,196,208,190,124,121,249,170,170,85,69,12,234,203,151,175,170,42,95,94,196,208,85,5,18,169,130,138,128,164,224,106,186,96,186,148,96,230,128,44,21,20,4,90,193,4,230,55,48,0,0,134,19,45,152,122,245,14,26,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(std::string file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQE_R_NOFILE;
		    
    currentImage = -1;
    
    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    long	tmp;
    s8	str[256], *ptr;

    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;
    
    if(!skip_comments(fptr))
	return SQE_R_BADFILE;

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQE_R_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQE_R_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQE_R_BADFILE;

    finfo.image[currentImage].w = (long)atoi(ptr+6);

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQE_R_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQE_R_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQE_R_BADFILE;

    finfo.image[currentImage].h = (long)atoi(ptr+7);

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

    tmp = lscan = finfo.image[currentImage].w;

    finfo.image[currentImage].bpp = 1;

    lscan /= 8;
    lscan = lscan + ((tmp%8)?1:0);

    memset(pal, 255, sizeof(RGB));
    memset(pal+1, 0, sizeof(RGB));

    finfo.images++;
    finfo.image[currentImage].compression = "-";
    finfo.image[currentImage].colorspace = "Monochrome";

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
//    printf("%ld ",lscan);return 1;

    s8 index,  c;
    s32 counter = 0, remain=((finfo.image[currentImage].w)<=8)?(finfo.image[currentImage].w):((finfo.image[currentImage].w)%8), j;
    u32 bt;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

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

void fmt_codec::fmt_read_close()
{
    fclose(fptr);

    finfo.meta.clear();
    finfo.image.clear();
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

bool fmt_codec::fmt_readable() const
{
    return true;
}

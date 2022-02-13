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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_xbm_defs.h"
#include "fmt_codec_xbm.h"

#include "error.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,0,0,255,255,255,0,4,4,4,176,201,253,137,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,79,73,68,65,84,120,218,99,96,96,72,3,1,6,32,72,20,20,20,20,3,51,148,148,148,196,210,160,12,160,144,49,20,48,152,184,128,129,51,131,169,139,171,107,136,171,171,51,131,73,8,144,114,13,5,50,92,93,144,69,92,145,213,192,116,193,205,129,152,172,36,6,181,20,108,43,216,25,9,0,96,116,27,33,72,26,231,24,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    
    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    long	tmp;
    s8	str[256], *ptr;

    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;
    
    if(!skip_comments(fptr))
	return SQERR_BADFILE;

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQERR_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQERR_BADFILE;

    finfo.image[currentImage].w = (long)atoi(ptr+6);

    if(!sq_fgets(str, sizeof(str)-1, fptr))
	return SQERR_BADFILE;

    if(strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQERR_BADFILE;

    finfo.image[currentImage].h = (long)atoi(ptr+7);

    while(sq_fgets(str, sizeof(str)-1, fptr))
    {
        if(sq_ferror(fptr)) return SQERR_BADFILE;

	if(strncmp(str, "#define ", 8) != 0)
	    break;
    }

    if(str[0] == '\n') if(!sq_fgets(str, sizeof(str)-1, fptr)) return SQERR_BADFILE;

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	return SQERR_BADFILE;
	
    if((strstr(str, "unsigned") && (strstr(str, "char"))) || strstr(str, "char"))
	version = 11;
    else if(strstr(str, "short"))
	version = 10;
    else
	return SQERR_NOTSUPPORTED;

    tmp = lscan = finfo.image[currentImage].w;

    finfo.image[currentImage].bpp = 1;

    lscan /= 8;
    lscan = lscan + ((tmp%8)?1:0);

    memset(pal, 255, sizeof(RGB));
    memset(pal+1, 0, sizeof(RGB));

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Monochrome" << "\n"
        << "-" << "\n"
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
//    printf("%ld ",lscan);return 1;

    s8 index,  c;
    s32 counter = 0, remain=((finfo.image[currentImage].w)<=8)?(finfo.image[currentImage].w):((finfo.image[currentImage].w)%8), j;
    u32 bt;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    for(j = 0;j < lscan;j++)
    {
	fscanf(fptr, "%x%c", &bt, &c);
	if(sq_ferror(fptr)) return SQERR_BADFILE;

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

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    FILE 	*m_fptr;
    s32 	w, h, bpp;
    long	m_lscan;
    s32		m_version;
    RGB		m_pal[2];
    s32 	m_bytes;
    jmp_buf	jmp;

    m_fptr = fopen(file.c_str(), "rb");
    
    if(!m_fptr)
        return SQERR_NOFILE;
			
    long	tmp;
    s8	str[256], *ptr;

    if(setjmp(jmp))
    {
        fclose(m_fptr);
        return SQERR_BADFILE;
    }

    if(!skip_comments(m_fptr))
	longjmp(jmp, 1);

    if(!sq_fgets(str, sizeof(str)-1, m_fptr))
	longjmp(jmp, 1);

    if(strncmp(str, "#define ", 8) != 0)
	longjmp(jmp, 1);

    if((ptr = strstr(str, "_width ")) == NULL)
	longjmp(jmp, 1);

    w = atoi(ptr+6);

    if(!sq_fgets(str, sizeof(str)-1, m_fptr))
	longjmp(jmp, 1);

    if(strncmp(str, "#define ", 8) != 0)
	longjmp(jmp, 1);

    if((ptr = strstr(str, "_height ")) == NULL)
	longjmp(jmp, 1);

    h = (long)atoi(ptr+7);

    while(sq_fgets(str, sizeof(str)-1, m_fptr))
	if(strncmp(str, "#define ", 8) != 0)
	    break;

    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

    if(str[0] == '\n') if(!sq_fgets(str, sizeof(str)-1, m_fptr)) longjmp(jmp, 1);

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	longjmp(jmp, 1);
	
    if((strstr(str, "unsigned") && (strstr(str, "char"))) || strstr(str, "char"))
	m_version = 11;
    else if(strstr(str, "short"))
	m_version = 10;
    else
	return SQERR_NOTSUPPORTED;

    tmp = m_lscan = w;
    bpp = 1;

    m_lscan /= 8;
    m_lscan = m_lscan + ((tmp%8)?1:0);

    memset(m_pal, 255, sizeof(RGB));
    memset(m_pal+1, 0, sizeof(RGB));

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Monochrome" << "\n"
        << "-" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }
									    
    memset(*image, 255, m_bytes);

    for(s32 h2 = 0;h2 < h;h2++)
    {
	s8 index,  c;
	s32 counter = 0, remain=(w<=8) ? w : (w%8), j;
	u32 bt;
        RGBA 	*scan = *image + h2 * w;

	for(j = 0;j < m_lscan;j++)
	{
	    fscanf(m_fptr, "%x%c", &bt, &c);
	    if(sq_ferror(m_fptr)) longjmp(jmp, 1);

	    // @todo make faster
	    if(j==m_lscan-1 && (remain-0)<=0 && remain)break; index = (bt & 1);        memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
    	    if(j==m_lscan-1 && (remain-1)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-2)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-3)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-4)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-5)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-6)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-7)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+counter, m_pal+(s32)index, 3); counter++;
	}
    }										

    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &)
{
    return SQERR_OK;
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

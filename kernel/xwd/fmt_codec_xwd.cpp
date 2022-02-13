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
#include "fmt_codec_xwd_defs.h"
#include "fmt_codec_xwd.h"

#include "error.h"

#define SQ_HAVE_FMT_UTILS
#include "fmt_utils.h"

/*
 *
 * The XWD (X Window Dump) format is used specifically to store screen
 * dumps
 *
 * Created by the X Window System. Under X11, screen dumps are created by the
 * xwd client. Using xwd, the window or background is selected to
 * dump and an XWD file is produced containing an image of the window. If you
 * issue the following command:
 * 
 *   % xwd -root &gt; output.xwd
 *
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.4.3");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X Window Dump");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.xwd ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,137,12,83,4,4,4,204,223,87,180,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,1,98,75,71,68,0,136,5,29,72,0,0,0,9,112,72,89,115,0,0,11,17,0,0,11,17,1,127,100,95,145,0,0,0,7,116,73,77,69,7,213,1,30,19,36,17,134,185,242,93,0,0,0,81,73,68,65,84,120,156,69,142,65,17,192,48,8,4,207,2,15,12,80,20,164,81,64,17,208,62,226,223,74,40,97,146,125,237,48,112,7,128,241,131,224,35,34,78,17,17,30,37,49,186,11,116,79,30,52,51,55,147,148,192,47,116,45,105,230,170,71,98,103,95,237,156,149,44,92,165,217,154,111,188,19,140,238,23,253,181,135,193,82,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;
    pal = 0;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    XWDFileHeader xfh;

    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    XWDColor	color;
    s8 	str[256];
    s32		i, ncolors;

    finfo.image[currentImage].passes = 1;

    if(!frs.readK(&xfh, sizeof(XWDFileHeader))) return SQERR_BADFILE;

    xfh.file_version = fmt_utils::konvertLong(xfh.file_version);

    if(xfh.file_version != XWD_FILE_VERSION)
	return SQERR_BADFILE;

    frs.get(str, 255, '\n');
    
    frs.clear();

    frs.seekg(fmt_utils::konvertLong(xfh.header_size), ios::beg);

    pal_entr = ncolors = fmt_utils::konvertLong(xfh.ncolors);

    pal = new RGB [ncolors];

    if(!pal)
	return SQERR_NOMEMORY;

    for(i = 0;i < ncolors;i++)
    {
	if(!frs.readK(&color, sizeof(XWDColor))) return SQERR_BADFILE;

	pal[i].r = (s8)fmt_utils::konvertWord(color.red);
	pal[i].g = (s8)fmt_utils::konvertWord(color.green);
	pal[i].b = (s8)fmt_utils::konvertWord(color.blue);
    }

    finfo.image[currentImage].w = fmt_utils::konvertLong(xfh.pixmap_width);
    finfo.image[currentImage].h = fmt_utils::konvertLong(xfh.pixmap_height);
    finfo.image[currentImage].bpp = fmt_utils::konvertLong(xfh.bits_per_pixel);//fmt_utils::konvertLong(xfh.pixmap_depth);

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
            
    finfo.images++;

    finfo.meta.push_back(fmt_metaentry());

    finfo.meta[0].group = "XWD Window Name";
    finfo.meta[0].data = str;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "RGB" << "\n"
        << "-" << "\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

    filler = fmt_utils::konvertLong(xfh.bytes_per_line) - finfo.image[currentImage].w * finfo.image[currentImage].bpp / 8;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 	i;
    RGBA	rgba;
    RGB		rgb;
    u8 d;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
        case 24:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
    	    {
    		if(!frs.readK(&rgb, sizeof(RGB))) return SQERR_BADFILE;

		memcpy(scan+i, &rgb, sizeof(RGB));
	    }
	    
	    for(s32 s = 0;s < filler;s++)
		if(!frs.readK(&d, 1))
		    return SQERR_BADFILE;
	break;

        case 32:
	    for(i = 0;i < finfo.image[currentImage].w;i++)
    	    {
    		if(!frs.readK(&rgba, sizeof(RGBA))) return SQERR_BADFILE;

		scan[i].r = rgba.b;
		scan[i].g = rgba.g;
		scan[i].b = rgba.r;
	    }
	    
	    for(s32 s = 0;s < filler;s++)
		if(!frs.readK(&d, 1))
		    return SQERR_BADFILE;
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    XWDFileHeader m_xfh;

    s32 	w, h, bpp, m_pal_entr, m_ncolors;
    s32 	m_bytes;
    jmp_buf	jmp;
    RGB		*m_pal;
    ifstreamK	m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    m_pal = 0;

    if(setjmp(jmp))
    {
        m_frs.close();
	
	if(m_pal)
	    delete [] m_pal;
	
        return SQERR_BADFILE;
    }

    XWDColor	color;
    s8 	str[256];
    s32		i;
    s32 	m_filler;

    if(!m_frs.readK(&m_xfh, sizeof(XWDFileHeader))) longjmp(jmp, 1);

    m_xfh.file_version = fmt_utils::konvertLong(m_xfh.file_version);

    if(m_xfh.file_version != XWD_FILE_VERSION)
	return SQERR_BADFILE;

    if(!m_frs.get(str, 255, '\n')) longjmp(jmp, 1);

    m_frs.seekg(fmt_utils::konvertLong(m_xfh.header_size), ios::beg);

    m_pal_entr = m_ncolors = fmt_utils::konvertLong(m_xfh.ncolors);
    
    m_pal = new RGB [m_ncolors];

    if(!m_pal)
    {
	longjmp(jmp, 1);
    }

    for(i = 0;i < m_ncolors;i++)
    {
	if(!m_frs.readK(&color, sizeof(XWDColor))) longjmp(jmp, 1);

	m_pal[i].r = (s8)fmt_utils::konvertWord(color.red);
	m_pal[i].g = (s8)fmt_utils::konvertWord(color.green);
	m_pal[i].b = (s8)fmt_utils::konvertWord(color.blue);
    }

    w = fmt_utils::konvertLong(m_xfh.pixmap_width);
    h = fmt_utils::konvertLong(m_xfh.pixmap_height);
    bpp = fmt_utils::konvertLong(m_xfh.bits_per_pixel);

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "RGB" << "\n"
        << "-" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    m_filler = fmt_utils::konvertLong(m_xfh.bytes_per_line) - w * bpp / 8;
				    
    *image = (RGBA*)realloc(*image, m_bytes);
					
    if(!*image)
    {
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);
    
    u8 d;
    
    for(s32 h2 = 0;h2 < h;h2++)
    {
	RGBA	rgba;
	RGB	rgb;
        RGBA 	*scan = *image + h2 * w;

/*
	for(s32 s = 0;s < w;s++)
	{
	    fread(&rgba, sizeof(RGBA), 1, fptr);

	    scan[s].r = rgba.b;
	    scan[s].g = rgba.g;
	    scan[s].b = rgba.r;
	}*/
        switch(bpp)
	{
    	    case 24:
		for(s32 s = 0;s < w;s++)
    		{
    		    if(!m_frs.readK(&rgb, sizeof(RGB))) longjmp(jmp, 1);

		    memcpy(scan+s, &rgb, sizeof(RGB));
		}

		for(s32 s = 0;s < m_filler;s++)
		    if(!m_frs.readK(&d, 1))
			longjmp(jmp, 1);
	    break;

    	    case 32:
		for(s32 s = 0;s < w;s++)
    		{
    		    if(!m_frs.readK(&rgba, sizeof(RGBA))) longjmp(jmp, 1);

		    scan[s].r = rgba.b;
		    scan[s].g = rgba.g;
		    scan[s].b = rgba.r;
		}

		for(s32 s = 0;s < m_filler;s++)
		    if(!m_frs.readK(&d, 1))
			longjmp(jmp, 1);
	    break;
	}

    }

    m_frs.close();
    
    if(m_pal)
	delete [] m_pal;

    return SQERR_OK;
}
			    
void fmt_codec::fmt_close()
{
    frs.close();

    if(pal)
	delete [] pal;

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

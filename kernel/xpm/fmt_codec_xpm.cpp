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
#include <map>

#include "fmt_types.h"
#include "fmt_codec_xpm.h"
#include "fmt_codec_xpm_defs.h"

#include "error.h"
#include "xpm_utils.h"

/*
 * 
 * The XPM (X PixMap) format is the current de facto standard for storing X Window
 * pixmap data to a disk file. This format is supported by many image editors,
 * graphics window managers, and image file converters. 
 *
 *
 * XPM is capable of storing black-and-white, gray-scale, or color image data.
 * Hotspot information for cursor bitmaps may also be stored. Although small
 * collections of data, such as icons, are typically associated with XPM files,
 * there is no limit to the size of an image or the number of colors that may be
 * stored in an XPM file.
 *
 */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    fillmap();
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.6.2");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("X11 Pixmap");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.xpm ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("/. XPM ./\n");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,95,95,95,4,4,4,61,221,162,37,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,65,21,192,48,8,67,99,129,3,6,120,85,176,85,1,69,192,118,168,127,43,163,41,93,46,228,125,32,0,96,46,33,245,138,136,210,152,153,206,50,137,238,18,122,80,3,87,184,55,247,129,222,178,184,165,241,68,135,176,181,102,130,228,108,253,57,59,217,180,142,242,42,223,120,62,156,44,24,95,168,109,132,217,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    file.clear();

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    finfo.image.push_back(fmt_image());

    s32		i;
    s8	str[256];

    s32 ret;

    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }
    if(!frs.getS(str, 256)) return SQERR_BADFILE;
    if(strncmp(str, "static", 6) != 0) return SQERR_BADFILE;
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }
    if(!frs.getS(str, 256)) return SQERR_BADFILE;
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }

    sscanf(str, "\"%d %d %d %d", &finfo.image[currentImage].w, &finfo.image[currentImage].h, &numcolors, (int*)&cpp);
//    printf("%d %d %d %d\n\n",finfo.image[currentImage].w,finfo.image[currentImage].h,numcolors,cpp);

    if(!numcolors)
	return SQERR_BADFILE;

    s8 name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < numcolors;i++)
    {
	if(!frs.getS(str, 256)) return SQERR_BADFILE;

	if(*str != '\"')
	{
	    cerr << "libSQ_read_xpm: file corrupted" << endl;
	    numcolors = i;
	    break;
	}

	strcpy(name, "");

	found = str;
	found++;

	strncpy(name, found, cpp);
	name[cpp] = 0;

	sscanf(found+cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

//	if(!i)printf("%s\n",color);

//	memcpy(Xmap[i].name, name, cpp);
//	Xmap[i].name[cpp] = 0;
//	Xmap[i].rgba = hex2rgb(color);
//	RGBA r = hex2rgb(color);
//	printf("%s <=> %d,%d,%d,%d\n", name, r.r, r.g, r.b, r.a);
	file[name] = hex2rgb(color);
    }

    if(!numcolors)
	return SQERR_BADFILE;

//    cout << "111111111111";
    while(true) { ret = skip_comments(frs); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }
//    cout << "333333333333333\n";

/*
    QuickSort(Xmap, 0, numcolors-1);

    for(i = 0;i < numcolors;i++)
    {
	printf("\"%s\"  %d %d %d %d\n",Xmap[i].name,Xmap[i].rgba.r,Xmap[i].rgba.g,Xmap[i].rgba.b,Xmap[i].rgba.a);
    }
*/
    finfo.image[currentImage].bpp = 24;
    finfo.image[currentImage].hasalpha = true;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
            
    finfo.images++;
    finfo.image[currentImage].passes = 1;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "RGBA" << "\n"
        << "-" << "\n"
        << bytes;
	
//    cout << s.str() << "\n";

    finfo.image[currentImage].dump = s.str();
					
    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    const s32	bpl = finfo.image[currentImage].w * (cpp+2);
    s32		i, j;
    s8 	line[bpl], key[KEY_LENGTH];
    
//    printf("bpl: %d\n", bpl);

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));
/*    
    static s32 ee = 0;
    printf("line %d\n", ee);
    ee++;
*/
    switch(finfo.image[currentImage].bpp)
    {
	case 24:
	{
	    RGBA  rgba;
	    bool f;

	    i = j = 0;
	    if(!frs.getS(line, sizeof(line))) return SQERR_BADFILE;

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < finfo.image[currentImage].w;j++)
	    {
		strncpy(key, line+i, cpp);
		i += cpp;

		//trgba = BinSearch(Xmap, 0, numcolors-1, key);
		std::map<std::string, RGBA>::const_iterator it = file.find(key);

		f = (it != file.end());

		if(!f)
		{
		    cerr << "XPM decoder: WARNING: color \"" << key << "\" not found, assuming transparent instead" << endl;
		    memset(&rgba, 0, sizeof(RGBA));
		}
		else
		    rgba = (*it).second;

		memcpy(scan+j, &rgba, sizeof(RGBA));
	    }
	}
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 	w, h, bpp;
    s32		m_numcolors;
    s32		m_cpp;
    s32 	m_bytes;
    jmp_buf	jmp;
    ifstreamK	m_frs;
    std::map<std::string, RGBA> m_Xmap;

    m_frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        m_frs.close();
	m_Xmap.clear();

        return SQERR_BADFILE;
    }
			    
    s32		i;
    s8		str[256];
    s32 	ret;

    while(true) { ret = skip_comments(m_frs); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }
    if(!m_frs.getS(str, 256)) longjmp(jmp, 1);
    if(strncmp(str, "static", 6) != 0) longjmp(jmp, 1);
    while(true) { ret = skip_comments(m_frs); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }
    if(!m_frs.getS(str, 256)) longjmp(jmp, 1);
    while(true) { ret = skip_comments(m_frs); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }

    sscanf(str, "\"%d %d %d %d", &w, &h, &m_numcolors, (int*)&m_cpp);
//    printf("%d %d %d %d\n\n",finfo.w,finfo.h,m_numcolors,m_cpp);


    s8 name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < m_numcolors;i++)
    {
	if(!m_frs.getS(str, 256)) longjmp(jmp, 1);

	if(*str != '\"')
	{
	    cerr << "libSQ_read_xpm: file corrupted." << endl;
	    m_numcolors = i;
	    break;
	}

	strcpy(name, "");

	found = str;
	found++;

	strncpy(name, found, m_cpp);
	name[m_cpp] = 0;

	sscanf(found+m_cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

//	memcpy(m_Xmap[i].name, name, m_cpp);
//	m_Xmap[i].name[m_cpp] = 0;
//	m_Xmap[i].rgba = hex2rgb(color);
	m_Xmap[name] = hex2rgb(color);
    }

    if(!m_numcolors)
	longjmp(jmp, 1);

    while(true) { ret = skip_comments(m_frs); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }
/*
    QuickSort(m_Xmap, 0, m_numcolors-1);

    for(i = 0;i < m_numcolors;i++)
    {
	printf("%d %d %d %d\n", m_Xmap[i].rgba.r,m_Xmap[i].rgba.g,m_Xmap[i].rgba.b,m_Xmap[i].rgba.a);
    }
*/
    bpp = 24;

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "RGBA" << "\n"
        << "-" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */
    
    for(s32 h2 = 0;h2 < h;h2++)
    {
        RGBA *scan = *image + h2 * w;

    const s32	bpl = w * (m_cpp+2);
    s32		i, j;
    s8 	line[bpl], key[KEY_LENGTH];

    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));

    switch(bpp)
    {
	case 24:
	{
	    RGBA rgba;
	    bool f;

	    i = j = 0;
	    if(!m_frs.getS(line, sizeof(line))) longjmp(jmp, 1);

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < w;j++)
	    {
		strncpy(key, line+i, m_cpp);
		i += m_cpp;
		
//		printf("\"%s\" %d  \"%s\"\n",line,i,key);

		//trgba = BinSearch(m_Xmap, 0, m_numcolors-1, key);
		std::map<std::string, RGBA>::const_iterator it = m_Xmap.find(key);

		f = (it != m_Xmap.end());

		if(!f)
		{
		    cerr << "XPM decoder: WARNING: color \"" << key << "\" not found, assuming transparent instead" << endl;
		    memset(&rgba, 0, sizeof(RGBA));
		}
		else
		    rgba = (*it).second;

		memcpy(scan+j, &rgba, sizeof(RGBA));
	    }
	}
	break;
    }
    }

    m_frs.close();
    m_Xmap.clear();
    
    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    frs.close();

    finfo.meta.clear();
    finfo.image.clear();

    file.clear();
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

void fmt_codec::fillmap()
{
    s8 	name[80];
    s32 	r, g, b, a;

    ifstream rgb_fstream;
    
    rgb_fstream.open("/usr/lib/ksquirrel-libs/rgbmap", ios::in);

    if(!rgb_fstream.good())
    {
	cerr << "libSQ_read_xpm: rgbmap not found" << endl;
	return;
    }

    while(rgb_fstream.good())
    {
	rgb_fstream >> name >> r >> g >> b >> a;

	named[name] = RGBA(r,g,b,a);
    }
    
    rgb_fstream.close();
}

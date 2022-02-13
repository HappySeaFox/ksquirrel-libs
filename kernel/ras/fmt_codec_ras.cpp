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
#include "fmt_codec_ras_defs.h"
#include "fmt_codec_ras.h"

#include "error.h"

/*
 *
 * The Sun Raster image file format is the native bitmap format of the Sun
 * Microsystems UNIX platforms using the SunOS operating system. This format is
 * capable of storing black-and-white, gray-scale, and color bitmapped data of any
 * pixel depth. The use of color maps and a simple Run-Length data compression
 * are also supported. Typically, most images found on a SunOS system are Sun
 * Raster images, and this format is supported by most UNIX imaging applications.
 *
 */

bool fmt_readdata(ifstreamK &, u8 *, u32, bool);

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.6.3");
}
    
std::string fmt_codec::fmt_quickinfo()
{
    return std::string("SUN Raster");
}
	
std::string fmt_codec::fmt_filter()
{
    return std::string("*.ras ");
}
	    
std::string fmt_codec::fmt_mime()
{
    return std::string("\x0059\x00A6\x006A\x0095");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,102,255,204,4,4,4,193,122,37,156,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,73,73,68,65,84,120,218,99,96,96,8,5,1,6,32,8,20,20,20,20,5,51,148,148,148,68,67,161,12,160,144,49,20,48,152,184,128,129,51,131,137,177,139,177,137,177,9,144,1,6,24,34,46,48,6,88,4,166,11,110,14,196,100,37,81,168,165,96,91,193,206,8,0,0,119,14,23,196,139,226,159,35,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQERR_NOFILE;

    currentImage = -1;
    rle = false;
    isRGB = false;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;
	    
    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    if(!frs.be_getlong(&rfh.ras_magic)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_width)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_height)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_depth)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_length)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_type)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_maptype)) return SQERR_BADFILE;
    if(!frs.be_getlong(&rfh.ras_maplength)) return SQERR_BADFILE;

    if(rfh.ras_magic != RAS_MAGIC) return SQERR_BADFILE;

    if(rfh.ras_type != RAS_OLD && rfh.ras_type != RAS_STANDARD && rfh.ras_type != RAS_BYTE_ENCODED && rfh.ras_type != RAS_RGB &&
	    rfh.ras_type != RAS_TIFF && rfh.ras_type != RAS_IFF &&  rfh.ras_type != RAS_EXPERIMENTAL)
	return SQERR_BADFILE;
    else if(rfh.ras_type == RAS_EXPERIMENTAL)
	return SQERR_NOTSUPPORTED;

    finfo.image[currentImage].w = rfh.ras_width;
    finfo.image[currentImage].h = rfh.ras_height;
    finfo.image[currentImage].bpp = rfh.ras_depth;

    switch(rfh.ras_maptype)
    {
	case RMT_NONE :
	{
		if (rfh.ras_depth < 24)
		{
		    s32 numcolors = 1 << rfh.ras_depth, i;

		    for (i = 0; i < numcolors; i++)
		    {
			pal[i].r = (255 * i) / (numcolors - 1);
			pal[i].g = (255 * i) / (numcolors - 1);
			pal[i].b = (255 * i) / (numcolors - 1);
		    }
		}

	break;
	}

	case RMT_EQUAL_RGB:
	{
		s8 *g, *b;

		s32 numcolors = 1 << rfh.ras_depth;

		s8 r[3 * numcolors];

		g = r + numcolors;
		b = g + numcolors;

		if(!frs.readK(r, 3 * numcolors)) return SQERR_BADFILE;

		for(s32 i = 0; i < numcolors; i++)
		{
			pal[i].r = r[i];
			pal[i].g = g[i];
			pal[i].b = b[i];
		}
	break;
	}

	case RMT_RAW:
	{
		s8 colormap[rfh.ras_maplength];

		if(!frs.readK(colormap, rfh.ras_maplength)) return SQERR_BADFILE;
	break;
	}
    }

    switch(rfh.ras_type)
    {
	case RAS_OLD:
	case RAS_STANDARD:
	case RAS_TIFF:
	case RAS_IFF:
	break;

	case RAS_BYTE_ENCODED:
	    rle = true;
	break;

	case RAS_RGB:
	    isRGB = true;
	break;
    }
    
    if(rfh.ras_depth == 1)
	linelength = (short)((rfh.ras_width / 8) + (rfh.ras_width % 8 ? 1 : 0));
    else
	linelength = (short)rfh.ras_width;

    fill = (linelength % 2) ? 1 : 0;

    buf = new u8 [rfh.ras_width * sizeof(RGB)];

    if(!buf)
	return SQERR_NOMEMORY;

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);
    
    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "RGB" << "\n"
        << ((isRGB) ? "-":"RLE") << "\n"
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
    u32 i;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    switch(finfo.image[currentImage].bpp)
    {
    	case 1:
	break;

	case 8:
		if(!fmt_readdata(frs, buf, linelength, rle))
		    return SQERR_BADFILE;

		for(i = 0;i < rfh.ras_width;i++)
		    memcpy(scan+i, &pal[i], sizeof(RGB));

		if(fill)
		{
		    if(!fmt_readdata(frs, &fillchar, fill, rle))
			return SQERR_BADFILE;
		}
    	break;

	case 24:
	{
	    u8 *b = buf;
	    
	    if(!fmt_readdata(frs, buf, rfh.ras_width * 3, rle))
		return SQERR_BADFILE;

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].r = *b;
    	    	    scan[i].g = *(b+1);
		    scan[i].b = *(b+2);
		    b += 3;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 2);
		    scan[i].g = *(b + 1);
		    scan[i].b = *b;
		    b += 3;
		}
		
	    if(fill)
	    {
	        if(!fmt_readdata(frs, &fillchar, fill, rle))
		    return SQERR_BADFILE;
	    }
	}	
	break;

	case 32:
	{
	    u8 *b = buf;
	    
	    if(!fmt_readdata(frs, buf, rfh.ras_width * 4, rle))
		return SQERR_BADFILE;

	    if(isRGB)
	        for (i = 0; i < rfh.ras_width; i++)
	        {
	    	    scan[i].a = *b;
	    	    scan[i].r = *(b+1);
    	    	    scan[i].g = *(b+2);
		    scan[i].b = *(b+3);
		    b += 4;
		}
	    else
	        for (i = 0; i < rfh.ras_width; i++)
	        {
		    scan[i].r = *(b + 3);
		    scan[i].g = *(b + 2);
		    scan[i].b = *(b + 1);
		    scan[i].a = *b;
		    b += 4;
		}
		
	    if(fill)
	    {
	        if(!fmt_readdata(frs, &fillchar, fill, rle))
		    return SQERR_BADFILE;
	    }

	}
	break;
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    jmp_buf		jmp;
    s32 		w, h, bpp, m_bytes;
    RAS_HEADER 		m_rfh;
    bool 		m_rle, m_isRGB;
    u16 	m_fill;
    u8 	m_fillchar;
    u16 	m_linelen;
    RGB			m_pal[256];
    ifstreamK		m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
	m_frs.close();
	return SQERR_BADFILE;
    }

    if(!m_frs.be_getlong(&m_rfh.ras_magic)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_width)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_height)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_depth)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_length)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_type)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_maptype)) longjmp(jmp, 1);
    if(!m_frs.be_getlong(&m_rfh.ras_maplength)) longjmp(jmp, 1);    

    if(m_rfh.ras_magic != RAS_MAGIC) longjmp(jmp, 1);

    if(m_rfh.ras_type != RAS_OLD && m_rfh.ras_type != RAS_STANDARD && m_rfh.ras_type != RAS_BYTE_ENCODED && m_rfh.ras_type != RAS_RGB &&
	    m_rfh.ras_type != RAS_TIFF && m_rfh.ras_type != RAS_IFF &&  m_rfh.ras_type != RAS_EXPERIMENTAL)
	longjmp(jmp, 1);
    else if(m_rfh.ras_type == RAS_EXPERIMENTAL)
	return SQERR_NOTSUPPORTED;

    w = m_rfh.ras_width;
    h = m_rfh.ras_height;
    bpp = m_rfh.ras_depth;

    switch(m_rfh.ras_maptype)
    {
	case RMT_NONE :
	{
		if (m_rfh.ras_depth < 24)
		{
		    s32 numcolors = 1 << m_rfh.ras_depth, i;

		    for (i = 0; i < numcolors; i++)
		    {
			m_pal[i].r = (255 * i) / (numcolors - 1);
			m_pal[i].g = (255 * i) / (numcolors - 1);
			m_pal[i].b = (255 * i) / (numcolors - 1);
		    }
		}

	break;
	}

	case RMT_EQUAL_RGB:
	{
		s8 *g, *b;

		s32 numcolors = 1 << m_rfh.ras_depth;

		s8 r[3 * numcolors];

		g = r + numcolors;
		b = g + numcolors;

		if(!m_frs.readK(r, 3 * numcolors)) longjmp(jmp, 1);

		for(s32 i = 0; i < numcolors; i++)
		{
			m_pal[i].r = r[i];
			m_pal[i].g = g[i];
			m_pal[i].b = b[i];
		}
	break;
	}

	case RMT_RAW:
	{
		s8 colormap[m_rfh.ras_maplength];
		if(!m_frs.readK(colormap, m_rfh.ras_maplength)) longjmp(jmp, 1);
	break;
	}
    }

    m_rle = false;
    m_isRGB = false;

    switch(m_rfh.ras_type)
    {
	case RAS_OLD:
	case RAS_STANDARD:
	case RAS_TIFF:
	case RAS_IFF:
	break;

	case RAS_BYTE_ENCODED:
	    m_rle = true;
	break;

	case RAS_RGB:
	    m_isRGB = true;
	break;
    }
    
    if(m_rfh.ras_depth == 1)
	m_linelen = (short)((m_rfh.ras_width / 8) + (m_rfh.ras_width % 8 ? 1 : 0));
    else
	m_linelen = (short)m_rfh.ras_width;

    m_fill = (m_linelen % 2) ? 1 : 0;

    u8 m_buf[m_rfh.ras_width * sizeof(RGB)];

    m_bytes = w * h * sizeof(RGBA);

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "RGB" << "\n"
        << ((m_isRGB) ? "-":"RLE") << "\n"
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
        RGBA 	*scan = *image + h2 * w;
	
        u32 i;

	switch(bpp)
	{
    	    case 1:
	    break;

	    case 8:
		if(!fmt_readdata(m_frs, m_buf, m_linelen, m_rle))
		    longjmp(jmp, 1);

		for(i = 0;i < m_rfh.ras_width;i++)
		    memcpy(scan+i, &m_pal[i], sizeof(RGB));

		if(m_fill)
		{
		    if(!fmt_readdata(m_frs, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
    	    break;

	    case 24:
	    {
		u8 *b = m_buf;
	    
		if(!fmt_readdata(m_frs, m_buf, m_rfh.ras_width * 3, m_rle))
		    longjmp(jmp, 1);

		if(m_isRGB)
	    	    for (i = 0; i < m_rfh.ras_width; i++)
	    	    {
	    		scan[i].r = *b;
    	    		scan[i].g = *(b+1);
			scan[i].b = *(b+2);
			b += 3;
		    }
		else
	    	    for (i = 0; i < m_rfh.ras_width; i++)
	    	    {
		        scan[i].r = *(b + 2);
			scan[i].g = *(b + 1);
			scan[i].b = *b;
			b += 3;
		    }

		if(m_fill)
		{
	    	    if(!fmt_readdata(m_frs, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
	    }	
	    break;

	    case 32:
	    {
		u8 *b = m_buf;
	    
		if(!fmt_readdata(m_frs, m_buf, m_rfh.ras_width * 4, m_rle))
		    longjmp(jmp, 1);

		if(m_isRGB)
	    	    for (i = 0; i < m_rfh.ras_width; i++)
	    	    {
	    		scan[i].a = *b;
	    		scan[i].r = *(b+1);
    	    		scan[i].g = *(b+2);
			scan[i].b = *(b+3);
			b += 4;
		    }
		else
	    	    for (i = 0; i < m_rfh.ras_width; i++)
	    	    {
			scan[i].r = *(b + 3);
			scan[i].g = *(b + 2);
			scan[i].b = *(b + 1);
			scan[i].a = *b;
			b += 4;
		    }

		if(m_fill)
		{
	    	    if(!fmt_readdata(m_frs, &m_fillchar, m_fill, m_rle))
			longjmp(jmp, 1);
		}
	    }
	    break;
	}
    }

    m_frs.close();

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    frs.close();

    if(buf)
	delete [] buf;

    finfo.meta.clear();
    finfo.image.clear();
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal;
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


bool fmt_readdata(ifstreamK &ff, u8 *_buf, u32 length, bool rle)
{
    u8 repchar, remaining = 0;

    if(rle)
    {
	while(length--)
	{
		if (remaining)
		{
    		    remaining--;
		    *(_buf++)= repchar;
		}
		else
		{
			if(!ff.readK(&repchar, 1)) return false;

			if(repchar == RESC)
			{
				if(!ff.readK(&remaining, 1)) return false;

				if (remaining == 0)
					*(_buf++) = RESC;
				else
				{
					if(!ff.readK(&repchar, 1)) return false;
					*(_buf++) = repchar;
				}
			}
			else
				*(_buf++) = repchar;
		}
	}
    }
    else
    {
	if(!ff.readK(_buf, length)) return false;
	
    }

    return true;
}

/*  This file is part of the ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004,2005 Dmitry Baryshev <ksquirrel@tut.by>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License (LGPL) as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.
			
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
					
    You should have received a copy of the GNU Library General Public License
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
							
#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_gif_defs.h"

#include "gif_lib.h"
#include "fmt_codec_gif.h"
#include "error.h"

/*
 *
 * Originally designed to facilitate image transfer and online
 * storage for use by CompuServe and its customers,
 * GIF is primarily an exchange and storage
 * format, although it is based on, and is supported by, many
 * applications.
 * 
 */

static s32
InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

fmt_codec::fmt_codec() : fmt_codec_base()
{
    cerr << "libSQ_codec_gif: using libungif 4.1.3" << endl;
}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.3.1");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Compuserve GIF");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.gif ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("\x0047\x0049\x0046\x0038[\x0039\x0037]\x0061");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,128,0,4,4,4,88,206,239,123,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,193,17,128,48,8,4,175,133,60,104,224,164,3,83,1,82,128,62,210,127,43,2,65,247,181,115,3,7,0,86,130,224,25,99,72,9,73,89,45,17,157,13,166,23,23,166,169,209,120,96,170,187,90,38,170,33,76,177,78,106,38,229,219,250,123,118,51,165,143,214,213,122,227,126,1,99,132,23,143,7,210,12,10,0,0,0,0,73,69,78,68,174,66,96,130,130");
}
    
/* inits decoding of 'file': opens it, fills struct fmt_info  */
s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
        return SQERR_NOFILE;
	    
    frs.close();

    transIndex = -1;

    gif = DGifOpenFileName(file.c_str());

    Last = 0;

    linesz = gif->SWidth * sizeof(GifPixelType);

    if((buf = (u8*)malloc(linesz)) == NULL)
        return SQERR_NOMEMORY;

    if((saved = (RGBA *)calloc(linesz, sizeof(RGBA))) == NULL)
        return SQERR_NOMEMORY;

    if(gif->SColorMap)
    {
	back.r = gif->SColorMap->Colors[gif->SBackGroundColor].Red;
	back.g = gif->SColorMap->Colors[gif->SBackGroundColor].Green;
	back.b = gif->SColorMap->Colors[gif->SBackGroundColor].Blue;
	back.a = 255;
    }
    else
    {
	memset(&back, 0, sizeof(RGBA));
    }

    layer = -1;
    line = 0;
    curLine = 0;

    Lines_h = gif->SHeight;
    Lines = (RGBA **)calloc(Lines_h, sizeof(RGBA*));

    if(!Lines)
	return SQERR_NOMEMORY;

    for(s32 i = 0;i < Lines_h;i++)
    {
	Lines[i] = (RGBA *)0;
    }

    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

    Last = (RGBA **)calloc(gif->SHeight, sizeof(RGBA*));

    if(!Last)
	return SQERR_NOMEMORY;

    for(s32 i = 0;i < gif->SHeight;i++)
    {
	Last[i] = (RGBA *)0;
    }

    for(s32 i = 0;i < gif->SHeight;i++)
    {
	Last[i] = (RGBA *)calloc(gif->SWidth, sizeof(RGBA));

	if(!Last[i])
	    return SQERR_NOMEMORY;

//	for(s32 k = 0;k < gif->SWidth;k++)
//	    memcpy(Last[i]+k, &back, sizeof(RGBA));
    }

    currentImage = -1;
    lastDisposal = DISPOSAL_NO;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    layer++;
    currentPass++;
    line = 0;
    curLine = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    bool foundExt = false;

    currentImage++;
	
    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].interlaced = gif->Image.Interlace;
    finfo.image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

//    prs32f("Entering fmt_next\n\n");

    while(true)
    {
        if (DGifGetRecordType(gif, &record) == GIF_ERROR)
	{
//	    prs32f("DGifGetRecordType(gif, &record) == GIF_ERROR\n");
	    PrintGifError();
	    return SQERR_BADFILE;
	}

//	prs32f("record = %d\n", record);
	switch(record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gif) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQERR_BADFILE;
		}

		finfo.images++;
	    
		if(!foundExt)
		{
		    lastDisposal = disposal;
		    disposal = DISPOSAL_NO;
		    finfo.image[currentImage].delay = 100;
		    transIndex = -1;
		    finfo.image[currentImage].hasalpha = true;
		}

		lastRow = (currentImage) ? Row : gif->Image.Top;
		lastCol = (currentImage) ? Col : gif->Image.Left;
		Row = gif->Image.Top;
		Col = gif->Image.Left;
		finfo.image[currentImage].w = gif->SWidth;
		finfo.image[currentImage].h = gif->SHeight;
		lastWidth = (currentImage) ? Width : gif->Image.Width;
		lastHeight = (currentImage) ? Height : gif->Image.Height;
		Width = gif->Image.Width;
		Height = gif->Image.Height;
		finfo.image[currentImage].bpp = 8;

		curLine = 0;

		if(gif->Image.Left + gif->Image.Width > gif->SWidth || gif->Image.Top + gif->Image.Height > gif->SHeight)
		{
	    	    return SQERR_BADFILE;
		}
	    break;

	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gif, &ExtCode, &Extension) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQERR_BADFILE;
		}
		
		if(ExtCode == 249)
		{
		    foundExt = true;

		    lastDisposal = disposal;
		    disposal = (Extension[1] >> 2) & 7;
		    bool b = Extension[1] & 1;
		    s32 u = (unsigned)*(Extension + 2);
		    finfo.image[currentImage].delay = (!u) ? 100 : (u * 10);

		    if(b)
		      transIndex = Extension[4];
		      
		    finfo.image[currentImage].hasalpha = b;
		}
		else if(ExtCode == 254)
		{
//		    prs32f("SIZE: %d\n", Extension[0]);
//		    prs32f("Record EXT 254\n");
//
//		    for(s32 s = 0;s < Extension[0];s++)
//		    prs32f("%c", Extension[1+s]);
		    if(Extension[0])
		    {
    			finfo.meta.push_back(fmt_metaentry());

			s8 d[Extension[0]+1];
			memcpy(d, (s8*)Extension+1, Extension[0]);
			d[Extension[0]] = '\0';

			for(s32 s = 0;s < Extension[0];s++)
			    if(d[s] == '\n')
				d[s] = ' ';

                	finfo.meta[0].group = "GIF Comment";
			finfo.meta[0].data = d;
		    }
		}

		while(Extension)
		{
		    if(DGifGetExtensionNext(gif, &Extension) == GIF_ERROR)
		    {
			PrintGifError();
			return SQERR_BADFILE;
		    }
		}
	    break;
	    
	    case TERMINATE_RECORD_TYPE:
		return SQERR_NOTOK;

	    default: ;
	}

	if(record == IMAGE_DESC_RECORD_TYPE)
	{
	    if(currentImage >= 1)
		finfo.animated = true;

	    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

	    back.a = (transIndex != -1) ? 0 : 255;

	    for(s32 k = 0;k < gif->SWidth;k++)
	    {
    		memcpy(saved+k, &back, sizeof(RGBA));
	    }
/*
	    snprs32f(finfo.image[currentImage].dump, sizeof(finfo.image[currentImage].dump), "%s\n%dx%d\n%d\n%s\nLZW\n%d\n",
		fmt_quickinfo(),
    		finfo.image[currentImage].w,
		finfo.image[currentImage].h,
		finfo.image[currentImage].bpp,
		"Color indexed",
		finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA));
*/
	    stringstream s;

	    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    	    s   << fmt_quickinfo() << "\n"
	        << finfo.image[currentImage].w << "x"
		<< finfo.image[currentImage].h << "\n"
		<< finfo.image[currentImage].bpp << "\n"
		<< "Color indexed" << "\n"
		<< "LZW\n"
		<< bytes;

	    finfo.image[currentImage].dump = s.str();

	    finfo.image[currentImage].interlaced = gif->Image.Interlace;
	    finfo.image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

	    layer = -1;
	    currentPass = -1;

	    return SQERR_OK;
	}
    }
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    if(curLine < Row || curLine >= Row + Height)
    {
    	if(currentPass == finfo.image[currentImage].passes-1)
	{
	    memcpy(scan, Last[curLine], finfo.image[currentImage].w * sizeof(RGBA));

	    if(lastDisposal == DISPOSAL_BACKGROUND)
		if(curLine >= lastRow && curLine < lastRow+lastHeight)
		{
		    memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));
		    memcpy(Last[curLine], scan, finfo.image[currentImage].w * sizeof(RGBA));
		}
	}
	
	curLine++;

	return SQERR_OK;
    }

    curLine++;

    s32 i;
    s32 index;

    if(gif->Image.Interlace)
    {
	memcpy(scan, Last[curLine-1], finfo.image[currentImage].w * sizeof(RGBA));

	if(line == 0)
	    j = InterlacedOffset[layer];

	if(line == j)
	{
	    if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
	    {
	        PrintGifError();
	        memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
	        return SQERR_BADFILE;
	    }
	    else
	    {
		j += InterlacedJumps[layer];

		for(i = 0;i < Width;i++)
		{
		    index = Col + i;

		    if(buf[i] == transIndex && transIndex != -1)
		    {
			RGB rgb = *((RGB *)&(map->Colors[buf[i]]));

			if(back == rgb && !currentImage)
			    (scan+index)->a = 0;
			else if(back == rgb && lastDisposal != DISPOSAL_BACKGROUND && currentImage)
			{
			    RGBA *t = &Last[curLine-1][index];
			    memcpy(scan+index, t, sizeof(RGBA));
			}
			else if(back == rgb && lastDisposal == DISPOSAL_BACKGROUND && currentImage)
			{
			    (scan+index)->a = 0;
			}
			else if(currentImage)
			{
			    RGBA *t = &Last[curLine-1][index];

			    if(lastDisposal == DISPOSAL_BACKGROUND)
			    {
				memcpy(scan+index, &back, sizeof(RGBA));//(scan+index)->a=0;

				if(t->a == 0)
				(scan+index)->a=0;
			    }
			}
		    }
		    else
		    {
		        memcpy(scan+index, &(map->Colors[buf[i]]), sizeof(RGB));
		        (scan+index)->a = 255;
		    }
		}

		Lines[line] = (RGBA*)realloc(Lines[line], finfo.image[currentImage].w * sizeof(RGBA));

		if(!Lines[line])
		    return SQERR_NOMEMORY;
			
		memcpy(Lines[line], scan, finfo.image[currentImage].w * sizeof(RGBA));
	    }
	} // if(line == j)
	else
	{
	    if(Lines[line])
	        memcpy(scan, Lines[line], finfo.image[currentImage].w * sizeof(RGBA));
	    else
	        memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
	}

	if(currentPass == finfo.image[currentImage].passes-1)
	    memcpy(Last[curLine-1], scan, finfo.image[currentImage].w * sizeof(RGBA));

	line++;
    }
    else // !s32erlaced
    {
        if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
        {
	    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));
	    PrintGifError();
	    return SQERR_BADFILE;
	}
	else
	{
	    memcpy(scan, Last[curLine-1], finfo.image[currentImage].w * sizeof(RGBA));

	    if(lastDisposal == DISPOSAL_BACKGROUND)
	    {
		if(curLine-1 >= lastRow && curLine-1 < lastRow+lastHeight)
		    memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));
	    }

	    for(i = 0;i < Width;i++)
	    {
	        index = Col + i;

		if(buf[i] == transIndex && transIndex != -1)
		{
		    RGB rgb = *((RGB *)&(map->Colors[buf[i]]));

		    if(back == rgb && !currentImage)
			(scan+index)->a = 0;
		    else if(back == rgb && lastDisposal != DISPOSAL_BACKGROUND && currentImage)
		    {
			RGBA *t = &Last[curLine-1][index];
			memcpy(scan+index, t, sizeof(RGBA));// = 255;
		    }
		    else if(back == rgb && lastDisposal == DISPOSAL_BACKGROUND && currentImage)
		    {
			(scan+index)->a = 0;
		    }
		    else if(currentImage)
		    {
			RGBA *t = &Last[curLine-1][index];

			if(lastDisposal == DISPOSAL_BACKGROUND)
			{
			    memcpy(scan+index, &back, sizeof(RGBA));//(scan+index)->a=0;

			    if(t->a == 0)
				(scan+index)->a=0;
			}
		    }
		}// if transIndex
		else
		{
		    memcpy(scan+index, &(map->Colors[buf[i]]), sizeof(RGB));
		    (scan+index)->a = 255;
		}
	    } // for

	    memcpy(Last[curLine-1], scan, finfo.image[currentImage].w * sizeof(RGBA));
	}
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    GifFileType 	*m_gif;
    GifRecordType	m_record;
    GifByteType		*m_Extension;
    u8	**m_buf;
    s32			m_i, m_j, m_Row = 0, m_Col = 0, m_Width = 0, m_Height = 0, ExtCode, m_transIndex;
    RGBA		m_back;
    ColorMapObject	*m_map;
    s32 		w = 0, h = 0, bpp = 0;
    s32 		m_bytes;
    ifstreamK		m_frs;

    m_frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!m_frs.good())
        return SQERR_NOFILE;
	
    m_frs.close();

    m_transIndex = -1;

    m_gif = DGifOpenFileName(file.c_str());

    if((m_buf = (u8**)calloc(m_gif->SHeight, sizeof(u8*))) == NULL)
        return SQERR_NOMEMORY;

    for(m_i = 0;m_i < m_gif->SHeight;m_i++)
	m_buf[m_i] = (u8 *)0;

    for(m_i = 0;m_i < m_gif->SHeight;m_i++)
	if((m_buf[m_i] = (u8*)calloc(m_gif->SWidth, sizeof(u8))) == NULL)
	{
	    for(s32 k = 0;k < m_i;k++)
		free(m_buf[k]);

	    free(m_buf);

	    DGifCloseFile(m_gif);

    	    return SQERR_NOMEMORY;
	}

    if(m_gif->SColorMap)
    {
	m_back.r = m_gif->SColorMap->Colors[m_gif->SBackGroundColor].Red;
	m_back.g = m_gif->SColorMap->Colors[m_gif->SBackGroundColor].Green;
	m_back.b = m_gif->SColorMap->Colors[m_gif->SBackGroundColor].Blue;
	m_back.a = 255;
    }
    else
    {
	memset(&m_back, 0, sizeof(RGBA));
    }

    m_map = (m_gif->Image.ColorMap) ? m_gif->Image.ColorMap : m_gif->SColorMap;

    while(true)
    {
        if(DGifGetRecordType(m_gif, &m_record) == GIF_ERROR)
	{
	    PrintGifError();

	    for(s32 k = 0;k < m_gif->SHeight;k++)
		free(m_buf[k]);

	    free(m_buf);

	    DGifCloseFile(m_gif);
	
	    return SQERR_BADFILE;
	}

	switch(m_record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(m_gif) == GIF_ERROR)
		{
		    PrintGifError();
		    
		    for(s32 k = 0;k < m_gif->SHeight;k++)
			free(m_buf[k]);
		    
		    free(m_buf);
		    
		    DGifCloseFile(m_gif);

		    return SQERR_BADFILE;
		}

		m_Row = m_gif->Image.Top;
		m_Col = m_gif->Image.Left;
		w = m_gif->SWidth;
		h = m_gif->SHeight;
		m_Width = m_gif->Image.Width;
		m_Height = m_gif->Image.Height;
		bpp = 8;

		if(m_gif->Image.Left + m_gif->Image.Width > m_gif->SWidth || m_gif->Image.Top + m_gif->Image.Height > m_gif->SHeight)
		{
		    for(s32 k = 0;k < m_gif->SHeight;k++)
			free(m_buf[k]);

		    free(m_buf);

		    DGifCloseFile(m_gif);
	    	    return SQERR_BADFILE;
		}
	    break;

	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(m_gif, &ExtCode, &m_Extension) == GIF_ERROR)
		{
		    PrintGifError();

		    for(s32 k = 0;k < m_gif->SHeight;k++)
			free(m_buf[k]);

		    free(m_buf);

		    DGifCloseFile(m_gif);
		    return SQERR_BADFILE;
		}
		
		if(ExtCode == 249)
		{
		    bool b = m_Extension[1] & 1;

		    if(b)
			m_transIndex = m_Extension[4];
		}

		while(m_Extension)
		{
		    if(DGifGetExtensionNext(m_gif, &m_Extension) == GIF_ERROR)
		    {
			PrintGifError();

			for(s32 k = 0;k < m_gif->SHeight;k++)
			    free(m_buf[k]);

			free(m_buf);

			DGifCloseFile(m_gif);
			return SQERR_BADFILE;
		    }
		}
	    break;
	    
	    case TERMINATE_RECORD_TYPE:
		for(s32 k = 0;k < m_gif->SHeight;k++)
		    free(m_buf[k]);

		free(m_buf);

		DGifCloseFile(m_gif);
	    return SQERR_BADFILE;

	    default: ;
	}

	if(m_record == IMAGE_DESC_RECORD_TYPE)
	{
	    m_map = (m_gif->Image.ColorMap) ? m_gif->Image.ColorMap : m_gif->SColorMap;
	    m_back.a = (m_transIndex != -1) ? 0 : 255;
	    break;
	}
    }

    m_bytes = w * h * sizeof(RGBA);
/*
    sprs32f(dump, "%s\n%d\n%d\n%d\n%s\nLZW\n%d\n%d\n",
        fmt_quickinfo(),
        w,
        h,
        bpp,
        "Color indexed",
        1,
        m_bytes);
*/
    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Color indexed" << "\n"
        << "LZW" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {

	for(s32 k = 0;k < m_gif->SHeight;k++)
	    free(m_buf[k]);

	free(m_buf);

	DGifCloseFile(m_gif);

        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    if(m_gif->Image.Interlace)
    {
	for(m_i = 0;m_i < 4;m_i++)
	{
	    for(m_j = m_Row + InterlacedOffset[m_i];m_j < m_Row + m_Height;m_j += InterlacedJumps[m_i])
	    {
		if(DGifGetLine(m_gif, &m_buf[m_j][m_Col], m_Width) == GIF_ERROR)
		{
		    PrintGifError();
		}
	    }
	}
    }
    else
    {
	for(s32 h2 = m_Row;h2 < m_Height+m_Row;h2++)
	{
	    if(DGifGetLine(m_gif, &m_buf[h2][m_Col], m_Width) == GIF_ERROR)
	    {
	        PrintGifError();
	    }
	}
    }

    RGB *r;

    for(m_i = m_Row;m_i < m_Height+m_Row;m_i++)
    {
	RGBA *scan = *image + m_i * w;

	for(m_j = m_Col;m_j < m_Col+m_Width;m_j++)
	{
	    r = (RGB*)(&m_map->Colors[m_buf[m_i][m_j]]);

	    if(m_buf[m_i][m_j] == m_transIndex && m_transIndex != -1)
	    {
		if(back == *r)
		    (scan+m_j)->a = 0;
	    }
	    else
	    {
	        memcpy(scan+m_j, r, sizeof(RGB));
		(scan+m_j)->a = 255;
	    }
	}
    }

    for(s32 k = 0;k < m_gif->SHeight;k++)
	free(m_buf[k]);

    free(m_buf);

    DGifCloseFile(m_gif);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
{
    if(buf)
	free(buf);

    if(saved)
	free(saved);

    if(Lines)
    {
	for(s32 i = 0;i < Lines_h;i++)
	    if(Lines[i])
		free(Lines[i]);
		
	free(Lines);
	Lines = 0;
    }

    if(Last)
    {
	for(s32 i = 0;i < gif->SHeight;i++)
	    if(Last[i])
		free(Last[i]);

	free(Last);
	Last = 0;
    }

    finfo.meta.clear();
    finfo.image.clear();

    DGifCloseFile(gif);
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = true;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
}

s32 fmt_codec::fmt_writeimage(std::string , RGBA *, s32 , s32 , const fmt_writeoptions &)
{

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

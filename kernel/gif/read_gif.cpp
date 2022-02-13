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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
							
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "read_gif.h"
#include "gif_lib.h"

GifFileType 	*gif;
GifRecordType	record;
GifByteType	*Extension;
unsigned char	*buf;
RGBA		*saved;
int		i, j, Error, Row, Col, Width, Height, lastRow, lastCol, lastWidth, lastHeight, ExtCode, Count,
		transIndex, layer, line, Lines_h, curLine, linesz, disposal, lastDisposal, currentImage, currentPass;
RGBA		**Lines, back, **Last;
FILE		*fptr;
ColorMapObject	*map;

static int
InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

const char* fmt_version(void)
{
    return (const char*)"1.1.4";
}

const char* fmt_quickinfo(void)
{
    return (const char*)"Compuserve GIF";
}

const char* fmt_filter(void)
{
    return (const char*)"*.gif ";
}

const char* fmt_mime(void)
{
    return (const char*)"\x0047\x0049\x0046\x0038[\x0039\x0037]\x0061";
}

const char* fmt_pixmap(void)
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,0,128,0,4,4,4,88,206,239,123,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,193,17,128,48,8,4,175,133,60,104,224,164,3,83,1,82,128,62,210,127,43,2,65,247,181,115,3,7,0,86,130,224,25,99,72,9,73,89,45,17,157,13,166,23,23,166,169,209,120,96,170,187,90,38,170,33,76,177,78,106,38,229,219,250,123,118,51,165,143,214,213,122,227,126,1,99,132,23,143,7,210,12,10,0,0,0,0,73,69,78,68,174,66,96,130,130";
}
    
/* inits decoding of 'file': opens it, fills struct fmt_info  */
int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
        return SQERR_NOMEMORY;

    fptr = fopen(file, "rb");

    if(!fptr)
        return SQERR_NOFILE;

    transIndex = -1;

    gif = DGifOpenFileName(file);

    Last = 0;

    linesz = gif->SWidth * sizeof(GifPixelType);

    if((buf = (unsigned char*)malloc(linesz)) == NULL)
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

    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

    Last = (RGBA **)calloc(gif->SHeight, sizeof(RGBA*));

    if(!Last)
	return SQERR_NOMEMORY;
	
    for(int i = 0;i < gif->SHeight;i++)
    {
	Last[i] = (RGBA *)calloc(gif->SWidth, sizeof(RGBA));

	if(!Last[i])
	    return SQERR_NOMEMORY;

//	for(int k = 0;k < gif->SWidth;k++)
//	    memcpy(Last[i]+k, &back, sizeof(RGBA));
    }

    currentImage = -1;
    lastDisposal = DISPOSAL_NO;

    
    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    layer++;
    currentPass++;
    line = 0;
    curLine = 0;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    bool foundExt = false;

    currentImage++;
	
    finfo->image[currentImage].interlaced = gif->Image.Interlace;
    finfo->image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

//    printf("Entering fmt_next\n\n");

    while(true)
    {
        if (DGifGetRecordType(gif, &record) == GIF_ERROR)
	{
//	    printf("DGifGetRecordType(gif, &record) == GIF_ERROR\n");
	    PrintGifError();
	    return SQERR_BADFILE;
	}

//	printf("record = %d\n", record);
	switch(record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gif) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQERR_BADFILE;
		}

		finfo->images++;
	    
		if(!foundExt)
		{
		    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));
		    lastDisposal = disposal;
		    disposal = DISPOSAL_NO;
		    finfo->image[currentImage].delay = 100;
		    transIndex = -1;
		    finfo->image[currentImage].hasalpha = true;
		}

		lastRow = (currentImage) ? Row : gif->Image.Top;
		lastCol = (currentImage) ? Col : gif->Image.Left;
		Row = gif->Image.Top;
		Col = gif->Image.Left;
		finfo->image[currentImage].w = gif->SWidth;
		finfo->image[currentImage].h = gif->SHeight;
		lastWidth = (currentImage) ? Width : gif->Image.Width;
		lastHeight = (currentImage) ? Height : gif->Image.Height;
		Width = gif->Image.Width;
		Height = gif->Image.Height;
		finfo->image[currentImage].bpp = 8;

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

		    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

		    lastDisposal = disposal;
		    disposal = (Extension[1] >> 2) & 7;
		    bool b = Extension[1] & 1;
		    int u = (unsigned)*(Extension + 2);
		    finfo->image[currentImage].delay = (!u) ? 100 : (u * 10);

		    if(b)
		      transIndex = Extension[4];
		      
		    finfo->image[currentImage].hasalpha = b;
		}
		else if(ExtCode == 254)
		{
		    //Extension[2];
//		    printf("Record EXT 254\n");
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
		finfo->animated = true;

	    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

	    back.a = (transIndex != -1) ? 0 : 255;

	    for(int k = 0;k < gif->SWidth;k++)
	    {
    		memcpy(saved+k, &back, sizeof(RGBA));
	    }

	    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\nLZW\n%d\n",
		fmt_quickinfo(),
    		finfo->image[currentImage].w,
		finfo->image[currentImage].h,
		finfo->image[currentImage].bpp,
		"Color indexed",
		finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA));

	    finfo->image[currentImage].interlaced = gif->Image.Interlace;
	    finfo->image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

	    layer = -1;
	    currentPass = -1;

	    return SQERR_OK;
	}
    }
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    if(curLine < Row || curLine >= Row + Height)
    {
    	if(currentPass == finfo->image[currentImage].passes-1)
	{
	    memcpy(scan, Last[curLine], finfo->image[currentImage].w * sizeof(RGBA));

	    if(lastDisposal == DISPOSAL_BACKGROUND)
		if(curLine >= lastRow && curLine < lastRow+lastHeight)
		{
		    memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));
		    memcpy(Last[curLine], scan, finfo->image[currentImage].w * sizeof(RGBA));
		}
	}
	
	curLine++;

	return SQERR_OK;
    }

    curLine++;

    int i;
    int index;

    if(gif->Image.Interlace)
    {
	memcpy(scan, Last[curLine-1], finfo->image[currentImage].w * sizeof(RGBA));

	if(line == 0)
	    j = InterlacedOffset[layer];

	if(line == j)
	{
	    if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
	    {
	        PrintGifError();
	        memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
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

		Lines[line] = (RGBA*)realloc(Lines[line], finfo->image[currentImage].w * sizeof(RGBA));

		if(!Lines[line])
		    return SQERR_NOMEMORY;
			
		memcpy(Lines[line], scan, finfo->image[currentImage].w * sizeof(RGBA));
	    }
	} // if(line == j)
	else
	{
	    if(Lines[line])
	        memcpy(scan, Lines[line], finfo->image[currentImage].w * sizeof(RGBA));
	    else
	        memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
	}

	if(currentPass == finfo->image[currentImage].passes-1)
	    memcpy(Last[curLine-1], scan, finfo->image[currentImage].w * sizeof(RGBA));

	line++;
    }
    else // !interlaced
    {
        if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
        {
	    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
	    PrintGifError();
	    return SQERR_BADFILE;
	}
	else
	{
	    memcpy(scan, Last[curLine-1], finfo->image[currentImage].w * sizeof(RGBA));

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

	    memcpy(Last[curLine-1], scan, finfo->image[currentImage].w * sizeof(RGBA));
	}
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    GifFileType 	*m_gif;
    GifRecordType	m_record;
    GifByteType		*m_Extension;
    unsigned char	**m_buf;
    int			m_i, m_j, m_Row = 0, m_Col = 0, m_Width = 0, m_Height = 0, ExtCode, m_transIndex;
    RGBA		m_back;
    FILE		*m_fptr;
    ColorMapObject	*m_map;
    int 		w = 0, h = 0, bpp = 0;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;
	
    fclose(m_fptr);

    m_transIndex = -1;

    m_gif = DGifOpenFileName(file);

    if((m_buf = (unsigned char**)calloc(m_gif->SHeight, sizeof(unsigned char*))) == NULL)
        return SQERR_NOMEMORY;

    for(m_i = 0;m_i < m_gif->SHeight;m_i++)
	if((m_buf[m_i] = (unsigned char*)calloc(m_gif->SWidth, sizeof(unsigned char))) == NULL)
	{
	    for(int k = 0;k < m_i;k++)
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

	    for(int k = 0;k < m_gif->SHeight;k++)
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
		    
		    for(int k = 0;k < m_gif->SHeight;k++)
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
		    for(int k = 0;k < m_gif->SHeight;k++)
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

		    for(int k = 0;k < m_gif->SHeight;k++)
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

			for(int k = 0;k < m_gif->SHeight;k++)
			    free(m_buf[k]);

			free(m_buf);

			DGifCloseFile(m_gif);
			return SQERR_BADFILE;
		    }
		}
	    break;
	    
	    case TERMINATE_RECORD_TYPE:
		for(int k = 0;k < m_gif->SHeight;k++)
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

    int m_bytes = w * h * sizeof(RGBA);

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\nLZW\n%d\n%d\n",
        fmt_quickinfo(),
        w,
        h,
        bpp,
        "Color indexed",
        1,
        m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        trace("libSQ_read_gif: Image is null!");

	for(int k = 0;k < m_gif->SHeight;k++)
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
	for(int h2 = m_Row;h2 < m_Height+m_Row;h2++)
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

    for(int k = 0;k < m_gif->SHeight;k++)
	free(m_buf[k]);

    free(m_buf);

    DGifCloseFile(m_gif);

    return SQERR_OK;
}

int fmt_close()
{
    if(buf)
	free(buf);

    if(saved)
	free(saved);

    if(Lines)
    {
	for(int i = 0;i < Lines_h;i++)
	    if(Lines[i])
		free(Lines[i]);
		
	free(Lines);
	Lines = 0;
    }

    if(Last)
    {
	for(int i = 0;i < gif->SHeight;i++)
	    if(Last[i])
		free(Last[i]);

	free(Last);
	Last = 0;
    }

    DGifCloseFile(gif);

    return SQERR_OK;
}

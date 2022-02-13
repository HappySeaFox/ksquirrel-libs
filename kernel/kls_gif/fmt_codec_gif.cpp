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

#include <iostream>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_gif_defs.h"

extern "C" {
#include "gif_lib.h"
}

#include "fmt_codec_gif.h"

#include "../xpm/codec_gif.xpm"

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
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "1.3.1";
    o->name = "Compuserve GIF";
    o->filter = "*.gif ";
    o->config = "";
    o->mime = "\x0047\x0049\x0046\x0038[\x0039\x0037]\x0061";
    o->mimetype = "image/gif";
    o->pixmap = codec_gif;
    o->readable = true;
    o->canbemultiple = true;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

s32 fmt_codec::read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
        return SQE_R_NOFILE;
	    
    frs.close();

    transIndex = -1;

    Last = 0;
    Lines = 0;
    buf = 0;
    saved = 0;

    gif = DGifOpenFileName(file.c_str());

    // for safety...
    if(!gif)
        return SQE_R_BADFILE;

    linesz = gif->SWidth * sizeof(GifPixelType);

    if((buf = (u8*)malloc(linesz)) == NULL)
        return SQE_R_NOMEMORY;

    if((saved = (RGBA *)calloc(linesz, sizeof(RGBA))) == NULL)
        return SQE_R_NOMEMORY;

    if(gif->SColorMap)
    {
	back.r = gif->SColorMap->Colors[gif->SBackGroundColor].Red;
	back.g = gif->SColorMap->Colors[gif->SBackGroundColor].Green;
	back.b = gif->SColorMap->Colors[gif->SBackGroundColor].Blue;
	back.a = 255;
    }
    else
	memset(&back, 0, sizeof(RGBA));

    layer = -1;
    line = 0;
    curLine = 0;

    Lines_h = gif->SHeight;
    Lines = (RGBA **)malloc(Lines_h * sizeof(RGBA*));

    if(!Lines)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < Lines_h;i++)
	Lines[i] = (RGBA *)0;

    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

    Last = (RGBA **)malloc(gif->SHeight * sizeof(RGBA*));

    if(!Last)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < gif->SHeight;i++)
	Last[i] = (RGBA *)0;

    for(s32 i = 0;i < gif->SHeight;i++)
    {
	Last[i] = (RGBA *)calloc(gif->SWidth, sizeof(RGBA));

	if(!Last[i])
	    return SQE_R_NOMEMORY;

//	for(s32 k = 0;k < gif->SWidth;k++)
//	    memcpy(Last[i]+k, &back, sizeof(RGBA));
    }

    currentImage = -1;
    lastDisposal = DISPOSAL_NO;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    layer++;
    currentPass++;
    line = 0;
    curLine = 0;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    bool foundExt = false;

    currentImage++;

    fmt_image image;

    image.interlaced = gif->Image.Interlace;
    image.passes = (gif->Image.Interlace) ? 4 : 1;

    while(true)
    {
        if (DGifGetRecordType(gif, &record) == GIF_ERROR)
	{
	    PrintGifError();
	    return SQE_R_BADFILE;
	}

	switch(record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gif) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQE_R_BADFILE;
		}

		if(!foundExt)
		{
		    lastDisposal = disposal;
		    disposal = DISPOSAL_NO;
		    image.delay = 100;
		    transIndex = -1;
		    image.hasalpha = true;
		}

		lastRow = (currentImage) ? Row : gif->Image.Top;
		lastCol = (currentImage) ? Col : gif->Image.Left;
		Row = gif->Image.Top;
		Col = gif->Image.Left;
		image.w = gif->SWidth;
		image.h = gif->SHeight;
		lastWidth = (currentImage) ? Width : gif->Image.Width;
		lastHeight = (currentImage) ? Height : gif->Image.Height;
		Width = gif->Image.Width;
		Height = gif->Image.Height;
		image.bpp = 8;

		curLine = 0;

		if(gif->Image.Left + gif->Image.Width > gif->SWidth || gif->Image.Top + gif->Image.Height > gif->SHeight)
		{
	    	    return SQE_R_BADFILE;
		}
	    break;

	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gif, &ExtCode, &Extension) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQE_R_BADFILE;
		}

                if(!Extension)
                    break;

		if(ExtCode == 249)
		{
		    foundExt = true;

		    lastDisposal = disposal;
		    disposal = (Extension[1] >> 2) & 7;
		    bool b = Extension[1] & 1;
		    s32 u = (unsigned)*(Extension + 2);
		    image.delay = (!u) ? 100 : (u * 10);

		    if(b)
		      transIndex = Extension[4];
		      
		    image.hasalpha = b;
		}
		else if(ExtCode == 254 && Extension[0])
		{
    		    fmt_metaentry mt;
    		    s8 d[Extension[0]+1];

                    memcpy(d, (s8*)Extension+1, Extension[0]);
                    d[Extension[0]] = '\0';

                    for(s32 s = 0;s < Extension[0];s++)
                        if(d[s] == '\n')
                            d[s] = ' ';

                    mt.group = "Comment";
                    mt.data = d;

                    addmeta(mt);
		}

		while(Extension)
		{
		    if(DGifGetExtensionNext(gif, &Extension) == GIF_ERROR)
		    {
			PrintGifError();
			return SQE_R_BADFILE;
		    }
		}
	    break;

	    case TERMINATE_RECORD_TYPE:
		return SQE_NOTOK;

	    default: ;
	}

	if(record == IMAGE_DESC_RECORD_TYPE)
	{
	    if(currentImage >= 1)
		finfo.animated = true;

	    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

	    back.a = (transIndex != -1) ? 0 : 255;

	    for(s32 k = 0;k < gif->SWidth;k++)
    		memcpy(saved+k, &back, sizeof(RGBA));

	    image.compression = "LZW";
    	    image.colorspace = "Color indexed";
	    image.interlaced = gif->Image.Interlace;
	    image.passes = (gif->Image.Interlace) ? 4 : 1;
    	
	    finfo.image.push_back(image);

	    layer = -1;
	    currentPass = -1;

	    return SQE_OK;
	}
    }
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(curLine < Row || curLine >= Row + Height)
    {
    	if(currentPass == im->passes-1)
	{
	    memcpy(scan, Last[curLine], im->w * sizeof(RGBA));

	    if(lastDisposal == DISPOSAL_BACKGROUND)
		if(curLine >= lastRow && curLine < lastRow+lastHeight)
		{
		    memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));
		    memcpy(Last[curLine], scan, im->w * sizeof(RGBA));
		}
	}
	
	curLine++;

	return SQE_OK;
    }

    curLine++;

    s32 i;
    s32 index;

    if(gif->Image.Interlace)
    {
	memcpy(scan, Last[curLine-1], im->w * sizeof(RGBA));

	if(line == 0)
	    j = InterlacedOffset[layer];

	if(line == j)
	{
	    if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
	    {
	        PrintGifError();
	        memset(scan, 255, im->w * sizeof(RGBA));
	        return SQE_R_BADFILE;
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

		Lines[line] = (RGBA*)realloc(Lines[line], im->w * sizeof(RGBA));

		if(!Lines[line])
		    return SQE_R_NOMEMORY;
			
		memcpy(Lines[line], scan, im->w * sizeof(RGBA));
	    }
	} // if(line == j)
	else
	{
	    if(Lines[line])
	        memcpy(scan, Lines[line], im->w * sizeof(RGBA));
	    else
	        memset(scan, 255, im->w * sizeof(RGBA));
	}

	if(currentPass == im->passes-1)
	    memcpy(Last[curLine-1], scan, im->w * sizeof(RGBA));

	line++;
    }
    else // !s32erlaced
    {
        if(DGifGetLine(gif, buf, Width) == GIF_ERROR)
        {
	    memset(scan, 255, im->w * sizeof(RGBA));
	    PrintGifError();
	    return SQE_R_BADFILE;
	}
	else
	{
	    memcpy(scan, Last[curLine-1], im->w * sizeof(RGBA));

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

	    memcpy(Last[curLine-1], scan, im->w * sizeof(RGBA));
	}
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    if(buf)   free(buf);
    if(saved) free(saved);

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

    if(gif) DGifCloseFile(gif);
}

#include "fmt_codec_cd_func.h"

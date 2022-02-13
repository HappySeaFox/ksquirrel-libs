/*  This file is part of the ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>
    
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
GifRowType	buf;
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
    return (const char*)"1.1.1";
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

    if((buf = (GifRowType)malloc(linesz)) == NULL)
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

    finfo->interlaced = gif->Image.Interlace;
    finfo->passes = (finfo->interlaced) ? 4 : 1;

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

    for(int k = 0;k < gif->SWidth;k++)
    {
        memcpy(saved+k, &back, sizeof(RGBA));
    }
    
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
	
    printf("Entering fmt_next\n\n");

    while(true)
    {
        if (DGifGetRecordType(gif, &record) == GIF_ERROR)
	{
	    printf("DGifGetRecordType(gif, &record) == GIF_ERROR\n");
	    PrintGifError();
	    return SQERR_BADFILE;
	}

	printf("record = %d\n", record);
	switch(record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
	    if(DGifGetImageDesc(gif) == GIF_ERROR)
	    {
		PrintGifError();
//		free(buf);
		return SQERR_BADFILE;
	    }
	    printf("Record IMAGE_DESC_RECORD_TYPE\n");
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

	    printf("** fmt_next: %dx%d [%d,%d,%d,%d]\n", finfo->image[currentImage].w, finfo->image[currentImage].h,
			    Col, Row, Width, Height);

	    curLine = 0;

	    if(gif->Image.Left + gif->Image.Width > gif->SWidth || gif->Image.Top + gif->Image.Height > gif->SHeight)
	    {
	        return SQERR_BADFILE;
	    }

/*	    linesz = gif->SWidth * sizeof(GifPixelType);
	    
	    if((buf = (GifRowType)realloc(buf, linesz)) == NULL)
	    {
    		return SQERR_NOMEMORY;
	    }
*/
	    break;

	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gif, &ExtCode, &Extension) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQERR_BADFILE;
		}
		
		printf("ExtCode = %d\n", ExtCode);
		
		if(ExtCode == 249)
		{
		    printf("Record EXT 249\n");
		    foundExt = true;

//		    finfo->image = (fmt_image *)realloc(finfo->image, sizeof(fmt_image) * finfo->images);
		    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

		    lastDisposal = disposal;
		    disposal = (Extension[1] >> 2) & 7;
//		    printf("Disposal method: %d\n", disposal);
		    bool b = Extension[1] & 1;
		    int u = (unsigned)*(Extension + 2);
		    finfo->image[currentImage].delay = (!u) ? 100 : (u * 10);
//		    printf("**** fmt_next: Image #%d, Delay: %u\n", currentImage, finfo->image[currentImage].delay);
		    if(b)
		      transIndex = Extension[4];
		      
		    finfo->image[currentImage].hasalpha = b;
		}
		else if(ExtCode == 254)
		{
		    //Extension[2];
		    printf("Record EXT 254\n");
		}

		while(Extension)
		{
		    if(DGifGetExtensionNext(gif, &Extension) == GIF_ERROR)
		    {
			PrintGifError();
			return SQERR_BADFILE;
		    }
		    
		    printf("Found NEWEXT %d\n", Extension);
		}
	    break;
	    
	    case TERMINATE_RECORD_TYPE:
		printf("Found TERMINATE!\n");
		return SQERR_NOTOK;

	    default: ;
	}

	if(record == IMAGE_DESC_RECORD_TYPE)
	{
	    if(currentImage >= 1)
		finfo->animated = true;

	    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

	    printf("transIndex = %d\n", transIndex);

	    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\nLZW\n%d\n",
		fmt_quickinfo(),
    		finfo->image[currentImage].w,
		finfo->image[currentImage].h,
		finfo->image[currentImage].bpp,
		"Color indexed",
		finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA));

	    finfo->interlaced = gif->Image.Interlace;
	    finfo->passes = (finfo->interlaced) ? 4 : 1;
	    
	    layer = -1;
	    currentPass = -1;

	    printf("fmt_next: finfo->interlaced = %d\n", finfo->interlaced);

	    return SQERR_OK;
	}
    }
}

/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    if(curLine < Row || curLine >= Row + Height)
    {
	memcpy(scan, Last[curLine], finfo->image[currentImage].w * sizeof(RGBA));
//	memset(scan, 0, finfo->image[currentImage].w * sizeof(RGBA));

	if(lastDisposal == DISPOSAL_BACKGROUND)
	    if(curLine >= lastRow && curLine < lastRow+lastHeight)
	    {
		memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));
		memcpy(Last[curLine], scan, finfo->image[currentImage].w * sizeof(RGBA));
	    }

	curLine++;

	return SQERR_OK;
    }
    
    curLine++;

    int i;
    
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
			int index = Col + i;

			if(buf[i] == transIndex && transIndex != -1)
			{
			    if(!currentImage)
			    {
    			        RGB *rgb = (RGB*)&(map->Colors[buf[i]]);

				if(back.r == rgb->r && back.g == rgb->b && back.b == rgb->b)
			    	    (scan+index)->a = 0;
				else
				    memcpy(scan+index, &back, sizeof(RGBA));
			    }
			}
			else
			    memcpy(scan+index, &(map->Colors[buf[i]]), sizeof(RGB));
		    }

		    Lines[line] = (RGBA*)calloc(finfo->image[currentImage].w, sizeof(RGBA));

		    if(!Lines[line])
			return SQERR_NOMEMORY;
			
		    memcpy(Lines[line], scan, finfo->image[currentImage].w * sizeof(RGBA));
		};
	    }
	    else
	    {
		if(Lines[line])
		    memcpy(scan, Lines[line], finfo->image[currentImage].w * sizeof(RGBA));
		else
		    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
	    }

	    memcpy(Last[curLine-1], scan, finfo->image[currentImage].w * sizeof(RGBA));
	    line++;
    }
    else
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
//	    memset(scan, 0, finfo->image[currentImage].w * sizeof(RGBA));

	    if(lastDisposal == DISPOSAL_BACKGROUND)
		if(curLine-1 >= lastRow && curLine-1 < lastRow+lastHeight)
		    memcpy(scan+lastCol, saved, lastWidth * sizeof(RGBA));

	    int index;

	    for(i = 0;i < Width;i++)
	    {
	        index = Col + i;

		if(buf[i] == transIndex && transIndex != -1)
		{
		    RGB rgb = *((RGB *)&(map->Colors[buf[i]]));

		    if(back == rgb && !currentImage)
			(scan+index)->a = 0;
		    else if(back == rgb)
		    {
			RGBA *t = &Last[curLine-1][index];
//			if(t->a == 255)
			    memcpy(scan+index, t, sizeof(RGBA));// = 255;
//			    else
//			    (scan+index)->a = 0;
		    }
		    else
		    {
			memcpy(scan+index, &back, sizeof(RGBA));
			(scan+index)->a = 255;
		    }
		}
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

int fmt_readimage(const char *file, RGBA **scan, char **dump)
{
    file = file;
    scan = scan;
    dump = dump;

    return SQERR_OK;
}

int fmt_close()
{
    free(buf);

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

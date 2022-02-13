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

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_gif_defs.h"

#include "gif_lib.h"
#include "fmt_codec_gif.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,0,128,0,76,76,76,151,95,119,105,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,90,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,32,35,52,50,50,52,18,200,152,58,115,102,100,36,72,100,234,84,160,8,152,1,20,136,132,169,129,48,160,186,224,230,128,44,5,155,204,5,118,199,2,6,0,38,50,57,42,250,158,60,196,0,0,0,0,73,69,78,68,174,66,96,130");
}
    
/* inits decoding of 'file': opens it, fills struct fmt_info  */
s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
        return SQE_R_NOFILE;
	    
    frs.close();

    transIndex = -1;

    gif = DGifOpenFileName(file.c_str());

    Last = 0;

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
    {
	memset(&back, 0, sizeof(RGBA));
    }

    layer = -1;
    line = 0;
    curLine = 0;

    Lines_h = gif->SHeight;
    Lines = (RGBA **)calloc(Lines_h, sizeof(RGBA*));

    if(!Lines)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < Lines_h;i++)
    {
	Lines[i] = (RGBA *)0;
    }

    map = (gif->Image.ColorMap) ? gif->Image.ColorMap : gif->SColorMap;

    Last = (RGBA **)calloc(gif->SHeight, sizeof(RGBA*));

    if(!Last)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < gif->SHeight;i++)
    {
	Last[i] = (RGBA *)0;
    }

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
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    layer++;
    currentPass++;
    line = 0;
    curLine = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    bool foundExt = false;

    currentImage++;
	
    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].interlaced = gif->Image.Interlace;
    finfo.image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

//    prs32f("Entering fmt_read_next\n\n");

    while(true)
    {
        if (DGifGetRecordType(gif, &record) == GIF_ERROR)
	{
//	    prs32f("DGifGetRecordType(gif, &record) == GIF_ERROR\n");
	    PrintGifError();
	    return SQE_R_BADFILE;
	}

//	prs32f("record = %d\n", record);
	switch(record)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gif) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQE_R_BADFILE;
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
	    	    return SQE_R_BADFILE;
		}
	    break;

	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gif, &ExtCode, &Extension) == GIF_ERROR)
		{
		    PrintGifError();
		    return SQE_R_BADFILE;
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
	    finfo.image[currentImage].compression = "LZW";
    	    finfo.image[currentImage].colorspace = "Color indexed";
	    finfo.image[currentImage].interlaced = gif->Image.Interlace;
	    finfo.image[currentImage].passes = (gif->Image.Interlace) ? 4 : 1;

	    layer = -1;
	    currentPass = -1;

	    return SQE_OK;
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

	return SQE_OK;
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

		Lines[line] = (RGBA*)realloc(Lines[line], finfo.image[currentImage].w * sizeof(RGBA));

		if(!Lines[line])
		    return SQE_R_NOMEMORY;
			
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
	    return SQE_R_BADFILE;
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

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
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
    opt->passes = 1;
    opt->needflip = false;
}

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
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

bool fmt_codec::fmt_readable() const
{
    return true;
}
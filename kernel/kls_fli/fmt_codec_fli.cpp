/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

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

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/error.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_fli_defs.h"
#include "fmt_codec_fli.h"

#include "../xpm/codec_fli.xpm"

/*
 *
 * The FLI file format (sometimes called Flic)
 * is one of the most popular
 * animation formats found in the MS-DOS and Windows environments today. FLI is
 * used widely in animation programs, computer games, and CAD applications
 * requiring 3D manipulation of vector drawings. Flic, in common
 * with most animation formats, does not support either audio or video data, but
 * instead stores only sequences of still image data.
 *
 */

// maximum number of frames in FLI is 1024
#define MAX_FRAME 1024

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.3.2";
    o->name = "FLI Animation";
    o->filter = "*.fli ";
    o->config = "";
    o->mime = "";
    o->mimetype = "video/x-flic";
    o->pixmap = codec_fli;
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
			
    if(!frs.readK(&flic, sizeof(FLICHEADER)))
	return SQE_R_BADFILE;

    if(flic.FileId != 0xAF11)// && flic.FileId != 0xAF12)
	return SQE_R_BADFILE;

    if(flic.Flags != 3)
	cerr << "libSQ_read_fli: WARNING: Flags != 3" << endl;

    memset(pal, 0, 768);

    currentImage = -1;
    
    buf = (u8**)calloc(flic.Height, sizeof(u8*));

    if(!buf)
	return SQE_R_NOMEMORY;

    for(s32 i = 0;i < flic.Height;i++)
    {
	buf[i] = (u8*)0;
    }

    for(s32 i = 0;i < flic.Height;i++)
    {
	buf[i] = (u8*)calloc(flic.Width, sizeof(u8));
	
	if(!buf[i])
	    return SQE_R_NOMEMORY;
    }
    
    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage == flic.NumberOfFrames || currentImage == MAX_FRAME)
	return SQE_NOTOK;

    fmt_image image;

    image.w = flic.Width;
    image.h = flic.Height;
    image.bpp = 8;
    image.delay = (s32)((float)flic.FrameDelay * 14.3);
    finfo.animated = (currentImage) ? true : false;

//    prs32f("%dx%d@%d, delay: %d\n", flic.Width, flic.Height, flic.PixelDepth, finfo.image[currentImage].delay);

    CHUNKHEADER chunk;
    CHUNKHEADER subchunk;
    u16 subchunks;

    fstream::pos_type pos = frs.tellg();
//    prs32f("POS AFTER HEADER: %d\n", pos);

    while(true)
    {
	if(!skip_flood(frs))
	    return SQE_R_BADFILE;

	if(!frs.readK(&chunk, sizeof(CHUNKHEADER)))
	    return SQE_R_BADFILE;

//	prs32f("Read MAIN chunk: size: %d, type: %X\n", chunk.size, chunk.type);

	if(chunk.type != CHUNK_PREFIX_TYPE &&
		    chunk.type != CHUNK_SCRIPT_CHUNK && 
		    chunk.type != CHUNK_FRAME_TYPE &&
		    chunk.type != CHUNK_SEGMENT_TABLE &&
		    chunk.type != CHUNK_HUFFMAN_TABLE)
	    return SQE_R_BADFILE;

	if(chunk.type != CHUNK_FRAME_TYPE)
	    frs.seekg(chunk.size - sizeof(CHUNKHEADER), ios::cur);
	else
	{
	    if(!frs.readK(&subchunks, sizeof(u16)))
		return SQE_R_BADFILE;
//	    prs32f("Chunk #%X has %d subchunks\n", chunk.type, subchunks);
	    frs.seekg(sizeof(u16) * 4, ios::cur);
	    break;
	}
    }

//    prs32f("POS MAIN: %d\n", ftell(fptr));
//    fsetpos(fptr, (fpos_t*)&pos);
//    fseek(fptr, chunk.size, SEEK_CUR);
//    prs32f("POS2 MAIN: %d\n", ftell(fptr));

    while(subchunks--)
    {
        pos = frs.tellg(); 

        if(!frs.readK(&subchunk, sizeof(CHUNKHEADER)))
	    return SQE_R_BADFILE;

//	prs32f("*** Subchunk: %d\n", subchunk.type);

	switch(subchunk.type)
        {
    	    case CHUNK_COLOR_64:
    	    case CHUNK_COLOR_256:
	    {
//		prs32f("*** Palette64 CHUNK\n");
		u8 skip, count;
		u16 packets;
		RGB e;

		if(!frs.readK(&packets, sizeof(u16)))
		    return SQE_R_BADFILE;
//		prs32f("COLOR_64 packets: %d\n", packets);

		for(s32 i = 0;i < packets;i++)
		{
		    if(!frs.readK(&skip, 1)) return SQE_R_BADFILE;
		    if(!frs.readK(&count, 1)) return SQE_R_BADFILE;
//		    prs32f("COLOR64 skip: %d, count: %d\n", skip, count);

		    if(count)
		    {
			for(s32 j = 0;j < count;j++)
			{
			    if(!frs.readK(&e, sizeof(RGB))) return SQE_R_BADFILE;
//			    prs32f("COLOR_64 PALLETTE CHANGE %d,%d,%d\n", e.r, e.g, e.b);
			}
		    }
		    else
		    {
//			prs32f("Reading pallette...\n");
			if(!frs.readK(pal, sizeof(RGB) * 256)) return SQE_R_BADFILE;

			u8 *pp = (u8 *)pal;

			if(subchunk.type == CHUNK_COLOR_64)
			    for(s32 j = 0;j < 768;j++)
				pp[j] <<= 2;

//			for(s32 j = 0;j < 256;j++)
//			prs32f("COLOR_64 PALLETTE %d,%d,%d\n", pal[j].r, pal[j].g, pal[j].b);
//			prs32f("\n");
		    }
		}
	    }
	    break;

	    case CHUNK_RLE:
	    {
//		prs32f("*** RLE DATA CHUNK\n");
		u8 value;
		s8 c;
		s32 count;

		for(s32 j = 0;j < image.h;j++)
		{
		    s32 index = 0;
		    count = 0;
		    if(!frs.readK(&c, 1)) return SQE_R_BADFILE;

		    while(count < image.w)
		    {
			if(!frs.readK(&c, 1)) return SQE_R_BADFILE;

			if(c < 0)
			{
			    c = -c;

			    for(s32 i = 0;i < c;i++)
			    {
				if(!frs.readK(&value, 1)) return SQE_R_BADFILE;
				buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
			else
			{
			    if(!frs.readK(&value, 1)) return SQE_R_BADFILE;

			    for(s32 i = 0;i < c;i++)
			    {
				buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
		    }
		}
	    }
	    break;
	    
	    case CHUNK_DELTA_FLI:
	    {
		u16 starty, totaly, ally, index;
		u8 packets, skip, byte;
		s8 size;
		s32 count;

		if(!frs.readK(&starty, 2)) return SQE_R_BADFILE;
		if(!frs.readK(&totaly, 2)) return SQE_R_BADFILE;

		ally = starty + totaly;
		
//		prs32f("Y: %d, Total: %d\n", starty, totaly);

		for(s32 j = starty;j < ally;j++)
		{
		    count = 0;
		    index = 0;

		    if(!frs.readK(&packets, 1)) return SQE_R_BADFILE;

		    while(count < image.w)
		    {
			for(s32 k = 0;k < packets;k++)
			{
//			    prs32f("LINE %d\n", j);
			    if(!frs.readK(&skip, 1)) return SQE_R_BADFILE;
			    if(!frs.readK(&size, 1)) return SQE_R_BADFILE;

			    index += skip;
			    
//			    prs32f("SKIP: %d, SIZE: %d\n", skip, size);

			    if(size > 0)
			    {
				if(!frs.readK(buf[j]+index, size)) return SQE_R_BADFILE;
			    }
			    else if(size < 0)
			    {
				size = -size;
				if(!frs.readK(&byte, 1)) return SQE_R_BADFILE;
				memset(buf[j]+index, byte, size);
			    }

			    index += size;
			    count += size;
			}

			break;
		    }
		}
	    }
	    break;

	    case CHUNK_BLACK:
	    break;

	    case CHUNK_COPY:
	    {
//		prs32f("*** COPY DATA CHUNK\n");

		for(s32 j = 0;j < image.h;j++)
		{
		    if(!frs.readK(buf[j], image.w)) return SQE_R_BADFILE;
		}
	    }
	    break;

	    default:
//		prs32f("*** UNKNOWN CHUNK! SEEKING ANYWAY\n");
		frs.seekg(pos);
		frs.seekg(subchunk.size, ios::cur);
	}

//	prs32f("POS: %d\n", ftell(fptr));
//	prs32f("POS2: %d\n", ftell(fptr));
    }

    image.compression = "RLE/DELTA_FLI";
    image.colorspace = "Color indexed";

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    line++;
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    for(s32 i = 0;i < im->w;i++)
    {
	memcpy(scan+i, pal+buf[line][i], sizeof(RGB));
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    if(buf)
    {
	for(s32 i = 0;i < flic.Height;i++)
	    if(buf[i])
		free(buf[i]);

	free(buf);
    }

    frs.close();
    
    finfo.meta.clear();
    finfo.image.clear();
}

bool find_chunk_type(const u16 type)
{
    static const u16 A[] = 
    {
	CHUNK_PREFIX_TYPE,
	CHUNK_SCRIPT_CHUNK,
	CHUNK_FRAME_TYPE,
	CHUNK_SEGMENT_TABLE,
	CHUNK_HUFFMAN_TABLE
    };
    
    static const s32 S = 5;

    for(s32 i = 0;i < S;i++)
	if(type == A[i])
	    return true;
	    
    return false;
}

bool fmt_codec::skip_flood(ifstreamK &s)
{
    u8 _f[4];
    u16 b;
    fstream::pos_type _pos;
    
//    prs32f("SKIP_FLOOD pos: %d\n", ftell(f));

    if(!s.readK(_f, 4)) return false;

    do
    {
	_pos = s.tellg();
	if(!s.readK(&b, 2)) return false;
//	prs32f("SKIP_FLOOD b: %X\n", b);
	s.seekg(-1, ios::cur);
    }while(!find_chunk_type(b));

    _pos -= 4;

    s.seekg(_pos);
    
    return true;

//    prs32f("SKIP_FLOOD pos2: %d\n", ftell(f));
}

#include "fmt_codec_cd_func.h"

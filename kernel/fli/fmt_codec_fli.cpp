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

#include <csetjmp>
#include <sstream>
#include <iostream>

#include "fmt_types.h"
#include "fmt_codec_fli_defs.h"
#include "fmt_codec_fli.h"

#include "error.h"

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

static const s32 MAX_FRAME = 256;

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.3.2");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("FLI Animation");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.fli ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,170,8,138,4,4,4,227,59,112,255,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,189,2,15,22,176,216,160,153,128,50,64,251,200,254,171,148,16,154,123,157,44,48,0,152,11,36,175,136,104,9,73,157,45,25,93,13,70,20,55,6,233,225,76,177,240,8,91,137,181,156,36,133,53,243,111,157,158,221,76,237,163,117,181,222,120,62,112,172,23,177,216,139,37,27,0,0,0,0,73,69,78,68,174,66,96,130,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    frs.open(file.c_str(), ios::binary | ios::in);
    
    if(!frs.good())
        return SQERR_NOFILE;
			
    if(!frs.readK(&flic, sizeof(FLICHEADER)))
	return SQERR_BADFILE;

    if(flic.FileId != 0xAF11)// && flic.FileId != 0xAF12)
	return SQERR_BADFILE;

    if(flic.Flags != 3)
	cerr << "libSQ_read_fli: WARNING: Flags != 3" << endl;

    memset(pal, 0, 768);

    currentImage = -1;
    
    buf = (u8**)calloc(flic.Height, sizeof(u8*));

    if(!buf)
	return SQERR_NOMEMORY;

    for(s32 i = 0;i < flic.Height;i++)
    {
	buf[i] = (u8*)0;
    }

    for(s32 i = 0;i < flic.Height;i++)
    {
	buf[i] = (u8*)calloc(flic.Width, sizeof(u8));
	
	if(!buf[i])
	    return SQERR_NOMEMORY;
    }
    
    finfo.images = 0;
    finfo.animated = false;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage == flic.NumberOfFrames || currentImage == MAX_FRAME)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;
    finfo.image[currentImage].w = flic.Width;
    finfo.image[currentImage].h = flic.Height;
    finfo.image[currentImage].bpp = 8;
    finfo.image[currentImage].delay = (s32)((float)flic.FrameDelay * 14.3);
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
	    return SQERR_BADFILE;

	if(!frs.readK(&chunk, sizeof(CHUNKHEADER)))
	    return SQERR_BADFILE;

//	prs32f("Read MAIN chunk: size: %d, type: %X\n", chunk.size, chunk.type);

	if(chunk.type != CHUNK_PREFIX_TYPE &&
		    chunk.type != CHUNK_SCRIPT_CHUNK && 
		    chunk.type != CHUNK_FRAME_TYPE &&
		    chunk.type != CHUNK_SEGMENT_TABLE &&
		    chunk.type != CHUNK_HUFFMAN_TABLE)
	    return SQERR_BADFILE;

	if(chunk.type != CHUNK_FRAME_TYPE)
	    frs.seekg(chunk.size - sizeof(CHUNKHEADER), ios::cur);
	else
	{
	    if(!frs.readK(&subchunks, sizeof(u16)))
		return SQERR_BADFILE;
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
	    return SQERR_BADFILE;

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
		    return SQERR_BADFILE;
//		prs32f("COLOR_64 packets: %d\n", packets);

		for(s32 i = 0;i < packets;i++)
		{
		    if(!frs.readK(&skip, 1)) return SQERR_BADFILE;
		    if(!frs.readK(&count, 1)) return SQERR_BADFILE;
//		    prs32f("COLOR64 skip: %d, count: %d\n", skip, count);

		    if(count)
		    {
			for(s32 j = 0;j < count;j++)
			{
			    if(!frs.readK(&e, sizeof(RGB))) return SQERR_BADFILE;
//			    prs32f("COLOR_64 PALLETTE CHANGE %d,%d,%d\n", e.r, e.g, e.b);
			}
		    }
		    else
		    {
//			prs32f("Reading pallette...\n");
			if(!frs.readK(pal, sizeof(RGB) * 256)) return SQERR_BADFILE;

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

		for(s32 j = 0;j < finfo.image[currentImage].h;j++)
		{
		    s32 index = 0;
		    count = 0;
		    if(!frs.readK(&c, 1)) return SQERR_BADFILE;

		    while(count < finfo.image[currentImage].w)
		    {
			if(!frs.readK(&c, 1)) return SQERR_BADFILE;

			if(c < 0)
			{
			    c = -c;

			    for(s32 i = 0;i < c;i++)
			    {
				if(!frs.readK(&value, 1)) return SQERR_BADFILE;
				buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
			else
			{
			    if(!frs.readK(&value, 1)) return SQERR_BADFILE;

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

		if(!frs.readK(&starty, 2)) return SQERR_BADFILE;
		if(!frs.readK(&totaly, 2)) return SQERR_BADFILE;

		ally = starty + totaly;
		
//		prs32f("Y: %d, Total: %d\n", starty, totaly);

		for(s32 j = starty;j < ally;j++)
		{
		    count = 0;
		    index = 0;

		    if(!frs.readK(&packets, 1)) return SQERR_BADFILE;

		    while(count < finfo.image[currentImage].w)
		    {
			for(s32 k = 0;k < packets;k++)
			{
//			    prs32f("LINE %d\n", j);
			    if(!frs.readK(&skip, 1)) return SQERR_BADFILE;
			    if(!frs.readK(&size, 1)) return SQERR_BADFILE;

			    index += skip;
			    
//			    prs32f("SKIP: %d, SIZE: %d\n", skip, size);

			    if(size > 0)
			    {
				if(!frs.readK(buf[j]+index, size)) return SQERR_BADFILE;
			    }
			    else if(size < 0)
			    {
				size = -size;
				if(!frs.readK(&byte, 1)) return SQERR_BADFILE;
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

		for(s32 j = 0;j < finfo.image[currentImage].h;j++)
		{
		    if(!frs.readK(buf[j], finfo.image[currentImage].w)) return SQERR_BADFILE;
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

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    finfo.images++;

    stringstream s;
    
    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << "Color indexed" << "\n"
        << "RLE/DELTA_FLI\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    line = -1;

    return SQERR_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    for(s32 i = 0;i < finfo.image[currentImage].w;i++)
    {
	memcpy(scan+i, pal+buf[line][i], sizeof(RGB));
    }

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    s32 w, h, bpp;
    FLICHEADER m_flic;
    RGB m_pal[768];
    s32 m_line;
    s32 m_bytes;
    ifstreamK	m_frs;
    jmp_buf	jmp;

    if(setjmp(jmp))
    {
	m_frs.close();
	return SQERR_BADFILE;
    }

    m_frs.open(file.c_str(), ios::binary | ios::in);

    if(!m_frs.good())
        return SQERR_NOFILE;
			
    if(!m_frs.readK(&m_flic, sizeof(FLICHEADER)))
	longjmp(jmp, 1);;

    if(m_flic.FileId != 0xAF11)// && m_flic.FileId != 0xAF12)
	longjmp(jmp, 1);;

    if(m_flic.Flags != 3)
	cerr << "libSQ_read_fli: WARNING: Flags != 3" << endl;

    memset(m_pal, 0, 768);

    u8 m_buf[m_flic.Height][m_flic.Width];

    w = m_flic.Width;
    h = m_flic.Height;
    bpp = 8;

    CHUNKHEADER chunk;
    CHUNKHEADER subchunk;
    u16 subchunks;

    fstream::pos_type pos = m_frs.tellg();
//    prs32f("POS AFTER HEADER: %d\n", pos);

    while(true)
    {
	if(!skip_flood(m_frs))
	    longjmp(jmp, 1);

	if(!m_frs.readK(&chunk, sizeof(CHUNKHEADER)))
	    longjmp(jmp, 1);

//	prs32f("Read MAIN chunk: size: %d, type: %X\n", chunk.size, chunk.type);

	if(chunk.type != CHUNK_PREFIX_TYPE &&
		    chunk.type != CHUNK_SCRIPT_CHUNK && 
		    chunk.type != CHUNK_FRAME_TYPE &&
		    chunk.type != CHUNK_SEGMENT_TABLE &&
		    chunk.type != CHUNK_HUFFMAN_TABLE)
	    longjmp(jmp, 1);

	if(chunk.type != CHUNK_FRAME_TYPE)
	    m_frs.seekg(chunk.size - sizeof(CHUNKHEADER), ios::cur);
	else
	{
	    if(!m_frs.readK(&subchunks, sizeof(u16)))
		longjmp(jmp, 1);
//	    prs32f("Chunk #%X has %d subchunks\n", chunk.type, subchunks);
	    m_frs.seekg(sizeof(u16) * 4, ios::cur);
	    break;
	}
    }
    
//    cout << "GO!\n";

//    prs32f("POS MAIN: %d\n", ftell(fptr));
//    fsetpos(fptr, (fpos_t*)&pos);
//    fseek(fptr, chunk.size, SEEK_CUR);
//    prs32f("POS2 MAIN: %d\n", ftell(fptr));

    while(subchunks--)
    {
        pos = m_frs.tellg(); 

        if(!m_frs.readK(&subchunk, sizeof(CHUNKHEADER)))
	    longjmp(jmp, 1);

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

		if(!m_frs.readK(&packets, sizeof(u16)))
		    longjmp(jmp, 1);
//		prs32f("COLOR_64 packets: %d\n", packets);

		for(s32 i = 0;i < packets;i++)
		{
		    if(!m_frs.readK(&skip, 1)) longjmp(jmp, 1);
		    if(!m_frs.readK(&count, 1)) longjmp(jmp, 1);
//		    prs32f("COLOR64 skip: %d, count: %d\n", skip, count);

		    if(count)
		    {
			for(s32 j = 0;j < count;j++)
			{
			    if(!m_frs.readK(&e, sizeof(RGB))) longjmp(jmp, 1);
//			    prs32f("COLOR_64 PALLETTE CHANGE %d,%d,%d\n", e.r, e.g, e.b);
			}
		    }
		    else
		    {
//			prs32f("Reading pallette...\n");
			if(!m_frs.readK(m_pal, sizeof(RGB) * 256)) longjmp(jmp, 1);

			u8 *pp = (u8 *)m_pal;

			if(subchunk.type == CHUNK_COLOR_64)
			    for(s32 j = 0;j < 768;j++)
				pp[j] <<= 2;

//			for(s32 j = 0;j < 256;j++)
//			prs32f("COLOR_64 PALLETTE %d,%d,%d\n", m_pal[j].r, m_pal[j].g, m_pal[j].b);
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

		for(s32 j = 0;j < h;j++)
		{
		    s32 index = 0;
		    count = 0;
		    if(!m_frs.readK(&c, 1)) longjmp(jmp, 1);

		    while(count < w)
		    {
			if(!m_frs.readK(&c, 1)) longjmp(jmp, 1);

			if(c < 0)
			{
			    c = -c;

			    for(s32 i = 0;i < c;i++)
			    {
				if(!m_frs.readK(&value, 1)) longjmp(jmp, 1);
				m_buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
			else
			{
			    if(!m_frs.readK(&value, 1)) longjmp(jmp, 1);

			    for(s32 i = 0;i < c;i++)
			    {
				m_buf[j][index] = value;
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

		if(!m_frs.readK(&starty, 2)) longjmp(jmp, 1);
		if(!m_frs.readK(&totaly, 2)) longjmp(jmp, 1);

		ally = starty + totaly;
		
//		prs32f("Y: %d, Total: %d\n", starty, totaly);

		for(s32 j = starty;j < ally;j++)
		{
		    count = 0;
		    index = 0;

		    if(!m_frs.readK(&packets, 1)) longjmp(jmp, 1);

		    while(count < w)
		    {
			for(s32 k = 0;k < packets;k++)
			{
//			    prs32f("LINE %d\n", j);
			    if(!m_frs.readK(&skip, 1)) longjmp(jmp, 1);
			    if(!m_frs.readK(&size, 1)) longjmp(jmp, 1);

			    index += skip;
			    
//			    prs32f("SKIP: %d, SIZE: %d\n", skip, size);

			    if(size > 0)
			    {
				if(!m_frs.readK(m_buf[j]+index, size)) longjmp(jmp, 1);
			    }
			    else if(size < 0)
			    {
				size = -size;
				if(!m_frs.readK(&byte, 1)) longjmp(jmp, 1);
				memset(m_buf[j]+index, byte, size);
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

		for(s32 j = 0;j < h;j++)
		{
		    if(!m_frs.readK(m_buf[j], w)) longjmp(jmp, 1);
		}
	    }
	    break;

	    default:
//		prs32f("*** UNKNOWN CHUNK! SEEKING ANYWAY\n");
		m_frs.seekg(pos);
		m_frs.seekg(subchunk.size, ios::cur);
	}

//	prs32f("POS: %d\n", ftell(fptr));
//	prs32f("POS2: %d\n", ftell(fptr));
    }

    stringstream s;

    m_bytes = w * h * sizeof(RGBA);

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << "Color indexed" << "\n"
        << "RLE/DELTA_FLI\n"
	<< m_flic.NumberOfFrames << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);
    
    m_line = -1;

    for(s32 h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	m_line++;
	
	for(s32 i = 0;i < w;i++)
	{
	    memcpy(scan+i, m_pal+m_buf[m_line][i], sizeof(RGB));
	}
    }

    m_frs.close();

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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

s32 fmt_codec::fmt_writeimage(std::string, RGBA *, s32, s32, const fmt_writeoptions &)
{
    return SQERR_NOTSUPPORTED;
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *)
{}

bool fmt_codec::fmt_writable() const
{
    return false;
}

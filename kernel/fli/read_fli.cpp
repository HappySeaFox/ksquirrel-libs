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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <string.h>

#include "read_fli.h"

static const int MAX_FRAME = 256;

FILE *fptr;
int bytes, currentImage;
FLICHEADER flic;
RGB pal[768];
unsigned char **buf;
int line;

const char* fmt_version()
{
    return (const char*)"0.2.2";
}

const char* fmt_quickinfo()
{
    return (const char*)"FLI Animation";
}

const char* fmt_filter()
{
    return (const char*)"*.fli ";
}

const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,170,8,138,4,4,4,227,59,112,255,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,80,73,68,65,84,120,218,61,142,193,13,192,48,8,3,189,2,15,22,176,216,160,153,128,50,64,251,200,254,171,148,16,154,123,157,44,48,0,152,11,36,175,136,104,9,73,157,45,25,93,13,70,20,55,6,233,225,76,177,240,8,91,137,181,156,36,133,53,243,111,157,158,221,76,237,163,117,181,222,120,62,112,172,23,177,216,139,37,27,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");

    if(!fptr)
        return SQERR_NOFILE;
	
    if(!sq_fread(&flic, sizeof(FLICHEADER), 1, fptr)) return SQERR_BADFILE;

//    printf("FILE HEADER: %X\n", flic.FileId);
    
    if(flic.FileId != 0xAF11)// && flic.FileId != 0xAF12)
	return SQERR_BADFILE;

    if(flic.Flags != 3)
	trace("libSQ_read_fli: WARNING: Flags != 3");

    memset(pal, 0, 768);

    currentImage = -1;
    
    buf = (unsigned char**)calloc(flic.Height, sizeof(unsigned char*));

    if(!buf)
	return SQERR_NOMEMORY;

    for(int i = 0;i < flic.Height;i++)
    {
	buf[i] = (unsigned char*)0;
    }

    for(int i = 0;i < flic.Height;i++)
    {
	buf[i] = (unsigned char*)calloc(flic.Width, sizeof(unsigned char));
	
	if(!buf[i])
	    return SQERR_NOMEMORY;
    }

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;

    if(currentImage == flic.NumberOfFrames || currentImage == MAX_FRAME)
	return SQERR_NOTOK;

    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
	return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;
    finfo->image[currentImage].w = flic.Width;
    finfo->image[currentImage].h = flic.Height;
    finfo->image[currentImage].bpp = 8;
    finfo->image[currentImage].delay = (int)((float)flic.FrameDelay * 14.3);
    finfo->animated = (currentImage) ? true : false;

//    printf("%dx%d@%d, delay: %d\n", flic.Width, flic.Height, flic.PixelDepth, finfo->image[currentImage].delay);

    CHUNKHEADER chunk;
    CHUNKHEADER subchunk;
    unsigned short subchunks;

    long pos = ftell(fptr);
//    printf("POS AFTER HEADER: %d\n", pos);

    while(true)
    {
	if(!skip_flood(fptr))
	    return SQERR_BADFILE;

	if(!sq_fread(&chunk, sizeof(CHUNKHEADER), 1, fptr)) return SQERR_BADFILE;

//	printf("Read MAIN chunk: size: %d, type: %X\n", chunk.size, chunk.type);

	if(chunk.type != CHUNK_PREFIX_TYPE &&
		    chunk.type != CHUNK_SCRIPT_CHUNK && 
		    chunk.type != CHUNK_FRAME_TYPE &&
		    chunk.type != CHUNK_SEGMENT_TABLE &&
		    chunk.type != CHUNK_HUFFMAN_TABLE)
	    return SQERR_BADFILE;

	if(chunk.type != CHUNK_FRAME_TYPE)
	    fseek(fptr, chunk.size - sizeof(CHUNKHEADER), SEEK_CUR);
	else
	{
	    if(!sq_fread(&subchunks, sizeof(unsigned short), 1, fptr)) return SQERR_BADFILE;
//	    printf("Chunk #%X has %d subchunks\n", chunk.type, subchunks);
	    fseek(fptr, sizeof(unsigned short) * 4, SEEK_CUR);
	    break;
	}
    }

//    printf("POS MAIN: %d\n", ftell(fptr));
//    fsetpos(fptr, (fpos_t*)&pos);
//    fseek(fptr, chunk.size, SEEK_CUR);
//    printf("POS2 MAIN: %d\n", ftell(fptr));

    while(subchunks--)
    {
        pos = ftell(fptr); 

        if(!sq_fread(&subchunk, sizeof(CHUNKHEADER), 1, fptr)) return SQERR_BADFILE;

//	printf("*** Subchunk: %d\n", subchunk.type);

	switch(subchunk.type)
        {
    	    case CHUNK_COLOR_64:
    	    case CHUNK_COLOR_256:
	    {
//		printf("*** Palette64 CHUNK\n");
		unsigned char skip, count;
		unsigned short packets;
		RGB e;

		if(!sq_fread(&packets, sizeof(unsigned short), 1, fptr)) return SQERR_BADFILE;
//		printf("COLOR_64 packets: %d\n", packets);

		for(int i = 0;i < packets;i++)
		{
		    if(!sq_fread(&skip, 1, 1, fptr)) return SQERR_BADFILE;
		    if(!sq_fread(&count, 1, 1, fptr)) return SQERR_BADFILE;
//		    printf("COLOR64 skip: %d, count: %d\n", skip, count);

		    if(count)
		    {
			for(int j = 0;j < count;j++)
			{
			    if(sq_fread(&e, sizeof(RGB), 1, fptr)) return SQERR_BADFILE;
//			    printf("COLOR_64 PALLETTE CHANGE %d,%d,%d\n", e.r, e.g, e.b);
			}
		    }
		    else
		    {
//			printf("Reading pallette...\n");
			if(!sq_fread(pal, sizeof(RGB), 256, fptr)) return SQERR_BADFILE;

			unsigned char *pp = (unsigned char *)pal;

			if(subchunk.type == CHUNK_COLOR_64)
			    for(int j = 0;j < 768;j++)
				pp[j] <<= 2;

//			for(int j = 0;j < 256;j++)
//			printf("COLOR_64 PALLETTE %d,%d,%d\n", pal[j].r, pal[j].g, pal[j].b);
//			printf("\n");
		    }
		}
	    }
	    break;

	    case CHUNK_RLE:
	    {
//		printf("*** RLE DATA CHUNK\n");
		unsigned char value;
		signed char c;
		int count;

		for(int j = 0;j < finfo->image[currentImage].h;j++)
		{
		    int index = 0;
		    count = 0;
		    if(!sq_fread(&c, 1, 1, fptr)) return SQERR_BADFILE;

		    while(count < finfo->image[currentImage].w)
		    {
			if(!sq_fread(&c, 1, 1, fptr)) return SQERR_BADFILE;

			if(c < 0)
			{
			    c = -c;

			    for(int i = 0;i < c;i++)
			    {
				if(!sq_fread(&value, 1, 1, fptr)) return SQERR_BADFILE;
				buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
			else
			{
			    if(!sq_fread(&value, 1, 1, fptr)) return SQERR_BADFILE;

			    for(int i = 0;i < c;i++)
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
		unsigned short starty, totaly, ally, index;
		unsigned char packets, skip, byte;
		signed char size;
		int count;
		
		if(!sq_fread(&starty, 2, 1, fptr)) return SQERR_BADFILE;
		if(!sq_fread(&totaly, 2, 1, fptr)) return SQERR_BADFILE;

		ally = starty + totaly;
		
//		printf("Y: %d, Total: %d\n", starty, totaly);

		for(int j = starty;j < ally;j++)
		{
		    count = 0;
		    index = 0;

		    if(!sq_fread(&packets, 1, 1, fptr)) return SQERR_BADFILE;

		    while(count < finfo->image[currentImage].w)
		    {
			for(int k = 0;k < packets;k++)
			{
//			    printf("LINE %d\n", j);
			    if(!sq_fread(&skip, 1, 1, fptr)) return SQERR_BADFILE;
			    if(!sq_fread(&size, 1, 1, fptr)) return SQERR_BADFILE;

			    index += skip;
			    
//			    printf("SKIP: %d, SIZE: %d\n", skip, size);

			    if(size > 0)
			    {
				if(!sq_fread(buf[j]+index, 1, size, fptr)) return SQERR_BADFILE;
			    }
			    else if(size < 0)
			    {
				size = -size;
				if(!sq_fread(&byte, 1, 1, fptr)) return SQERR_BADFILE;
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
//		printf("*** COPY DATA CHUNK\n");

		for(int j = 0;j < finfo->image[currentImage].h;j++)
		{
		    if(!sq_fread(buf[j], finfo->image[currentImage].w, 1, fptr)) return SQERR_BADFILE;
		}
	    }
	    break;

	    default:
//		printf("*** UNKNOWN CHUNK! SEEKING ANYWAY\n");
		fsetpos(fptr, (fpos_t*)&pos);
		fseek(fptr, subchunk.size, SEEK_CUR);
	}

//	printf("POS: %d\n", ftell(fptr));
//	printf("POS2: %d\n", ftell(fptr));
    }

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    char type[25];
    strcpy(type, "Color indexed");

    finfo->images++;

    /* Write dump */
    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\nRLE/DELTA_FLI\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	type,
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    line = -1;

    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    line++;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    for(int i = 0;i < finfo->image[currentImage].w;i++)
    {
	memcpy(scan+i, pal+buf[line][i], sizeof(RGB));
    }

    return SQERR_OK;
}


int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    FILE *m_fptr;
    int w, h, bpp;
    FLICHEADER m_flic;
    RGB m_pal[768];
    int m_line;
    int m_bytes;

    m_fptr = fopen(file, "rb");
				        
    if(!m_fptr)
        return SQERR_NOFILE;

    if(!sq_fread(&m_flic, sizeof(FLICHEADER), 1, m_fptr)) return SQERR_BADFILE;
//    printf("FILE HEADER: %X\n", m_flic.FileId);

    if(m_flic.FileId != 0xAF11)// && m_flic.FileId != 0xAF12)
	return SQERR_BADFILE;

    if(m_flic.Flags != 3)
	trace("libSQ_read_fli: WARNING: Flags != 3");

    memset(m_pal, 0, 768);

    currentImage = -1;
/*    
    m_buf = (unsigned char**)calloc(m_flic.Height, sizeof(unsigned char*));

    if(!m_buf)
	return SQERR_NOMEMORY;
    
    for(int i = 0;i < m_flic.Height;i++)
    {
	m_buf[i] = (unsigned char*)calloc(m_flic.Width, sizeof(unsigned char));

	if(!m_buf[i])
	{
	    for(int k = 0;k < i;k++)
		free(m_buf[k]);
		
	    free(m_buf);

	    return SQERR_NOMEMORY;
	}
    }
*/
    unsigned char m_buf[m_flic.Height][m_flic.Width];

    w = m_flic.Width;
    h = m_flic.Height;
    bpp = 8;

    CHUNKHEADER chunk;
    CHUNKHEADER subchunk;
    unsigned short subchunks;

    long pos = ftell(m_fptr);
//    printf("POS AFTER HEADER: %d\n", pos);

    while(true)
    {
	if(!skip_flood(m_fptr))
	{
	    for(int k = 0;k < h;k++)
		free(m_buf[k]);

	    free(m_buf);
	    
	    fclose(m_fptr);

	    return SQERR_BADFILE;
	}

	if(!sq_fread(&chunk, sizeof(CHUNKHEADER), 1, m_fptr)) return SQERR_BADFILE;
//	printf("Read MAIN chunk: size: %d, type: %X\n", chunk.size, chunk.type);

	if(chunk.type != CHUNK_PREFIX_TYPE &&
		    chunk.type != CHUNK_SCRIPT_CHUNK && 
		    chunk.type != CHUNK_FRAME_TYPE &&
		    chunk.type != CHUNK_SEGMENT_TABLE &&
		    chunk.type != CHUNK_HUFFMAN_TABLE)
	{
	    for(int k = 0;k < h;k++)
		free(m_buf[k]);

	    free(m_buf);
	    
	    fclose(m_fptr);

	    return SQERR_BADFILE;
	}

	if(chunk.type != CHUNK_FRAME_TYPE)
	    fseek(m_fptr, chunk.size - sizeof(CHUNKHEADER), SEEK_CUR);
	else
	{
	    if(!sq_fread(&subchunks, sizeof(unsigned short), 1, m_fptr)) return SQERR_BADFILE;
//	    printf("Chunk #%X has %d subchunks\n", chunk.type, subchunks);
	    fseek(m_fptr, sizeof(unsigned short) * 4, SEEK_CUR);
	    break;
	}
    }

//    printf("POS MAIN: %d\n", ftell(m_fptr));
//    fsetpos(m_fptr, (fpos_t*)&pos);
//    fseek(m_fptr, chunk.size, SEEK_CUR);
//    printf("POS2 MAIN: %d\n", ftell(m_fptr));

    while(subchunks--)
    {
        pos = ftell(m_fptr); 

        if(!sq_fread(&subchunk, sizeof(CHUNKHEADER), 1, m_fptr)) return SQERR_BADFILE;
//	printf("*** Subchunk: %d\n", subchunk.type);

	switch(subchunk.type)
        {
    	    case CHUNK_COLOR_64:
    	    case CHUNK_COLOR_256:
	    {
//		printf("*** Palette64 CHUNK\n");
		unsigned char skip, count;
		unsigned short packets;
		RGB e;

		if(!sq_fread(&packets, sizeof(unsigned short), 1, m_fptr)) return SQERR_BADFILE;
//		printf("COLOR_64 packets: %d\n", packets);

		for(int i = 0;i < packets;i++)
		{
		    if(!sq_fread(&skip, 1, 1, m_fptr)) return SQERR_BADFILE;
		    if(!sq_fread(&count, 1, 1, m_fptr)) return SQERR_BADFILE;
//		    printf("COLOR64 skip: %d, count: %d\n", skip, count);

		    if(count)
		    {
			for(int j = 0;j < count;j++)
			{
			    if(!sq_fread(&e, sizeof(RGB), 1, m_fptr)) return SQERR_BADFILE;

//			    printf("COLOR_64 PALLETTE CHANGE %d,%d,%d\n", e.r, e.g, e.b);
			}
		    }
		    else
		    {
//			printf("Reading pallette...\n");
			if(!sq_fread(m_pal, sizeof(RGB), 256, m_fptr)) return SQERR_BADFILE;

			unsigned char *pp = (unsigned char *)m_pal;

			if(subchunk.type == CHUNK_COLOR_64)
			    for(int j = 0;j < 768;j++)
				pp[j] <<= 2;

//			for(int j = 0;j < 256;j++)
//			printf("COLOR_64 PALLETTE %d,%d,%d\n", m_pal[j].r, m_pal[j].g, m_pal[j].b);
//			printf("\n");
		    }
		}
	    }
	    break;

	    case CHUNK_RLE:
	    {
//		printf("*** RLE DATA CHUNK\n");
		unsigned char value;
		signed char c;
		int count;

		for(int j = 0;j < h;j++)
		{
		    int index = 0;
		    count = 0;
		    if(!sq_fread(&c, 1, 1, m_fptr)) return SQERR_BADFILE;

		    while(count < w)
		    {
			if(!sq_fread(&c, 1, 1, m_fptr)) return SQERR_BADFILE;

			if(c < 0)
			{
			    c = -c;

			    for(int i = 0;i < c;i++)
			    {
				if(!sq_fread(&value, 1, 1, m_fptr)) return SQERR_BADFILE;

				m_buf[j][index] = value;
				index++;
			    }

			    count += c;
			}
			else
			{
			    if(!sq_fread(&value, 1, 1, m_fptr)) return SQERR_BADFILE;

			    for(int i = 0;i < c;i++)
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
		unsigned short starty, totaly, ally, index;
		unsigned char packets, skip, byte;
		signed char size;
		int count;
		
		if(!sq_fread(&starty, 2, 1, m_fptr)) return SQERR_BADFILE;
		if(!sq_fread(&totaly, 2, 1, m_fptr)) return SQERR_BADFILE;

		ally = starty + totaly;
		
//		printf("Y: %d, Total: %d\n", starty, totaly);

		for(int j = starty;j < ally;j++)
		{
		    count = 0;
		    index = 0;

		    if(!sq_fread(&packets, 1, 1, m_fptr)) return SQERR_BADFILE;

		    while(count < w)
		    {
			for(int k = 0;k < packets;k++)
			{
//			    printf("LINE %d\n", j);
			    if(!sq_fread(&skip, 1, 1, m_fptr)) return SQERR_BADFILE;
			    if(!sq_fread(&size, 1, 1, m_fptr)) return SQERR_BADFILE;

			    index += skip;
			    
//			    printf("SKIP: %d, SIZE: %d\n", skip, size);

			    if(size > 0)
			    {
				if(!sq_fread(m_buf[j]+index, 1, size, m_fptr)) return SQERR_BADFILE;
			    }
			    else if(size < 0)
			    {
				size = -size;
				if(!sq_fread(&byte, 1, 1, m_fptr)) return SQERR_BADFILE;

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
//		printf("*** COPY DATA CHUNK\n");

		for(int j = 0;j < h;j++)
		{
		    if(!sq_fread(m_buf[j], w, 1, m_fptr)) return SQERR_BADFILE;
		}
	    }
	    break;

	    default:
//		printf("*** UNKNOWN CHUNK! SEEKING ANYWAY\n");
		fsetpos(m_fptr, (fpos_t*)&pos);
		fseek(m_fptr, subchunk.size, SEEK_CUR);
	}
    }

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);
						
    if(!*image)
    {
        fprintf(stderr, "libSQ_read_fli: Image is null!\n");

        fclose(m_fptr);

	for(int k = 0;k < h;k++)
	    free(m_buf[k]);

	free(m_buf);

        return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);
    
    m_line = -1;

    for(int h2 = 0;h2 < h;h2++)
    {
	RGBA 	*scan = *image + h2 * w;

	m_line++;

	memset(scan, 255, w * sizeof(RGBA));

	for(int i = 0;i < w;i++)
	{
	    memcpy(scan+i, m_pal+m_buf[m_line][i], sizeof(RGB));
	}
    }

    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
    
    if(buf)
    {
	for(int i = 0;i < flic.Height;i++)
	    if(buf[i])
		free(buf[i]);
		
	free(buf);
    }
}

bool find_chunk_type(const unsigned short type)
{
    static const unsigned short A[] = 
    {
	CHUNK_PREFIX_TYPE,
	CHUNK_SCRIPT_CHUNK,
	CHUNK_FRAME_TYPE,
	CHUNK_SEGMENT_TABLE,
	CHUNK_HUFFMAN_TABLE
    };
    
    static const int S = 5;

    for(int i = 0;i < S;i++)
	if(type == A[i])
	    return true;
	    
    return false;
}

bool skip_flood(FILE *f)
{
    unsigned char _f[4];
    unsigned short b;
    long _pos;
    
//    printf("SKIP_FLOOD pos: %d\n", ftell(f));

    if(!sq_fread(_f, 4, 1, f)) return false;
    
    do
    {
	_pos = ftell(f);
	if(!sq_fread(&b, 2, 1, f)) return false;
//	printf("SKIP_FLOOD b: %X\n", b);
	fseek(f, -1, SEEK_CUR);
    }while(!find_chunk_type(b));

    _pos -= 4;

    fsetpos(f, (fpos_t*)&_pos);
    
    return true;

//    printf("SKIP_FLOOD pos2: %d\n", ftell(f));
}

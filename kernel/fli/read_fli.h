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

#ifndef KSQUIRREL_READ_IMAGE_fli
#define KSQUIRREL_READ_IMAGE_fli

#include "defs.h"
#include "err.h"
#include "fio.h"

struct FLICHEADER
{
    unsigned int	FileSize;       /* Total size of file */
    unsigned short	FileId;         /* File format indicator */
    unsigned short	NumberOfFrames; /* Total number of frames */
    unsigned short	Width;          /* Screen width in pixels */
    unsigned short	Height;         /* Screen height in pixels */
    unsigned short	PixelDepth;     /* Number of bits per pixel */
    unsigned short	Flags;          /* Set to 03h */
    unsigned int	FrameDelay;     /* Time delay between frames */
    unsigned short	Reserved1;      /* Not used (Set to 00h) */

// The following fields are set to 00h in a .FLI file
    unsigned int	DateCreated;    /* Time/Date the file was created */
    unsigned int	CreatorSN;      /* Serial number of creator program */
    unsigned int	LastUpdated;    /* Time/Date the file last changed */
    unsigned int	UpdaterSN;      /* Serial number of updater program */
    unsigned short	XAspect;        /* X-axis of display aspect ratio */
    unsigned short	YAspect;        /* Y-axis of display aspect ratio */
    unsigned char	Reserved2[38];  /* Not used (Set to 00h) */
    unsigned int	Frame1Offset;   /* Offset of first frame */
    unsigned int	Frame2Offset;   /* Offset of second frame */
    unsigned char	Reserved3[40];  /* Not used (Set to 00h) */

}ATTR_;

struct CHUNKHEADER
{
    unsigned int	size;        /* Total size of chunk */
    unsigned short	type;        /* Chunk identifier */
//    unsigned short	subchunks;   /* Number of subchunks in this chunk */
//    unsigned char	res[8];      /* Not used (Set to 00h) */

}ATTR_;

#define CHUNK_CEL_DATA	3
#define CHUNK_COLOR_256	4
#define CHUNK_DELTA_FLC	7
#define CHUNK_COLOR_64	11
#define CHUNK_DELTA_FLI	12
#define CHUNK_BLACK	13
#define CHUNK_RLE	15
#define CHUNK_COPY	16
#define CHUNK_PSTAMP	18
#define CHUNK_DTA_BRUN	25
#define CHUNK_DTA_COPY	26
#define CHUNK_DTA_LC	27
#define CHUNK_LABEL	31
#define CHUNK_BMP_MASK	32
#define CHUNK_MLEV_MASK	33
#define CHUNK_SEGMENT	34
#define CHUNK_KEY_IMAGE	35
#define CHUNK_KEY_PAL	36
#define CHUNK_REGION 	37
#define CHUNK_WAVE	38
#define CHUNK_USERSTR	39
#define CHUNK_RGN_MASK	40
#define CHUNK_LABELEX	41
#define CHUNK_SHIFT	42
#define CHUNK_PATHMAP	43

#define CHUNK_PREFIX_TYPE	0xF100
#define CHUNK_SCRIPT_CHUNK	0xF1E0
#define CHUNK_FRAME_TYPE	0xF1FA
#define CHUNK_SEGMENT_TABLE	0xF1FB
#define CHUNK_HUFFMAN_TABLE	0xF1FC

bool skip_flood(FILE *f);

extern "C" {

const char*	fmt_version();
const char*	fmt_quickinfo();
const char*	fmt_filter();
const char*	fmt_mime();
const char*	fmt_pixmap();

int 	fmt_init(fmt_info *finfo, const char *file);
int	fmt_read_scanline(fmt_info *finfo, RGBA *scan);
int	fmt_readimage(const char*, RGBA **scan, char *dump);
void	fmt_close();

int    fmt_next(fmt_info *finfo);
int    fmt_next_pass(fmt_info *finfo);

}

#endif

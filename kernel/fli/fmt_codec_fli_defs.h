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

#ifndef KSQUIRREL_READ_IMAGE_fli
#define KSQUIRREL_READ_IMAGE_fli

struct FLICHEADER
{
   u32	FileSize;       /* Total size of file */
    u16	FileId;         /* File format indicator */
    u16	NumberOfFrames; /* Total number of frames */
    u16	Width;          /* Screen width in pixels */
    u16	Height;         /* Screen height in pixels */
    u16	PixelDepth;     /* Number of bits per pixel */
    u16	Flags;          /* Set to 03h */
   u32	FrameDelay;     /* Time delay between frames */
    u16	Reserved1;      /* Not used (Set to 00h) */

// The following fields are set to 00h in a .FLI file
   u32	DateCreated;    /* Time/Date the file was created */
   u32	CreatorSN;      /* Serial number of creator program */
   u32	LastUpdated;    /* Time/Date the file last changed */
   u32	UpdaterSN;      /* Serial number of updater program */
    u16	XAspect;        /* X-axis of display aspect ratio */
    u16	YAspect;        /* Y-axis of display aspect ratio */
    u8	Reserved2[38];  /* Not used (Set to 00h) */
   u32	Frame1Offset;   /* Offset of first frame */
   u32	Frame2Offset;   /* Offset of second frame */
    u8	Reserved3[40];  /* Not used (Set to 00h) */

}PACKED;

struct CHUNKHEADER
{
   u32	size;        /* Total size of chunk */
    u16	type;        /* Chunk identifier */
//    u16	subchunks;   /* Number of subchunks in this chunk */
//    u8	res[8];      /* Not used (Set to 00h) */

}PACKED;

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

#endif

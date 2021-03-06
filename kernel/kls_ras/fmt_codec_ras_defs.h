/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>

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

#ifndef KSQUIRREL_READ_IMAGE_ras
#define KSQUIRREL_READ_IMAGE_ras

struct RAS_HEADER
{
    u32  ras_magic;
    u32  ras_width;
    u32  ras_height;
    u32  ras_depth;
    u32  ras_length;
    u32  ras_type;
    u32  ras_maptype;
    u32  ras_maplength;

}PACKED;

#define RAS_MAGIC 0x59A66A95 // Magic number for Sun rasterfiles

#define RAS_OLD		0	// Old format (raw image in 68000 byte order)
#define RAS_STANDARD	1	// Raw image in 68000 byte order
#define RAS_BYTE_ENCODED	2	// Run-length encoding of bytes
#define RAS_RGB		3	// XRGB or RGB instead of XBGR or BGR
#define RAS_TIFF		4	// TIFF <-> standard rasterfile
#define RAS_IFF		5	// IFF (TAAC format) <-> standard rasterfile

#define RAS_EXPERIMENTAL 0xffff	// Reserved for testing

#define RMT_NONE	0	// maplength is expected to be 0
#define RMT_EQUAL_RGB	1	// red[maplength/3], green[maplength/3], blue[maplength/3]
#define RMT_RAW		2	// Raw colormap
#define RESC		128 	// Run-length encoding escape character

#endif

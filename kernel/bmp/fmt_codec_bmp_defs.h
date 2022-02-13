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

#ifndef KSQIURREL_READ_IMAGE_bmp
#define KSQUIRREL_READ_IMAGE_bmp

/*  Compression type  */
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L

#define BMP_IDENTIFIER	0x4D42

struct BITMAPFILE_HEADER
{
    u16	Type; /*  "BM"  */
    u32 	Size;
    u32	Reserved1;
    u32 	OffBits;

}PACKED;

struct BITMAPINFO_HEADER
{
    u32	Size;
    u32	Width;
    u32	Height;
    u16	Planes;
    u16	BitCount;
    u32	Compression;
    u32	SizeImage;
    u32	XPelsPerMeter;
    u32	YPelsPerMeter;
    u32	ClrUsed;
    u32	ClrImportant;

}PACKED;

#endif

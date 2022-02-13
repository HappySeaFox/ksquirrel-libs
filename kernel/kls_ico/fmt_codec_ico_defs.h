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

#ifndef KSQUIRREL_READ_IMAGE_ico
#define KSQUIRREL_READ_IMAGE_ico

struct ICO_DIRENTRY
{
    u8	bWidth;
    u8	bHeight;
    u8	bColorCount;
    u8	bReserved; /*  0  */
    u16	wPlanes;   /*  0  */
    u16	wBitCount; /*  0  */
    u32	dwBytes;
    u32	dwImageOffset;

}PACKED;


struct ICO_HEADER
{
    u16	idReserved;
    u16	idType;  /*  must be  1  */
    u16	idCount;

}PACKED;

struct BITMAPINFO_HEADER
{
    u32	Size;
    u32	Width;
    u32	Height;
    u16	Planes;
    u16	BitCount;
    u32	Compression; /*  not used -->>  */
    u32	SizeImage;
    u32	XPelsPerMeter;
    u32	YPelsPerMeter;
    u32	ClrUsed;
    u32	ClrImportant;

}PACKED;

#endif

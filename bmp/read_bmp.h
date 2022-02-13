/*  This file is part of SQuirrel (http://ksquirrel.sf.net) libraries

    Copyright (c) 2004 Dmitry Baryshev <ckult@yandex.ru>

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

#ifndef _SQIURREL_READ_IMAGE_bmp
#define _SQUIRREL_READ_IMAGE_bmp

#include "../defs.h"
#include "../err.h"


#include <sys/mman.h>

/*  Compression type  */
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L


typedef struct
{
    unsigned short	Type; /*  "BM"  */
    unsigned long 	Size;
    unsigned long	Reserved1;
//    unsigned short	Reserved2;
    unsigned long 	OffBits;

}ATTR_ BITMAPFILE_HEADER;

typedef struct
{
    unsigned long	Size;
    unsigned long	Width;
    unsigned long	Height;
    unsigned short	Planes;
    unsigned short	BitCount;
    unsigned long	Compression;
    unsigned long	SizeImage;
    unsigned long	XPelsPerMeter;
    unsigned long	YPelsPerMeter;
    unsigned long	ClrUsed;
    unsigned long	ClrImportant;

}ATTR_ BITMAPINFO_HEADER;

#endif

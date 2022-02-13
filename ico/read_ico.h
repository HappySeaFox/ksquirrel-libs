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

#ifndef _SQUIRREL_READ_IMAGE_ico
#define _SQUIRREL_READ_IMAGE_ico

#include "../defs.h"
#include "../err.h"

typedef struct
{
    unsigned char	bWidth;
    unsigned char	bHeight;
    unsigned char	bColorCount;
    unsigned char	bReserved; /*  0  */
    unsigned short	wPlanes;   /*  0  */
    unsigned short	wBitCount; /*  0  */
    unsigned long	dwBytes;
    unsigned long	dwImageOffset;
}ATTR_ ICO_DIRENTRY;


typedef struct
{
    unsigned short	idReserved;
    unsigned short	idType;  /*  must be  1  */
    unsigned short	idCount;
}ATTR_ ICO_HEADER;

// ICO_DIRENTRY*

typedef struct
{
    unsigned long	Size;
    unsigned long	Width;
    unsigned long	Height;
    unsigned short	Planes;
    unsigned short	BitCount;
    unsigned long	Compression; /*  not used -->>  */
    unsigned long	SizeImage;
    unsigned long	XPelsPerMeter;
    unsigned long	YPelsPerMeter;
    unsigned long	ClrUsed;
    unsigned long	ClrImportant;

}ATTR_ BITMAPINFO_HEADER;

#endif

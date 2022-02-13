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

#ifndef _SQUIRREL_READ_IMAGE_pcx
#define _SQUIRREL_READ_IMAGE_pcx

#include "../defs.h"
#include "../err.h"

typedef struct
{
	unsigned char	ID;
	unsigned char	Version;
	unsigned char	Encoding;
	unsigned char	bpp;
	unsigned short	Xmin,Ymin,Xmax,Ymax;
	unsigned short	VDpi;
	unsigned short	HDpi;
	unsigned char	Palette[48];
	unsigned char	Reserved;
	unsigned char	NPlanes;
	unsigned short	BytesPerLine;
	unsigned short	PaletteInfo;
	unsigned short	HScreenSize;
	unsigned short	VScreenSize;
	unsigned char	Filler[54];       /*        Header should be 128 byte  length  */

}PCX_HEADER;

#endif

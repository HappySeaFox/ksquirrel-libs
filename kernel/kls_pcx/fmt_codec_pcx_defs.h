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

#ifndef KSQUIRREL_READ_IMAGE_pcx
#define KSQUIRREL_READ_IMAGE_pcx

#define PCX_COLORMAP_SIGNATURE          (0x0c)
#define PCX_COLORMAP_SIGNATURE_NEW      (0x0a)

struct PCX_HEADER
{
	u8	ID;
	u8	Version;
	u8	Encoding;
	u8	bpp;
	u16	Xmin,Ymin,Xmax,Ymax;
	u16	VDpi;
	u16	HDpi;
	u8	Palette[48];
	u8	Reserved;
	u8	NPlanes;
	u16	BytesPerLine;
	u16	PaletteInfo;
	u16	HScreenSize;
	u16	VScreenSize;
	u8	Filler[54];       /*        Header should be 128 byte  length  */

}PACKED;

#endif

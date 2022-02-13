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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQIURREL_READ_IMAGE_bmp
#define KSQUIRREL_READ_IMAGE_bmp

#include "defs.h"
#include "err.h"

/*  Compression type  */
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L


struct BITMAPFILE_HEADER
{
    unsigned short	Type; /*  "BM"  */
    unsigned long 	Size;
    unsigned long	Reserved1;
//    unsigned short	Reserved2;
    unsigned long 	OffBits;

}ATTR_;

struct BITMAPINFO_HEADER
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

}ATTR_;

extern "C" {

const char*     fmt_version();
const char*     fmt_quickinfo();
const char*     fmt_filter();
const char*     fmt_mime();
const char*     fmt_pixmap();

int     fmt_init(fmt_info *finfo, const char *file);
int     fmt_read_scanline(fmt_info *finfo, RGBA *scan);
int     fmt_readimage(const char*, RGBA **scan, char **);
int     fmt_close();

int     fmt_next(fmt_info *finfo);
int     fmt_next_pass(fmt_info *finfo);

}

#endif

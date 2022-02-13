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

#ifndef KSQUIRREL_CODEC_DEFS_sct
#define KSQUIRREL_CODEC_DEFS_sct

#define SCT_UNITS_MM   0
#define SCT_UNITS_INCH 1

#define SCT_FORMAT_RGB 7
#define SCT_FORMAT_GRAY 8
#define SCT_FORMAT_CMYK 0xf

//
// SCT header starts from offset 0x400. Before it
// comes comment string (50 symbols, offset 0x0)
// and signature "CT" (offset 0x50).
//
// Image pixels are stored from file offset 0x800
//

struct sct_header
{
    u8	units;         // units (0=MM,1=INCH)
    u8	channels;      // number of channels
    u16	format;        // format (7=RGB, 8=GREYSCALE, 0xF=CMYK)
    s8	wh_units[28];  // width and height in units stored as a Scitex FP (not used)
    s8	width[12];     // width  in pixels stored as 12 digits of text including sign ("%+012d").
    s8	height[12];    // height  in pixels stored as 12 digits of text including sign ("%+012d").

}PACKED;

#endif

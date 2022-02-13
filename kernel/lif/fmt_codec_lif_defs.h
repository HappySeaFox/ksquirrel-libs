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

#ifndef KSQUIRREL_CODEC_DEFS_lif
#define KSQUIRREL_CODEC_DEFS_lif

#define LIF_VERSION 260
#define LIF_FLAGS   50
#define LIF_ID      "Willy 7"

struct lif_header
{
    s8       id[8];          // "Willy 7"
    u32      version;        // Version Number (260)
    u32      flags;          // Usually 50
    u32      width;
    u32      height;
    u32      paletteCRC;     // CRC of palettes for fast comparison.
    u32      imageCRC;       // CRC of the image.
    u32      palOffset;      // Offset to the palette (not used).
    u32      teamEffect0;    // Team effect offset 0
    u32      teamEffect1;    // Team effect offset 1

}PACKED;

#endif

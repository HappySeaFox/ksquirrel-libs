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
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQUIRREL_READ_IMAGE_iff
#define KSQUIRREL_READ_IMAGE_iff

struct CHUNK_HEAD
{
    u32 type;
    u32 size;

}PACKED;

struct CHUNK_BMHD
{
    u16  Width;        // Width of image in pixels
    u16  Height;       // Height of image in pixels
    u16  Left;         // X coordinate of image
    u16  Top;          // Y coordinate of image
    u8   Bitplanes;    // Number of bitplanes
    u8   Masking;      // Type of masking used
    u8   Compress;     // Compression method use on image data
    u8   Padding;      // Alignment padding (always 0)
    u16  Transparency; // Transparent background color
    u8   XAspectRatio; // Horizontal pixel size
    u8   YAspectRatio; // Vertical pixel size
    u16  PageWidth;    // Horizontal resolution of display device
    u16  PageHeight;   // Vertical resolution of display device

}PACKED;

/*
 *
 * taken from FreeImage's IFF plugin
 *
 */

typedef u32 IFF_ID;

#define MAKE_ID(d, c, b, a)         ((IFF_ID)(a)<<24 | (IFF_ID)(b)<<16 | (IFF_ID)(c)<<8 | (IFF_ID)(d))

#define IFF_FORM     MAKE_ID('F', 'O', 'R', 'M')     /* EA IFF 85 group identifier */
#define IFF_CAT      MAKE_ID('C', 'A', 'T', ' ')     /* EA IFF 85 group identifier */
#define IFF_LIST     MAKE_ID('L', 'I', 'S', 'T')     /* EA IFF 85 group identifier */
#define IFF_PROP     MAKE_ID('P', 'R', 'O', 'P')     /* EA IFF 85 group identifier */
#define IFF_END      MAKE_ID('E', 'N', 'D', ' ')     /* unofficial END-of-FORM identifier (see Amiga RKM Devices Ed.3 page 376) */

#define IFF_ILBM     MAKE_ID('I', 'L', 'B', 'M')     /* EA IFF 85 raster bitmap form */
#define IFF_DEEP     MAKE_ID('D', 'E', 'E', 'P')     /* Chunky pixel image files (Used in TV Paint) */
#define IFF_RGB8     MAKE_ID('R', 'G', 'B', '8')     /* RGB image forms, Turbo Silver (Impulse) */
#define IFF_RGBN     MAKE_ID('R', 'G', 'B', 'N')     /* RGB image forms, Turbo Silver (Impulse) */
#define IFF_PBM      MAKE_ID('P', 'B', 'M', ' ')     /* 256-color chunky format (DPas32 2 ?) */
#define IFF_ACBM     MAKE_ID('A', 'C', 'B', 'M')     /* Amiga Contiguous Bitmap (AmigaBasic) */

/* generic */
#define IFF_FVER     MAKE_ID('F', 'V', 'E', 'R')     /* AmigaOS version string */
#define IFF_JUNK     MAKE_ID('J', 'U', 'N', 'K')     /* always ignore this chunk */
#define IFF_ANNO     MAKE_ID('A', 'N', 'N', 'O')     /* EA IFF 85 Generic Annotation chunk */
#define IFF_AUTH     MAKE_ID('A', 'U', 'T', 'H')     /* EA IFF 85 Generic Author chunk */
#define IFF_CHRS     MAKE_ID('C', 'H', 'R', 'S')     /* EA IFF 85 Generic character string chunk */
#define IFF_NAME     MAKE_ID('N', 'A', 'M', 'E')     /* EA IFF 85 Generic Name of art, music, etc. chunk */
#define IFF_TEXT     MAKE_ID('T', 'E', 'X', 'T')     /* EA IFF 85 Generic unformatted ASCII text chunk */
#define IFF_copy     MAKE_ID('(', 'c', ')', ' ')     /* EA IFF 85 Generic Copyright text chunk */

/* ILBM chunks */
#define IFF_BMHD     MAKE_ID('B', 'M', 'H', 'D')     /* ILBM BitmapHeader */
#define IFF_CMAP     MAKE_ID('C', 'M', 'A', 'P')     /* ILBM 8bit RGB colormap */
#define IFF_GRAB     MAKE_ID('G', 'R', 'A', 'B')     /* ILBM "hotspot" coordiantes */
#define IFF_DEST     MAKE_ID('D', 'E', 'S', 'T')     /* ILBM destination image info */
#define IFF_SPRT     MAKE_ID('S', 'P', 'R', 'T')     /* ILBM sprite identifier */
#define IFF_CAMG     MAKE_ID('C', 'A', 'M', 'G')     /* Amiga viewportmodes */
#define IFF_BODY     MAKE_ID('B', 'O', 'D', 'Y')     /* ILBM image data */
#define IFF_CRNG     MAKE_ID('C', 'R', 'N', 'G')     /* color cycling */
#define IFF_CCRT     MAKE_ID('C', 'C', 'R', 'T')     /* color cycling */
#define IFF_CLUT     MAKE_ID('C', 'L', 'U', 'T')     /* Color Lookup Table chunk */
#define IFF_DPI      MAKE_ID('D', 'P', 'I', ' ')     /* Dots per inch chunk */
#define IFF_DPPV     MAKE_ID('D', 'P', 'P', 'V')     /* DPas32 perspective chunk (EA) */
#define IFF_DRNG     MAKE_ID('D', 'R', 'N', 'G')     /* DPas32 IV enhanced color cycle chunk (EA) */
#define IFF_EPSF     MAKE_ID('E', 'P', 'S', 'F')     /* Encapsulated Postscript chunk */
#define IFF_CMYK     MAKE_ID('C', 'M', 'Y', 'K')     /* Cyan, Magenta, Yellow, & Black color map (Soft-Logik) */
#define IFF_CNAM     MAKE_ID('C', 'N', 'A', 'M')     /* Color naming chunk (Soft-Logik) */
#define IFF_PCHG     MAKE_ID('P', 'C', 'H', 'G')     /* Line by line palette control information (Sebastiano Vigna) */
#define IFF_PRVW     MAKE_ID('P', 'R', 'V', 'W')     /* A mini duplicate ILBM used for preview (Gary Bonham) */
#define IFF_XBMI     MAKE_ID('X', 'B', 'M', 'I')     /* eXtended BitMap Information (Soft-Logik) */
#define IFF_CTBL     MAKE_ID('C', 'T', 'B', 'L')     /* Newtek Dynamic Ham color chunk */
#define IFF_DYCP     MAKE_ID('D', 'Y', 'C', 'P')     /* Newtek Dynamic Ham chunk */
#define IFF_SHAM     MAKE_ID('S', 'H', 'A', 'M')     /* Sliced HAM color chunk */

#endif

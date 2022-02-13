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

#ifndef KSQUIRREL_READ_IMAGE_utah
#define KSQUIRREL_READ_IMAGE_utah

#define UTAH_MAGIC 0xCC52

#define     USkipLinesOp        1
#define     USetColorOp         2
#define     USkipPixelsOp       3
#define     UByteDataOp         5
#define     URunDataOp          6
#define     UEOFOp              7

#define     H_CLEARFIRST        0x1   /*  clear framebuffer flag              */
#define     H_NO_BACKGROUND     0x2   /*  if set, no bg color supplied        */
#define     H_ALPHA             0x4   /*  if set, alpha channel (-1) present  */
#define     H_COMMENT           0x8   /*  if set, comments present            */

struct UTAH_HEADER
{
    u16 magic;
    u16 xpos;
    u16 ypos;
    u16 xsize;
    u16 ysize;
    u8  flags;
    u8  ncolors;
    u8  pixelbytes;
    u8  ncmap;
    u8  cmaplen;
    u8  red;
    u8  green;
    u8  blue;

}PACKED;

#endif

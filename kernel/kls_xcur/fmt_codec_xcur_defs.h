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

#ifndef KSQUIRREL_READ_IMAGE_xcur
#define KSQUIRREL_READ_IMAGE_xcur

struct XCUR_CHUNK_DESC
{
    u32 type;
    u32 subtype;
    u32 pos;

}PACKED;

struct XCUR_HEADER
{
    u32 magic;
    u32 header;
    u32 version;
    u32 ntoc;

}PACKED;
/* Array of XCUR_CHUNK_DESC comes here */

struct XCUR_CHUNK_HEADER
{
    u32 header;
    u32 type;
    u32 subtype;
    u32 version;

}PACKED;

struct XCUR_COMMENT_HEADER
{
//    u32 version;
    u32 length;
//    s8 	 *text;

}PACKED;

struct XCUR_CHUNK_IMAGE
{
//    u32 version;
//    u32 size;
    u32 width;
    u32 height;
    u32 xhot;
    u32 yhot;
    u32 delay;

}PACKED;


#define XCUR_CHUNK_TYPE_COMMENT 0xFFFE0001
#define XCUR_CHUNK_TYPE_IMAGE   0xFFFD0002

#endif

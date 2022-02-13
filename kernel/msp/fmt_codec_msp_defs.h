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

#ifndef KSQUIRREL_CODEC_DEFS_msp
#define KSQUIRREL_CODEC_DEFS_msp

#define MAGIC_OLD_1 0x6144
#define MAGIC_OLD_2 0x4D6E
#define MAGIC_1     0x694C
#define MAGIC_2     0x536E

struct msp_header
{
    u16  key1;             /* Magic number    */
    u16  key2;             /* Magic number    */
    u16  width;            /* Width of the bitmap in pixels   */
    u16  height;           /* Height of the bitmap in pixels   */
    u16  XARBitmap;        /* X Aspect ratio of the bitmap   */
    u16  YARBitmap;        /* Y Aspect ratio of the bitmap   */
    u16  XARPrinter;       /* X Aspect ratio of the printer   */
    u16  YARPrinter;       /* Y Aspect ratio of the printer   */
    u16  printerWidth;     /* Width of the printer in pixels   */
    u16  printerHeight;    /* Height of the printer in pixels   */
    u16  XAspectCorr;      /* X aspect correction (unused)     */
    u16  YAspectCorr;      /* Y aspect correction (unused)     */
    u16  checksum;         /* Checksum of previous 24 bytes   */
    u16  padding[3];       /* Unused padding    */

}PACKED;

#endif

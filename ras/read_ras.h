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
    aint with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _SQUIRREL_READ_IMAGE_ras
#define _SQUIRREL_READ_IMAGE_ras

#include "../defs.h"
#include "../err.h"

enum RAS_TYPE { ras_old=0x0, ras_standard = 0x1, ras_byte_encoded = 0x2, ras_experimental = 0xFFFF};

typedef struct
{
    unsigned int  ras_magic;
    unsigned int  ras_width;
    unsigned int  ras_height;
    unsigned int  ras_depth;
    unsigned int  ras_length;
    unsigned int  ras_type;
    unsigned int  ras_maptype;
    unsigned int  ras_maplength;
}ATTR_ RAS_HEADER;

#endif

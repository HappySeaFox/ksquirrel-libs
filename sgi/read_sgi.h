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

#ifndef _SQUIRREL_READ_IMAGE_sgi
#define _SQUIRREL_READ_IMAGE_sgi

#include "../defs.h"
#include "../err.h"

typedef struct
{
    unsigned short	Magik; /*  should be 474  */
    unsigned char	StorageFormat; /* 1 == RLE, 0 = Verbatim  */
    unsigned char	bpc;		/*  1|2  */
    unsigned short	Dimensions;	/* 1|2|3 1==1channle+1scanline, 2==1channle+some scanlines, 3==number of channels */
    unsigned short	x;
    unsigned short	y;
    unsigned short	z;
    unsigned long	pixmin;
    unsigned long	pixmax;
    unsigned long	dummy;
    unsigned char	name[80]; /*  ascii string  */
    unsigned long	ColormapID;
    
		/*  0=Normal
		    1=Dither. RGB==3:3:2 per byte, 1 channel
		    2=Screen. Obsolete
		    3=Colormap. Has ONLY colormap, nothing else. 
		*/

    unsigned char	dummy2[404];
    
}SGI_HEADER;


#endif

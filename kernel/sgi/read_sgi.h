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

#include "defs.h"
#include "err.h"
#include "fio.h"

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
    char		name[80]; /*  ascii string  */
    unsigned long	ColormapID;
    
		/*  0=Normal
		    1=Dither. RGB==3:3:2 per byte, 1 channel
		    2=Screen. Obsolete
		    3=Colormap. Has ONLY colormap, nothing else. 
		*/

    unsigned char	dummy2[404];
    
}SGI_HEADER;

extern "C" {

const char*     fmt_version();
const char*     fmt_quickinfo();
const char*     fmt_filter();
const char*     fmt_mime();
const char*     fmt_pixmap();

int     fmt_init(fmt_info *finfo, const char *file);
int     fmt_read_scanline(fmt_info *finfo, RGBA *scan);
int     fmt_readimage(const char*, RGBA **scan, char *);
void    fmt_close();

int     fmt_next(fmt_info *finfo);
int     fmt_next_pass(fmt_info *finfo);

}
			    
#endif

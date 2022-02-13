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

#ifndef __SQUIRREL_LIBS_DEFS_ENDIAN_CONVERSIONS__
#define __SQUIRREL_LIBS_DEFS_ENDIAN_CONVERSIONS__

#include <stdio.h>

unsigned short sLE2BE(unsigned short a)
{
    short i = a, b;
    unsigned short res;

    b = i & 255;
    b = b << 8;
    b = b|0;
    i = i >> 8;
    i = i|0;
    
    res = i|b;
    
    return res;
}

unsigned long lLE2BE(unsigned long a)
{
    unsigned long i = a, b, c, res;
    unsigned short m, n;
    
    b = i >> 16; /* high word */
    c = i << 16;
    c = c >> 16; /* low word */
    
    m = (short)b;
    n = (short)c;
    
    m = sLE2BE(m);
    n = sLE2BE(n);
    
    b = (unsigned long)m;
    c = (unsigned long)n;
    c = c << 16;
    
    b = b|0;
    c = c|0;
    
    res = b|c;
    
    return res;
}


#ifdef BE_SHORT
unsigned short BE_getshort(FILE *inf)
{
    unsigned char buf[2];
 
    fread(buf,2,1,inf);
    return (buf[0]<<8)+(buf[1]<<0);
}
#endif

#ifdef BE_LONG
static long BE_getlong(FILE *inf)
{
    unsigned char buf[4];
 
    fread(buf,4,1,inf);
    return (buf[0]<<24)+(buf[1]<<16)+(buf[2]<<8)+(buf[3]<<0);
}
#endif

#endif

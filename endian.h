#ifndef __SQUIRREL_LIBS_DEFS_ENDIAN_CONVERSIONS__
#define __SQUIRREL_LIBS_DEFS_ENDIAN_CONVERSIONS__

#include <stdio.h>

void sLE2BE(unsigned short *a)
{
    short i = *a, b;

    b = i & 255;
    b = b << 8;
    b = b|0;
    i = i >> 8;
    i = i|0;
    *a = i|b;
}

void lLE2BE(unsigned long *a)
{
    unsigned long i = *a, b, c;
    unsigned short m, n;
    
    b = i >> 16; /* high word */
    c = i << 16;
    c = c >> 16; /* low word */
    
    m = (short)b;
    n = (short)c;
    
    sLE2BE(&m);
    sLE2BE(&n);
    
    b = (unsigned long)m;
    c = (unsigned long)n;
    c = c << 16;
    
    b = b|0;
    c = c|0;
    
    *a = b|c;
}


#endif

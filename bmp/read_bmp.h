#ifndef _READ_WRITE_IMAGE_bmp
#define _READ_WRITE_IMAGE_bmp

#include "../defs.h"
#include "../err.h"


#include <sys/mman.h>

/*  Compression type  */
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L


typedef struct
{
    unsigned short	Type; /*  "BM"  */
    unsigned long 	Size;
    unsigned long	Reserved1;
//    unsigned short	Reserved2;
    unsigned long 	OffBits;

}ATTR_ BITMAPFILE_HEADER;

typedef struct
{
    unsigned long	Size;
    unsigned long	Width;
    unsigned long	Height;
    unsigned short	Planes;
    unsigned short	BitCount;
    unsigned long	Compression;
    unsigned long	SizeImage;
    unsigned long	XPelsPerMeter;
    unsigned long	YPelsPerMeter;
    unsigned long	ClrUsed;
    unsigned long	ClrImportant;

}ATTR_ BITMAPINFO_HEADER;

#endif

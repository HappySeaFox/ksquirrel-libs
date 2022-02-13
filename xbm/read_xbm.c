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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libiberty.h>

#include "read_xbm.h"


char* fmt_version()
{
    return "0.6";
}

char* fmt_quickinfo()
{
    return "X BitMap";
}

char* fmt_extension()
{
    return "*.xbm";
}


/* inits decoding of 'file': opens it, fills struct fmt_info  */
int fmt_init(fmt_info **finfo, const char *file)
{
    *finfo = (fmt_info*)calloc(1, sizeof(fmt_info));
    
    if(!finfo)
	return SQERR_NOMEMORY;
	
    (*finfo)->w = 0;
    (*finfo)->h = 0;
    (*finfo)->bpp = 0;
    (*finfo)->hasalpha = FALSE;
    (*finfo)->needflip = FALSE;
    (*finfo)->images = 1;
    (*finfo)->animated = FALSE;

    (*finfo)->fptr = fopen(file, "rb");
    
    if(!((*finfo)->fptr))
    {
	free(*finfo);
	return SQERR_NOFILE;
    }
    
    return SQERR_OK;
}

long	lscan;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    long		tmp;
    uchar		str[256], *ptr;

    // thanks zgv for error handling :) (svgalib.org)
    if(fgets(str, sizeof(str)-1, finfo->fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQERR_BADFILE;

    finfo->w = (long)atoi(ptr+6);

    if(fgets(str, sizeof(str)-1, finfo->fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQERR_BADFILE;

    finfo->h = (long)atoi(ptr+7);

    while(fgets(str, sizeof(str)-1, finfo->fptr) != NULL)
	if(strncmp(str, "#define ", 8) != 0) break;

    if(str[0] == '\n') fgets(str, sizeof(str)-1, finfo->fptr);

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	return SQERR_BADFILE;

    tmp = lscan = finfo->w;

    finfo->bpp = 1;
    finfo->pal_entr = 2;

    lscan /= 8;
    lscan = lscan + ((tmp%8)?1:0);

    if((finfo->pal = (RGB*)calloc(2, sizeof(RGB))) == 0)
    {
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    memset(finfo->pal, 255, 3);
    memset((finfo->pal)+1, 0, 3);

    asprintf(&finfo->dump, "Width: %ld\nHeight: %ld\nBits per pixel: %d\nNumber of images: %d\nAnimated: %s\nHas palette: %s\n",
    finfo->w,finfo->h,finfo->bpp,finfo->images,(finfo->animated)?"yes":"no",(finfo->pal_entr)?"yes":"no");

    return SQERR_OK;
}


/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
//    printf("%ld ",lscan);return 1;

    uchar index,  c;
    long counter = 0, remain=((finfo->w)<=8)?(finfo->w):((finfo->w)%8), j;
    unsigned int bt;

    memset(scan, 255, finfo->w * 4);

    for(j = 0;j < lscan;j++)
    {
	fscanf(finfo->fptr, "%x%c", &bt, &c);
	// @todo make faster
	if(j==lscan-1 && (remain-0)<=0 && remain)break; index = (bt & 1);        memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-1)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-2)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-3)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-4)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-5)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-6)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-7)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+counter, (finfo->pal)+(int)index, 3); counter++;
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

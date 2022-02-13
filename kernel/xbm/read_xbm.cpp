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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "read_xbm.h"

typedef unsigned char uchar;

FILE *fptr;
int currentImage, bytes;
RGB *pal;

const char* fmt_version()
{
    return (const char*)"0.6.0";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"X BitMap";
}
	
const char* fmt_filter()
{
    return (const char*)"*.xbm ";
}
	    
const char* fmt_mime()
{
    return (const char*)0;
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,112,0,25,192,192,192,255,255,255,0,0,0,0,0,255,255,255,0,4,4,4,176,201,253,137,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,79,73,68,65,84,120,218,99,96,96,72,3,1,6,32,72,20,20,20,20,3,51,148,148,148,196,210,160,12,160,144,49,20,48,152,184,128,129,51,131,169,139,171,107,136,171,171,51,131,73,8,144,114,13,5,50,92,93,144,69,92,145,213,192,116,193,205,129,152,172,36,6,181,20,108,43,216,25,9,0,96,116,27,33,72,26,231,24,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
	return SQERR_NOMEMORY;
	
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    pal = 0;

    return SQERR_OK;
}

long	lscan;
int	version;

int fmt_next(fmt_info *finfo)
{
    long	tmp;
    char	str[256], *ptr;

    currentImage++;
        
    if(currentImage)
	return SQERR_NOTOK;
		    
    if(!finfo->image)
        return SQERR_NOMEMORY;
			
    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;
    
    skip_comments(fptr);

    // thanks zgv for error handling :) (http://svgalib.org)
    if(fgets(str, sizeof(str)-1, fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQERR_BADFILE;

    finfo->image[currentImage].w = (long)atoi(ptr+6);

    if(fgets(str, sizeof(str)-1, fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQERR_BADFILE;

    finfo->image[currentImage].h = (long)atoi(ptr+7);

    while(fgets(str, sizeof(str)-1, fptr) != NULL)
	if(strncmp(str, "#define ", 8) != 0) break;

    if(str[0] == '\n') fgets(str, sizeof(str)-1, fptr);

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	return SQERR_BADFILE;
	
    if((strstr(str, "unsigned") && (strstr(str, "char"))) || strstr(str, "char"))
	version = 11;
    else if(strstr(str, "short"))
	version = 10;
    else
	return SQERR_NOTSUPPORTED;

    tmp = lscan = finfo->image[currentImage].w;

    finfo->image[currentImage].bpp = 1;

    lscan /= 8;
    lscan = lscan + ((tmp%8)?1:0);

    if((pal = (RGB*)calloc(2, sizeof(RGB))) == 0)
    {
	fclose(fptr);
	return SQERR_NOMEMORY;
    }

    memset(pal, 255, sizeof(RGB));
    memset(pal+1, 0, sizeof(RGB));

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
            
    finfo->images++;
	    
    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"Monochrome",
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
//    printf("%ld ",lscan);return 1;

    uchar index,  c;
    long counter = 0, remain=((finfo->image[currentImage].w)<=8)?(finfo->image[currentImage].w):((finfo->image[currentImage].w)%8), j;
    unsigned int bt;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    for(j = 0;j < lscan;j++)
    {
	fscanf(fptr, "%x%c", &bt, &c);
	// @todo make faster
	if(j==lscan-1 && (remain-0)<=0 && remain)break; index = (bt & 1);           memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-1)<=0 && remain)break; index = (bt & 2) ? 1 : 0;   memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-2)<=0 && remain)break; index = (bt & 4) ? 1 : 0;   memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-3)<=0 && remain)break; index = (bt & 8) ? 1 : 0;   memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-4)<=0 && remain)break; index = (bt & 16) ? 1 : 0;  memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-5)<=0 && remain)break; index = (bt & 32) ? 1 : 0;  memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-6)<=0 && remain)break; index = (bt & 64) ? 1 : 0;  memcpy(scan+counter, pal+(int)index, 3); counter++;
	if(j==lscan-1 && (remain-7)<=0 && remain)break; index = (bt & 128) ? 1 : 0; memcpy(scan+counter, pal+(int)index, 3); counter++;
    }

    return (ferror(fptr)) ? SQERR_BADFILE:SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    FILE 	*m_fptr;
    int 	w, h, bpp;
    long	m_lscan;
    int		m_version;
    RGB		*m_pal;

    m_fptr = fopen(file, "rb");
    
    if(!m_fptr)
        return SQERR_NOFILE;
			
    long	tmp;
    char	str[256], *ptr;

    skip_comments(m_fptr);

    // thanks zgv for error handling :) (http://svgalib.org)
    if(fgets(str, sizeof(str)-1, m_fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_width ")) == NULL)
	return SQERR_BADFILE;

    w = atoi(ptr+6);

    if(fgets(str, sizeof(str)-1, m_fptr) == NULL || strncmp(str, "#define ", 8) != 0)
	return SQERR_BADFILE;

    if((ptr = strstr(str, "_height ")) == NULL)
	return SQERR_BADFILE;

    h = (long)atoi(ptr+7);

    while(fgets(str, sizeof(str)-1, m_fptr) != NULL)
	if(strncmp(str, "#define ", 8) != 0) break;

    if(str[0] == '\n') fgets(str, sizeof(str)-1, m_fptr);

    if(strstr(str, "_bits[") == NULL || (ptr = strrchr(str, '{')) == NULL)
	return SQERR_BADFILE;
	
    if((strstr(str, "unsigned") && (strstr(str, "char"))) || strstr(str, "char"))
	m_version = 11;
    else if(strstr(str, "short"))
	m_version = 10;
    else
	return SQERR_NOTSUPPORTED;

    tmp = m_lscan = w;
    bpp = 1;

    m_lscan /= 8;
    m_lscan = m_lscan + ((tmp%8)?1:0);

    if((m_pal = (RGB*)calloc(2, sizeof(RGB))) == 0)
    {
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    memset(m_pal, 255, sizeof(RGB));
    memset(m_pal+1, 0, sizeof(RGB));

    int m_bytes = w * h * sizeof(RGBA);
    
    asprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"Monochrome",
	1,
	m_bytes);
				    
    *image = (RGBA*)realloc(*image, m_bytes);
					
    if(!*image)
    {
        fprintf(stderr, "libSQ_read_xbm: Image is null!\n");
        fclose(m_fptr);
        return SQERR_NOMEMORY;
    }
									    
    memset(*image, 255, m_bytes);

    for(int h2 = 0;h2 < h;h2++)
    {
	uchar index,  c;
	long counter = 0, remain=(w<=8) ? w : (w%8), j;
	unsigned int bt;
        RGBA 	*scan = *image + h2 * w;

	memset(scan, 255, w * sizeof(RGBA));

	for(j = 0;j < m_lscan;j++)
	{
	    fscanf(m_fptr, "%x%c", &bt, &c);
	    // @todo make faster
	    if(j==m_lscan-1 && (remain-0)<=0 && remain)break; index = (bt & 1);        memcpy(scan+counter, m_pal+(int)index, 3); counter++;
    	    if(j==m_lscan-1 && (remain-1)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-2)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-3)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-4)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-5)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-6)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	    if(j==m_lscan-1 && (remain-7)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+counter, m_pal+(int)index, 3); counter++;
	}
    }										

    fclose(m_fptr);

    free(m_pal);
    
    return SQERR_OK;
}

int fmt_close()
{
    fclose(fptr);

    if(pal)
	free(pal);

    return SQERR_OK;
}

/*  skip a single line C-like comment  */
void skip_comments(FILE *fp)
{
    char str[513];
    long pos;

    do
    {
        pos = ftell(fp);
        fgets(str, 512, fp);

        if(!strstr(str, "/*"))
            break;
    }while(true);

    fsetpos(fp, (fpos_t*)&pos);
}

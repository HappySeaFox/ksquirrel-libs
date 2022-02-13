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
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "read_sgi.h"
#include "endian.h"

#define SQ_NEED_FLIP
#include "utils.h"

typedef unsigned char uchar;

FILE *fptr;
int currentImage, bytes;
ulong		*starttab, *lengthtab;
SGI_HEADER	sfh;
int 		rle_row;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.8.3";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"SGI Format";
}
	
const char* fmt_filter()
{
    return (const char*)"*.rgb *.rgba *.bw";
}
	    
const char* fmt_mime()
{
    return (const char*)"\001\332.[\001\002]";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,255,0,224,4,4,4,219,147,181,29,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,82,73,68,65,84,120,218,61,142,193,17,128,64,8,3,105,129,7,13,100,232,192,171,0,83,128,62,174,255,86,4,68,247,181,147,129,128,136,236,66,146,91,85,173,5,128,237,145,140,142,65,22,155,83,86,128,112,164,56,131,244,74,60,194,75,24,193,22,248,204,124,91,127,207,219,12,155,163,125,181,223,184,30,91,52,23,81,90,195,36,47,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;
    starttab = 0;
    lengthtab = 0;

    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    currentImage++;
    
    if(currentImage)
	return SQERR_NOTOK;
	    
    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    finfo->image[currentImage].passes = 1;

    sfh.Magik = BE_getshort(fptr);
    if(!sq_fgetc(fptr, &sfh.StorageFormat)) return SQERR_BADFILE;
    if(!sq_fgetc(fptr, &sfh.bpc)) return SQERR_BADFILE;
    sfh.Dimensions = BE_getshort(fptr);
    sfh.x = BE_getshort(fptr);
    sfh.y = BE_getshort(fptr);
    sfh.z = BE_getshort(fptr);
    sfh.pixmin = BE_getlong(fptr);
    sfh.pixmax = BE_getlong(fptr);
    sfh.dummy = BE_getlong(fptr);

    if(sq_ferror(fptr)) return SQERR_BADFILE;

    if(!sq_fread(sfh.name, sizeof(sfh.name), 1, fptr)) return SQERR_BADFILE;

    sfh.ColormapID = BE_getlong(fptr);

    if(sq_ferror(fptr)) return SQERR_BADFILE;

    if(!sq_fread(sfh.dummy2, sizeof(sfh.dummy2), 1, fptr)) return SQERR_BADFILE;

    finfo->image[currentImage].w = sfh.x;
    finfo->image[currentImage].h = sfh.y;
    finfo->image[currentImage].bpp = sfh.bpc * sfh.z * 8;

    if(finfo->image[currentImage].bpp == 32) finfo->image[currentImage].hasalpha = true;

    if(sfh.Magik != 474 || (sfh.StorageFormat != 0 && sfh.StorageFormat != 1) || (sfh.Dimensions != 1 && sfh.Dimensions != 2 && sfh.Dimensions != 3) || (sfh.bpc != 1 && sfh.bpc != 2))
	return SQERR_BADFILE;

    if(sfh.bpc == 2 || sfh.ColormapID > 0)
	return SQERR_NOTSUPPORTED;

    if(sfh.StorageFormat == 1)
    {
	long sz = sfh.y * sfh.z, i;
        lengthtab = (ulong*)calloc(sz, sizeof(ulong));
	starttab = (ulong*)calloc(sz, sizeof(ulong));
    
        if(!lengthtab)
    	    return SQERR_NOMEMORY;

        if(!starttab)
	{
	    free(lengthtab);
	    return SQERR_NOMEMORY;
	}

	fseek(fptr, 512, SEEK_SET);

	for(i = 0;i < sz;i++)
	    starttab[i] = BE_getlong(fptr);

	for(i = 0;i < sz;i++)
	    lengthtab[i] = BE_getlong(fptr);
    }

    rle_row = 0;

    if(strlen(sfh.name))
    {
	finfo->meta = (fmt_metainfo *)calloc(1, sizeof(fmt_metainfo));

	if(finfo->meta)
	{
	    finfo->meta->entries++;
	    finfo->meta->m = (fmt_meta_entry *)calloc(1, sizeof(fmt_meta_entry));
	    fmt_meta_entry *entry = finfo->meta->m;

	    if(entry)
	    {
		entry[0].datalen = strlen(sfh.name) + 1;
		strcpy(entry[0].group, "SGI Image Name");
		entry[0].data = (char *)malloc(entry[0].datalen);

		if(entry->data)
		    strcpy(entry[0].data, sfh.name);
	    }
	}
    }

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    finfo->image[currentImage].needflip = true;
    finfo->images++;

    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\nRLE\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	(finfo->image[currentImage].bpp == 32) ? "RGBA":"RGB",
	bytes);

    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    const int sz = sfh.x;
    int i = 0, j = 0;
    long pos, len;

    memset(scan, 255, finfo->image[currentImage].w * 4);

    // channel[0] == channel RED, channel[1] = channel GREEN...
    uchar	channel[4][sz];
    uchar	bt;

    memset(channel[3], 255, sz);

    switch(sfh.z)
    {
    	case 1:
	{
	    if(sfh.StorageFormat)
	    {
		    j = 0;

		    fseek(fptr, starttab[rle_row], SEEK_SET);
		    len = lengthtab[rle_row];

		    for(;;)
		    {
			char count;
		    
    			if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;
			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!sq_fgetc(fptr, &channel[0][j])) return SQERR_BADFILE; 

				    j++;

				    if(!len--) goto ex1;
				}
			else
			{
			    if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

			    if(!len--) goto ex1;

			    while(count--)
				channel[0][j++] = bt;
			}
		    }
		    ex1:
		    len = len; // some stuff: get rid of compile warning

		rle_row++;
	    }
	    else
	    {
		if(!sq_fread(channel[0], sz, 1, fptr)) return SQERR_BADFILE;
	    }

	    memcpy(channel[1], channel[0], sz);
	    memcpy(channel[2], channel[0], sz);
	}
	break;


	case 3:
	case 4:
	{
	    if(sfh.StorageFormat)
	    {
		for(i = 0;i < sfh.z;i++)
		{
		    j = 0;

		    fseek(fptr, starttab[rle_row + i*finfo->image[currentImage].h], SEEK_SET);
		    len = lengthtab[rle_row + i*finfo->image[currentImage].h];

		    for(;;)
		    {
			char count;
		    
    			if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!sq_fgetc(fptr, &channel[i][j])) return SQERR_BADFILE; 
				    j++;
				    if(!len--) goto ex;
				}
			else
			{
			    if(!sq_fgetc(fptr, &bt)) return SQERR_BADFILE;

			    if(!len--) goto ex;

			    while(count--)
				channel[i][j++] = bt;
			}
		    }
		    ex:
		    len = len; // some stuff: get rid of compile warning
		}
		rle_row++;
	    }
	    else
	    {
		if(!sq_fread(channel[0], sz, 1, fptr)) return SQERR_BADFILE;

		pos = ftell(fptr);
		fseek(fptr, finfo->image[currentImage].w * (finfo->image[currentImage].h - 1), SEEK_CUR);
		if(!sq_fread(channel[1], sz, 1, fptr)) return SQERR_BADFILE;

		fseek(fptr, finfo->image[currentImage].w * (finfo->image[currentImage].h - 1), SEEK_CUR);
		if(!sq_fread(channel[2], sz, 1, fptr)) return SQERR_BADFILE;

		fseek(fptr, finfo->image[currentImage].w * (finfo->image[currentImage].h - 1), SEEK_CUR);
		if(!sq_fread(channel[3], sz, 1, fptr)) return SQERR_BADFILE;

		fsetpos(fptr, (fpos_t*)&pos);
	    }

	}
	break;
    }

    for(i = 0;i < sz;i++)
    {
        scan[i].r = channel[0][i];
        scan[i].g = channel[1][i];
        scan[i].b = channel[2][i];
        scan[i].a = channel[3][i];
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    FILE 	*m_fptr;
    int 	w, h, bpp;
    SGI_HEADER	m_sfh;
    int 	m_rle_row;
    int 	m_bytes;
    jmp_buf	jmp;

    m_fptr = fopen(file, "rb");

    if(!m_fptr)
        return SQERR_NOFILE;

    if(setjmp(jmp))
    {
        fclose(m_fptr);

        return SQERR_BADFILE;
    }

    m_sfh.Magik = BE_getshort(m_fptr);
    if(!sq_fgetc(m_fptr, &m_sfh.StorageFormat)) longjmp(jmp, 1);
    if(!sq_fgetc(m_fptr, &m_sfh.bpc)) longjmp(jmp, 1);
    m_sfh.Dimensions = BE_getshort(m_fptr);
    m_sfh.x = BE_getshort(m_fptr);
    m_sfh.y = BE_getshort(m_fptr);
    m_sfh.z = BE_getshort(m_fptr);
    m_sfh.pixmin = BE_getlong(m_fptr);
    m_sfh.pixmax = BE_getlong(m_fptr);
    m_sfh.dummy = BE_getlong(m_fptr);

    if(sq_ferror(m_fptr)) longjmp(jmp, 1);
    
    if(!sq_fread(m_sfh.name, sizeof(m_sfh.name), 1, m_fptr)) longjmp(jmp, 1);

    m_sfh.ColormapID = BE_getlong(m_fptr);

    if(sq_ferror(m_fptr)) longjmp(jmp, 1);
    
    if(!sq_fread(m_sfh.dummy2, sizeof(m_sfh.dummy2), 1, m_fptr)) longjmp(jmp, 1);

    w = m_sfh.x;
    h = m_sfh.y;
    bpp = m_sfh.bpc * m_sfh.z * 8;

    if(m_sfh.Magik != 474 || (m_sfh.StorageFormat != 0 && m_sfh.StorageFormat != 1) || (m_sfh.Dimensions != 1 && m_sfh.Dimensions != 2 && m_sfh.Dimensions != 3) || (m_sfh.bpc != 1 && m_sfh.bpc != 2))
	return SQERR_BADFILE;

    if(m_sfh.bpc == 2 || m_sfh.ColormapID > 0)
	return SQERR_NOTSUPPORTED;

    long sz = m_sfh.y * m_sfh.z, i;

    ulong m_lengthtab[sz];
    ulong m_starttab[sz];

    if(m_sfh.StorageFormat == 1)
    {
	fseek(m_fptr, 512, SEEK_SET);

	for(i = 0;i < sz;i++)
	    m_starttab[i] = BE_getlong(m_fptr);

	for(i = 0;i < sz;i++)
	    m_lengthtab[i] = BE_getlong(m_fptr);
    }

    m_rle_row = 0;

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\nRLE\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	(bpp == 32) ? "RGBA":"RGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_sgi: Image is null!\n");
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */

    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA 	*scan = *image + h2 * w;

    const int sz = m_sfh.x;
    int i = 0, j = 0;
    long pos, len;

    memset(scan, 255, w * sizeof(RGBA));

    uchar	channel[4][sz];
    uchar	bt;

    memset(channel[3], 255, sz);

    switch(m_sfh.z)
    {
    	case 1:
	{
	    if(m_sfh.StorageFormat)
	    {
		    j = 0;

		    fseek(m_fptr, m_starttab[m_rle_row], SEEK_SET);
		    len = m_lengthtab[m_rle_row];

		    for(;;)
		    {
			char count;
		    
    			if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);

			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!sq_fgetc(m_fptr, &channel[0][j])) longjmp(jmp, 1); 
				    j++;

				    if(!len--) goto ex1;
				}
			else
			{
			    if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);

			    if(!len--) goto ex1;

			    while(count--)
				channel[0][j++] = bt;
			}
		    }
		    ex1:
		    len = len; // some stuff: get rid of compile warning

		m_rle_row++;
	    }
	    else
	    {
		if(!sq_fread(channel[0], sz, 1, m_fptr)) longjmp(jmp, 1);
	    }

	    memcpy(channel[1], channel[0], sz);
	    memcpy(channel[2], channel[0], sz);
	}
	break;


	case 3:
	case 4:
	{
	    if(m_sfh.StorageFormat)
	    {
		for(i = 0;i < m_sfh.z;i++)
		{
		    j = 0;

		    fseek(m_fptr, m_starttab[m_rle_row + i*h], SEEK_SET);
		    len = m_lengthtab[m_rle_row + i*h];

		    for(;;)
		    {
			char count;
		    
    			if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);

			count = bt&0x7f;

			if(!count) break;
		    
			if(bt & 0x80)
			    while(count--)
				{
				    if(!sq_fgetc(m_fptr, &channel[i][j])) longjmp(jmp, 1); 
				    j++;

				    if(!len--) goto ex;
				}
			else
			{
			    if(!sq_fgetc(m_fptr, &bt)) longjmp(jmp, 1);
			    if(!len--) goto ex;

			    while(count--)
				channel[i][j++] = bt;
			}
		    }
		    ex:
		    len = len; // some stuff: get rid of compile warning
		}
		m_rle_row++;
	    }
	    else
	    {
		if(!sq_fread(channel[0], sz, 1, m_fptr)) longjmp(jmp, 1);

		pos = ftell(m_fptr);
		fseek(m_fptr, w * (h - 1), SEEK_CUR);
		if(!sq_fread(channel[1], sz, 1, m_fptr)) longjmp(jmp, 1);

		fseek(m_fptr, w * (h - 1), SEEK_CUR);
		if(!sq_fread(channel[2], sz, 1, m_fptr)) longjmp(jmp, 1);

		fseek(m_fptr, w * (h - 1), SEEK_CUR);
		if(!sq_fread(channel[3], sz, 1, m_fptr)) longjmp(jmp, 1);

		fsetpos(m_fptr, (fpos_t*)&pos);
	    }

	}
	break;
    }

    for(i = 0;i < sz;i++)
    {
        scan[i].r = channel[0][i];
        scan[i].g = channel[1][i];
        scan[i].b = channel[2][i];
        scan[i].a = channel[3][i];
    }
    }

    fclose(m_fptr);

    flip((char*)*image, w * sizeof(RGBA), h);

    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
    
    if(starttab)
	free(starttab);
	
    if(lengthtab)
	free(lengthtab);
}

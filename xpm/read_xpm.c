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

#include "xpm_utils.h"

char* fmt_version()
{
    return "0.5.1";
}

char* fmt_quickinfo()
{
    return "X11 PixMap";
}

char* fmt_extension()
{
    return "*.xpm";
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

ulong		numcolors;
uchar		cpp;
XPM_VALUE	*Xmap;


/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    long		i;
    uchar		str[256];

    skip_comments(finfo->fptr);
    fgets(str, 256, finfo->fptr);  /*  static char* ....  */
    skip_comments(finfo->fptr);
    fgets(str, 256, finfo->fptr);
    skip_comments(finfo->fptr);
    
    sscanf(str, "\"%ld %ld %ld %d", &finfo->w, &finfo->h, &numcolors, (int*)&cpp);
//    printf("%d %d %d %d\n\n",finfo->w,finfo->h,numcolors,cpp);
    
    if((Xmap = (XPM_VALUE*)calloc(numcolors, sizeof(XPM_VALUE))) == NULL)
    {
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    uchar name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < numcolors;i++)
    {
	fgets(str, 256, finfo->fptr);

	strcpy(name, "");

	found = strstr(str, "\"");
	found++;

	strncpy(name, found, cpp);
	name[cpp] = 0;

	sscanf(found+cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

//	if(!i)printf("%s\n",color);

	memcpy(Xmap[i].name, name, cpp);
	Xmap[i].rgba = hex2rgb(color);
    }

    skip_comments(finfo->fptr);

    QuickSort(Xmap, 0, numcolors-1);
/*
    for(i = 0;i < numcolors;i++)
    {
	printf("\"%s\"  %d %d %d %d\n",Xmap[i].name,Xmap[i].rgba.r,Xmap[i].rgba.g,Xmap[i].rgba.b,Xmap[i].rgba.a);
    }
*/
    finfo->bpp = 24;

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
    const int	bpl = finfo->w * (cpp+2);
    int		i, j;
    uchar 	line[bpl], key[KEY_LENGTH];
    
    memset(scan, 255, finfo->w * 4);
    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));

    switch(finfo->bpp)
    {
	case 24:
	{
	    static RGBA *trgba;

	    i = j = 0;
	    fgets(line, sizeof(line), finfo->fptr);

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < finfo->w;j++)
	    {
		strncpy(key, line+i, cpp);
		i += cpp;
		
//		printf("\"%s\" %d  \"%s\"\n",line,i,key);

		trgba = BinSearch(Xmap, 0, numcolors-1, key);
		
		if(!trgba)
		{
		    printf("XPM decoder: WARNING: color \"%s\" not found, assuming black instead\n", key);
		    memset(trgba, 0, sizeof(RGBA));
		    trgba->a = 255;
		}

		memcpy(scan+j, trgba, sizeof(RGBA));
	    }
	}
	break;

	default:
	{
		//@TODO:  free memory !!
		return SQERR_BADFILE;
	}
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

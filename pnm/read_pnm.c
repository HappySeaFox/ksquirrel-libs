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

#include "read_pnm.h"


char* fmt_version()
{
    return "0.6";
}

char* fmt_quickinfo()
{
    return "Portable aNy Map";
}

char* fmt_extension()
{
    return "*.pnm *.pbm *.pgm *.ppm";
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


PPM_HEADER	pfh;
int		pnm;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    uchar		str[256];
    int			w, h, maxcolor;

    fread(&pfh, 2, 1, finfo->fptr);
    pfh.ID[2] = '\0';

    fgetc(finfo->fptr);

    for(;;)
    {
	fgets(str, 255, finfo->fptr);
        if(str[0] != '#')
	    break;
    }

    sscanf(str, "%d%d", &w, &h);
    if(pfh.ID[1] != '4')
	fscanf(finfo->fptr, "%d", &maxcolor);

    finfo->pal_entr = 0;
    finfo->pal = 0;
    finfo->w = w;
    finfo->h = h;
    finfo->bpp = 0;

    pnm = pfh.ID[1] - 48;

    if(pnm != 4 && pnm != 1)
	fgetc(finfo->fptr);

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
    RGB		rgb;
    uchar	bt;
    int		i;


//    printf("pos: %ld\n", ftell(finfo->fptr));

    memset(scan, 255, finfo->w * 4);

    switch(pnm)
    {
	case 3:
    	    for(i = 0;i < finfo->w;i++)
	    {
		fscanf(finfo->fptr ,"%d%d%d", (int*)&rgb.r, (int*)&rgb.g, (int*)&rgb.b);
		memcpy(scan+i, &rgb, 3);
	    }
	break;

	case 6:
	    for(i = 0;i < finfo->w;i++)
	    {
		fread(&rgb,sizeof(RGB),1,finfo->fptr);
		memcpy(scan+i, &rgb, 3);
    	    }
	break;
	
	case 2:
	    for(i = 0;i < finfo->w;i++)
	    {
		fscanf(finfo->fptr ,"%d", (int*)&bt);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
    
	case 5:
	    for(i = 0;i < finfo->w;i++)
	    {
		fread(&bt,1,1,finfo->fptr);
		rgb.r = rgb.g = rgb.b = bt;
		memcpy(scan+i, &rgb, 3);
	    }
	break;
	
	case 1:
        {
    	    RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;i < finfo->w;i++)
	    {
		fscanf(finfo->fptr ,"%d", (int*)&bt);
		memcpy(scan+i, palmono+(int)bt, 3);
    	    }
	}
	break;

	case 4:
	{
	    int index;//, remain = finfo->w % 8;

    	    static RGB palmono[2] = {{255,255,255}, {0,0,0}};

	    for(i = 0;;)
	    {
		fread(&bt,1,1,finfo->fptr);
		index = (bt&128)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&64)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&32)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&16)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&8)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&4)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&2)?1:0;
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
		index = (bt&1);
		memcpy(scan+i, palmono+index, 3);i++; if(i >= finfo->w) break;
	    }
	}
	break;
    }

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);
    return SQERR_OK;
}

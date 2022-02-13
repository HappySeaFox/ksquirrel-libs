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
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "xpm_utils.h"
#include "utils.h"

FILE *fptr;
int currentImage, bytes;

typedef unsigned char uchar;

const char* fmt_version()
{
    return (const char*)"0.5.2";
}
    
const char* fmt_quickinfo()
{
    return (const char*)"X11 Pixmap";
}
	
const char* fmt_filter()
{
    return (const char*)"*.xpm ";
}
	    
const char* fmt_mime()
{
    return (const char*)"/. XPM ./\n";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,18,80,76,84,69,99,109,97,192,192,192,255,255,255,0,0,0,95,95,95,4,4,4,61,221,162,37,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,83,73,68,65,84,120,218,61,142,65,21,192,48,8,67,99,129,3,6,120,85,176,85,1,69,192,118,168,127,43,163,41,93,46,228,125,32,0,96,46,33,245,138,136,210,152,153,206,50,137,238,18,122,80,3,87,184,55,247,129,222,178,184,165,241,68,135,176,181,102,130,228,108,253,57,59,217,180,142,242,42,223,120,62,156,44,24,95,168,109,132,217,0,0,0,0,73,69,78,68,174,66,96,130,130";
}

int fmt_init(fmt_info *, const char *file)
{
    fptr = fopen(file, "rb");
	        
    if(!fptr)
	return SQERR_NOFILE;
		    
    currentImage = -1;

    return SQERR_OK;
}

int		numcolors;
int		cpp;
XPM_VALUE	*Xmap;

int fmt_next(fmt_info *finfo)
{
    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    if(!finfo)
        return SQERR_NOMEMORY;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    int		i;
    char	str[256];

    int ret;

    while(true) { ret = skip_comments(fptr); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }
    if(!sq_fgets(str, 256, fptr)) return SQERR_BADFILE;
    if(strncmp(str, "static", 6) != 0) return SQERR_BADFILE;
    while(true) { ret = skip_comments(fptr); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }
    if(!sq_fgets(str, 256, fptr)) return SQERR_BADFILE;
    while(true) { ret = skip_comments(fptr); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }

    sscanf(str, "\"%d %d %d %d", &finfo->image[currentImage].w, &finfo->image[currentImage].h, &numcolors, (int*)&cpp);
//    printf("%d %d %d %d\n\n",finfo->image[currentImage].w,finfo->image[currentImage].h,numcolors,cpp);

    if(!numcolors)
	return SQERR_BADFILE;

    if((Xmap = (XPM_VALUE*)calloc(numcolors, sizeof(XPM_VALUE))) == NULL)
    {
//	fclose(fptr);
	return SQERR_NOMEMORY;
    }

    char name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < numcolors;i++)
    {
	if(!sq_fgets(str, 256, fptr)) return SQERR_BADFILE;

	if(*str != '\"')
	{
	    trace("libSQ_read_xpm: file corrupted");
	    numcolors = i;
	    break;
	}

	strcpy(name, "");

	found = str;
	found++;

	strncpy(name, found, cpp);
	name[cpp] = 0;

	sscanf(found+cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

//	if(!i)printf("%s\n",color);

	memcpy(Xmap[i].name, name, cpp);
	Xmap[i].name[cpp] = 0;
	Xmap[i].rgba = hex2rgb(color);
    }
    
    if(!numcolors)
	return SQERR_BADFILE;

    while(true) { ret = skip_comments(fptr); if(ret == 1) continue; else if(!ret) break; else return SQERR_BADFILE; }

    QuickSort(Xmap, 0, numcolors-1);
/*
    for(i = 0;i < numcolors;i++)
    {
	printf("\"%s\"  %d %d %d %d\n",Xmap[i].name,Xmap[i].rgba.r,Xmap[i].rgba.g,Xmap[i].rgba.b,Xmap[i].rgba.a);
    }
*/
    finfo->image[currentImage].bpp = 24;
    finfo->image[currentImage].hasalpha = true;

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);
            
    finfo->images++;

    finfo->image[currentImage].passes = 1;

    snprintf(finfo->image[currentImage].dump, sizeof(finfo->image[currentImage].dump), "%s\n%dx%d\n%d\n%s\n-\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	"RGBA",
	bytes);
					
    return SQERR_OK;
}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}

int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    const int	bpl = finfo->image[currentImage].w * (cpp+2);
    int		i, j;
    char 	line[bpl], key[KEY_LENGTH];

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));
    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));
/*    
    static int ee = 0;
    printf("line %d\n", ee);
    ee++;
*/
    switch(finfo->image[currentImage].bpp)
    {
	case 24:
	{
	    RGBA *trgba;
	    RGBA  rgba;

	    i = j = 0;
	    if(!sq_fgets(line, sizeof(line), fptr)) return SQERR_BADFILE;

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < finfo->image[currentImage].w;j++)
	    {
		strncpy(key, line+i, cpp);
		i += cpp;

		trgba = BinSearch(Xmap, 0, numcolors-1, key);

		if(!trgba)
		{
		    printf("XPM decoder: WARNING: color \"%s\" not found, assuming transparent instead\n", key);
		    memset(&rgba, 0, sizeof(RGBA));
		    trgba = &rgba;
		}

		memcpy(scan+j, trgba, sizeof(RGBA));
	    }
	}
	break;
    }

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char *dump)
{
    FILE 	*m_fptr;
    int 	w, h, bpp;
    int		m_numcolors;
    int		m_cpp;
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
			    
    int		i;
    char	str[256];
    int 	ret;

    while(true) { ret = skip_comments(m_fptr); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }
    if(!sq_fgets(str, 256, m_fptr)) longjmp(jmp, 1);
    if(strncmp(str, "static", 6) != 0) longjmp(jmp, 1);
    while(true) { ret = skip_comments(m_fptr); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }
    if(!sq_fgets(str, 256, m_fptr)) longjmp(jmp, 1);
    while(true) { ret = skip_comments(m_fptr); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }

    sscanf(str, "\"%d %d %d %d", &w, &h, &m_numcolors, (int*)&m_cpp);
//    printf("%d %d %d %d\n\n",finfo->w,finfo->h,m_numcolors,m_cpp);

    XPM_VALUE	m_Xmap[m_numcolors];

    char name[KEY_LENGTH], c[3], color[10], *found;

    for(i = 0;i < m_numcolors;i++)
    {
	if(!sq_fgets(str, 256, m_fptr)) longjmp(jmp, 1);

	if(*str != '\"')
	{
	    trace("libSQ_read_xpm: file corrupted.");
	    m_numcolors = i;
	    break;
	}

	strcpy(name, "");

	found = str;
	found++;

	strncpy(name, found, m_cpp);
	name[m_cpp] = 0;

	sscanf(found+m_cpp+1, "%s %s", c, color);
	
	found = strstr(color, "\"");
	if(found) *found = 0;

	memcpy(m_Xmap[i].name, name, m_cpp);
	m_Xmap[i].name[m_cpp] = 0;
	m_Xmap[i].rgba = hex2rgb(color);
    }

    if(!m_numcolors)
	longjmp(jmp, 1);

    while(true) { ret = skip_comments(m_fptr); if(ret == 1) continue; else if(!ret) break; else longjmp(jmp, 1); }

    QuickSort(m_Xmap, 0, m_numcolors-1);
/*
    for(i = 0;i < m_numcolors;i++)
    {
	printf("%d %d %d %d\n", m_Xmap[i].rgba.r,m_Xmap[i].rgba.g,m_Xmap[i].rgba.b,m_Xmap[i].rgba.a);
    }
*/
    bpp = 24;

    m_bytes = w * h * sizeof(RGBA);

    sprintf(dump, "%s\n%d\n%d\n%d\n%s\n-\n%d\n%d\n",
	fmt_quickinfo(),
	w,
	h,
	bpp,
	"RGB",
	1,
	m_bytes);

    *image = (RGBA*)realloc(*image, m_bytes);

    if(!*image)
    {
        fprintf(stderr, "libSQ_read_xpm: Image is null!\n");
	longjmp(jmp, 1);
    }

    memset(*image, 255, m_bytes);

    /*  reading ... */
    
    for(int h2 = 0;h2 < h;h2++)
    {
        RGBA *scan = *image + h2 * w;

    const int	bpl = w * (m_cpp+2);
    int		i, j;
    char 	line[bpl], key[KEY_LENGTH];

    memset(key, 0, sizeof(key));
    memset(line, 0, sizeof(line));

    switch(bpp)
    {
	case 24:
	{
	    RGBA *trgba;
	    RGBA rgba;

	    i = j = 0;
	    if(!sq_fgets(line, sizeof(line), m_fptr)) longjmp(jmp, 1);

	    while(line[i++] != '\"') // skip spaces
	    {}

	    for(;j < w;j++)
	    {
		strncpy(key, line+i, m_cpp);
		i += m_cpp;
		
//		printf("\"%s\" %d  \"%s\"\n",line,i,key);

		trgba = BinSearch(m_Xmap, 0, m_numcolors-1, key);
		
		if(!trgba)
		{
		    printf("XPM decoder: WARNING: color \"%s\" not found, assuming transparent instead\n", key);
		    memset(&rgba, 0, sizeof(RGBA));
		    trgba = &rgba;
		}

		memcpy(scan+j, trgba, sizeof(RGBA));
	    }
	}
	break;
    }

    }

    fclose(m_fptr);
    
    return SQERR_OK;
}

void fmt_close()
{
    fclose(fptr);
    
    if(Xmap)
	free(Xmap);
}

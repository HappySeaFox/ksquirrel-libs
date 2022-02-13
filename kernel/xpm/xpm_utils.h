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

#ifndef SQ_XPM_UTILS
#define SQ_XPM_UTILS

#include "read_xpm.h"
#include "rgb.h"
#include <ctype.h>

#define KEY_LENGTH  10

#ifndef uchar
typedef unsigned char uchar;
#endif

template <class T>
void Swap(T *a, T *b);

RGBA hex2rgb(const char *hex);
void skip_comments(FILE *fp);

template <class T>
RGBA* BinSearch(T A[], int low, int high, const char *key);

/***************************************************************************/


template <class T>
void Swap(T *a, T *b)
{
    T tmp;
    
    memcpy(&tmp, a, sizeof(T));
    memcpy(a, b, sizeof(T));
    memcpy(b, &tmp, sizeof(T));
}

template <class T>
void QuickSort(T A[], int low, int high)
{
    T  pivot;
    int scanUp, scanDown;
    int mid;

    if(high - low <= 0)
	return;
    else
	if(high - low == 1)
	{
	    if(strcmp(A[high].name, A[low].name) < 0) 
        	Swap(&A[low], &A[high]);
    	    return;
	}

    mid = (low + high)/2;
    memcpy(&pivot, &A[mid], sizeof(XPM_VALUE));

    Swap(&A[mid], &A[low]);
    scanUp = low + 1;
    scanDown = high;

    do 
    {
	while(scanUp <= scanDown && strcmp(A[scanUp].name, pivot.name) <= 0)
	    scanUp++;

	while(strcmp(pivot.name, A[scanDown].name) < 0)
	    scanDown--;

	if(scanUp < scanDown)
	    Swap(&A[scanUp], &A[scanDown]);
    }while(scanUp < scanDown);

    memcpy(&A[low], &A[scanDown], sizeof(XPM_VALUE));
    memcpy(&A[scanDown], &pivot, sizeof(XPM_VALUE));

    if(low < scanDown-1)
	QuickSort(A, low, scanDown-1);

    if(scanDown+1 < high)
	QuickSort(A, scanDown+1, high);
}

RGBA hex2rgb(const char *hex)
{
    RGBA rgba;
    char c[3];
    const uchar add = strlen(hex+1) / 3 - 2;

    if(!strncasecmp(hex, "none", 4) || !strncasecmp(hex, "one", 3)) // not pretty hack
    {
	memset(&rgba, 0, sizeof(RGBA));
	return rgba;
    }

    if(isalpha(*hex))
    {
	RGBA *trgba = BinSearch(named, 0, sizeof(named) / sizeof(XPM_NAMED_COLOR) - 1, hex);
	if(!trgba)
	{
	    fprintf(stderr, "XPM decoder: WARNING: named color \"%s\" not found, assuming black color instead\n", hex);
    	    rgba.r = rgba.g = rgba.b = 0;
	    rgba.a = 255;
	    return rgba;
	}
	
	return *trgba;
    }

    hex++;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.r = (uchar)strtol(c, NULL, 16);
    hex = hex + 2 + add;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.g = (uchar)strtol(c, NULL, 16);
    hex = hex + 2 + add;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.b = (uchar)strtol(c, NULL, 16);

    rgba.a = 255;

    return rgba;
}

template <class T>
RGBA* BinSearch(T A[], int low, int high, const char *key)
{
    int mid;
    char midvalue[KEY_LENGTH];

    if(low > high)
        return 0;
    else
    {
        mid = (low+high)/2;

        strcpy(midvalue, A[mid].name);

        if (!strcmp(key, midvalue))
            return &A[mid].rgba;
        else if(strcmp(key, midvalue) < 0)
            return BinSearch(A,low,mid-1,key);
        else
            return BinSearch(A,mid+1,high,key);
    }
}

/*  skip a single line C-like comment  */
void skip_comments(FILE *fp)
{
    char str[256];
    long pos;
    
    do
    {
	pos = ftell(fp);
	fgets(str, 256, fp);
	
	if(!strstr(str, "/*"))
	    break;
    }while(1);
    
    fsetpos(fp, (fpos_t*)&pos);
}

#endif

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
    as32 with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQUIRREL_READ_IMAGE_pnm
#define KSQUIRREL_READ_IMAGE_pnm

bool skip_flood(FILE *);

bool sq_fgetc(FILE *f, u8 *c)
{
    s32 e = fgetc(f);

    if(ferror(f) || feof(f))
        return false;

    *c = e;

    return true;
}

bool sq_fgets(s8 *s, s32 size, FILE *stream)
{
    s8 *r = (s8*)fgets((char*)s, size, stream);

    if(ferror(stream) || feof(stream) || !r)
        return false;

    return true;
}

bool sq_ferror(FILE *f)
{
    return (ferror(f) || feof(f));
}

bool sq_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t r = fread(ptr, size, nmemb, stream);

    if(ferror(stream) || feof(stream) || r != nmemb)
	return false;

    return true;
}

#endif

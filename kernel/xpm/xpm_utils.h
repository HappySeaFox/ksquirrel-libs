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

#ifndef SQ_XPM_UTILS
#define SQ_XPM_UTILS

#include <cctype>

#define KEY_LENGTH  25

RGBA fmt_codec::hex2rgb(const s8 *hex)
{
    RGBA rgba;
    s8 c[3];
    const s8 add = (const s8)strlen(hex+1) / 3 - 2;

    if(!strncasecmp(hex, "none", 4) || !strncasecmp(hex, "one", 3)) // not pretty hack
    {
	memset(&rgba, 0, sizeof(RGBA));
	return rgba;
    }

    if(isalpha(*hex))
    {
	RGBA trgba;
	bool f;// = BinSearch(named, 0, sizeof(named) / sizeof(XPM_NAMED_COLOR) - 1, hex);

	std::map<std::string, RGBA>::const_iterator it = named.find(hex);

	f = (it != named.end());

	if(!f)
	{
	    cerr << "XPM decoder: WARNING: named color \"" << hex << "\" not found, assuming transparent instead" << endl;
    	    memset(&rgba, 0, sizeof(RGBA));
	    return rgba;
	}

	trgba = named[std::string(hex)];

	return trgba;
    }

    hex++;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.r = (s8)strtol(c, NULL, 16);
    hex = hex + 2 + add;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.g = (s8)strtol(c, NULL, 16);
    hex = hex + 2 + add;

    memcpy(c, hex, 2);
    c[2] = 0;
    rgba.b = (s8)strtol(c, NULL, 16);

    rgba.a = 255;

    return rgba;
}

/*  skip a single line C-like comment  */
s32 skip_comments(ifstreamK &ff)
{
    s8 str[4096];
    fstream::pos_type pos;
    bool skipped = false;

    pos = ff.tellg();

    ff.getline(str, sizeof(str)-1);

    if(ff.eof()) return 2;

    if((*str == '\n' && *(str+1) == '\0') || (*str == '\n' && *(str+1) == '\r' && *(str+2) == '\0') || (*str == '\r' && *(str+1) == '\n' && *(str+2) == '\0'))
        skipped = true;

    if(strstr(str, "/*") || *str == '#')
        skipped = true;

    if(!skipped)
	ff.seekg(pos);

    return (int)skipped;
}

#endif

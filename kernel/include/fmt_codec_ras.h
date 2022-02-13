/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

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

#ifndef KSQUIRREL_LIBS_CLASS_DEFINITION_RAS_H
#define KSQUIRREL_LIBS_CLASS_DEFINITION_RAS_H

#include "ksquirrel-libs/fmt_codec_base.h"

class fmt_codec : public fmt_codec_base
{
    public:

        BASE_CODEC_DECLARATIONS

    private:
	RGB		pal[256];
	RAS_HEADER 	rfh;
	bool 		rle, isRGB;
	u16 	fill;
	u8 	fillchar;
	u16 	linelength;
	u8 	*buf;
};

#endif

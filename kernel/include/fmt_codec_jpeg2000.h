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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQUIRREL_LIBS_CLASS_DEFINITION_jpeg2000_H
#define KSQUIRREL_LIBS_CLASS_DEFINITION_jpeg2000_H

#include "ksquirrel-libs/fmt_codec_base.h"

typedef struct
{
    jas_image_t		*image;
    s32			cmptlut[MAXCMPTS];
    jas_image_t		*altimage;
    jas_matrix_t        *data[3];
    jas_seqent_t        *d[3];

} gs_t;

class fmt_codec : public fmt_codec_base
{
    public:

        BASE_CODEC_DECLARATIONS

    private:
	bool convert_colorspace();
	
    private:
	gs_t		gs;
	jas_stream_t	*in;
};

#endif

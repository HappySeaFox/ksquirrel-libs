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

#ifndef KSQUIRREL_LIBS_CLASS_DEFINITION_PNG_H
#define KSQUIRREL_LIBS_CLASS_DEFINITION_PNG_H

#include "ksquirrel-libs/fmt_codec_base.h"

class fmt_codec : public fmt_codec_base
{
    public:

        BASE_CODEC_DECLARATIONS

#ifdef CODEC_ANOTHER
        virtual void fill_default_settings();
#endif

#ifdef CODEC_PNG
        WRITE_CODEC_DECLARATIONS
#endif

    private:
	png_structp     png_ptr;
	png_infop       info_ptr;
	png_uint_32     width, height, number_passes;
	s32             color_type;
	png_bytep       *cur, *prev, *frame;
	FILE		*fptr;
        s32             bit_depth, interlace_type;
        s32             frames;
        fmt_image       img;

        png_uint_32 next_frame_width, next_frame_height, next_frame_x_offset, next_frame_y_offset;
        png_uint_16 next_frame_delay_num, next_frame_delay_den;
        png_byte next_frame_dispose_op, next_frame_blend_op;

	FILE        	*m_fptr;
        png_structp 	m_png_ptr;
	png_infop   	m_info_ptr;
	png_bytep 	m_row_pointer;
        bool            zerror;
};

#endif

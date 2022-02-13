/*  This file is part of the ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2004 Dmitry Baryshev <ksquirrel@tut.by>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License (LGPL) as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQUIRREL_READ_IMAGE_gif
#define KSQUIRREL_READ_IMAGE_gif

#define SQ_NEED_OPERATOR_RGBA_RGB

#include "defs.h"
#include "err.h"

extern "C" {

const char*     fmt_version(void);
const char*     fmt_quickinfo(void);
const char*     fmt_filter(void);
const char*     fmt_mime(void);
const char*     fmt_pixmap(void);

int     fmt_init(fmt_info *finfo, const char *file);
int     fmt_read_scanline(fmt_info *finfo, RGBA *scan);
int     fmt_readimage(const char*, RGBA **scan, char **);
int     fmt_close();

int	fmt_next(fmt_info *finfo);
int	fmt_next_pass(fmt_info *finfo);
}

#define DISPOSAL_NO          0
#define DOSPOSAL_LEFT        1
#define DISPOSAL_BACKGROUND  2
#define DISPOSAL_PREVIOUS    3

#endif

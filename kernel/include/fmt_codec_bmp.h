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

#ifndef KSQUIRREL_LIBS_CLASS_DEFINITION_BMP_H
#define KSQUIRREL_LIBS_CLASS_DEFINITION_BMP_H

#include "fmt_codec_base.h"

class fmt_codec : public fmt_codec_base
{
    public:

	fmt_codec();
	~fmt_codec();

	virtual std::string	fmt_version();
	virtual std::string	fmt_quickinfo();
	virtual std::string	fmt_filter();
	virtual std::string	fmt_mime();
	virtual std::string	fmt_pixmap();

	virtual bool 	fmt_readable() const;
	virtual s32	fmt_read_init(std::string file);
	virtual s32	fmt_read_next();
	virtual s32	fmt_read_next_pass();
	virtual s32	fmt_read_scanline(RGBA *scan);
	virtual void	fmt_read_close();

	virtual bool	fmt_writable() const;
	virtual void	fmt_getwriteoptions(fmt_writeoptionsabs *);
        virtual s32     fmt_write_init(std::string file, const fmt_image &image, const fmt_writeoptions &opt);
        virtual s32     fmt_write_next();
        virtual s32     fmt_write_next_pass();
        virtual s32     fmt_write_scanline(RGBA *scan);
        virtual void    fmt_write_close();

    private:
	RGB			pal[256];
	s32			pal_entr;
	u16			filler;
	BITMAPFILE_HEADER       bfh;
	BITMAPINFO_HEADER       bih;
        s32                 	m_FILLER;
        BITMAPFILE_HEADER   	m_bfh;
        BITMAPINFO_HEADER  	m_bih;
};

extern "C" fmt_codec_base* fmt_codec_create()
{
    return (new fmt_codec);
}

extern "C" void fmt_codec_destroy(fmt_codec_base *p)
{
    delete p;
}

#endif

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

#ifndef KSQUIRREL_LIBS_CLASS_DEFINITION_H
#define KSQUIRREL_LIBS_CLASS_DEFINITION_H

#include <ksquirrel-libs/fmt_defs.h>
#include <ksquirrel-libs/fileio.h>
#include <ksquirrel-libs/settings.h>

//////////////////////////////////
//                              //
//  Base class for all          //
//  codecs                      //
//                              //
//////////////////////////////////


class fmt_codec_base
{
    public:
	fmt_codec_base()
	{}

	virtual ~fmt_codec_base()
	{}

	// version of the library, e.g. "1.2.2", etc.
        //
        // tell clients that fmt_codec_base couldn't be
        // instantianted
	virtual void	        options(codec_options *) = 0;

	// return file extension for a given bpp
	virtual std::string     extension(const s32 bpp);

	/*
	 *             read methods
	 */

	// fmt_read_init: do what you need before decoding
	virtual s32	read_init(const std::string &file);

	// fmt_read_next: seek to correct file offset, do other initialization stuff.
	// this method should be (and will be) called before image is about to
	// be decoded.
	virtual s32	read_next();

	// fmt_read_next_pass: do somethimg important before the next pass
	// will be decoded (usually do nothing, if the image has only 1 pass (like BMP, PCX ...),
	// or adjust variables if the image is interlaced, with passes > 1 (like GIF, PNG))
	virtual s32	read_next_pass();

	// fmt_readscanline: read one scanline from file
	virtual s32	read_scanline(RGBA *scan);

	// fmt_read_close: close all handles, free memory, etc.
	virtual void	read_close();

	/*
	 *             write methods
	 */

	// fmt_getwriteoptions: return write options for this image format
	virtual void    getwriteoptions(fmt_writeoptionsabs *);

	// fmt_write_init: init writing
	virtual s32     write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt);

	virtual s32	write_next();

	virtual s32	write_next_pass();

	// fmt_write_scanline: write scanline
	virtual s32	write_scanline(RGBA *scan);

	// fmt_write_close: close writing(close descriptors, free memory, etc.)
	virtual void	write_close();

	fmt_info information() const;

	fmt_image* image(const int index);

        void addmeta(const fmt_metaentry &m);

        void settempfile(const std::string &t);

        // fill fmt_settings with values
        void set_settings(const fmt_settings &sett);

        fmt_settings settings() const;

        virtual void fill_default_settings()
        {}

    protected:
	// image index in finfo.image
	s32               currentImage;

	// fmt_info structure
	fmt_info          finfo;

	// input stream
	ifstreamK         frs;

	// output stream
	ofstreamK         fws;

	// some additional error checkers
	bool              read_error, write_error;

	// line and layer indexes - needed by some
	// interlaced or layered images
	s32               line, layer;

	// error code
	s32               write_error_code;

	// write options
	fmt_writeoptions  writeopt;

	// saved fmt_image
	fmt_image         writeimage;

        // path to temporary file
        // should be set by the highlevel application
        // with settempfile()
        std::string       tmp;

        fmt_settings      m_settings;
};

inline
fmt_settings fmt_codec_base::settings() const
{
    return m_settings;
}

inline
void fmt_codec_base::set_settings(const fmt_settings &sett)
{
    m_settings = sett;
}

inline
fmt_info fmt_codec_base::information() const
{
    return finfo;
}

inline
fmt_image* fmt_codec_base::image(const int index)
{
    return &finfo.image[index];
}

inline
void fmt_codec_base::addmeta(const fmt_metaentry &m)
{
    finfo.meta.push_back(m);
}

inline
void fmt_codec_base::settempfile(const std::string &t)
{
    tmp = t;
}

inline
std::string fmt_codec_base::extension(const s32)
{
    return std::string("");
}

inline
s32 fmt_codec_base::read_init(const std::string &)
{ return 1; }

inline
s32 fmt_codec_base::read_next()
{ return 1; }

inline
s32 fmt_codec_base::read_next_pass()
{ return 1; }

inline
s32 fmt_codec_base::read_scanline(RGBA *)
{ return 1; }

inline
void fmt_codec_base::read_close()
{}

inline
void fmt_codec_base::getwriteoptions(fmt_writeoptionsabs *)
{}

inline
s32 fmt_codec_base::write_init(const std::string &, const fmt_image &, const fmt_writeoptions &)
{ return 1; }

inline
s32 fmt_codec_base::write_next()
{ return 1; }

inline
s32 fmt_codec_base::write_next_pass()
{ return 1; }

inline
s32 fmt_codec_base::write_scanline(RGBA *)
{ return 1; }

inline
void fmt_codec_base::write_close()
{}

#define BASE_CODEC_DECLARATIONS \
	fmt_codec();            \
	~fmt_codec();           \
                                \
	virtual void    options(codec_options *o); \
	virtual s32	read_init(const std::string &file);\
	virtual s32	read_next();                       \
	virtual s32	read_next_pass();                  \
	virtual s32	read_scanline(RGBA *scan);         \
	virtual void	read_close();

#define WRITE_CODEC_DECLARATIONS  \
	virtual std::string extension(const s32 bpp);  \
	virtual void	getwriteoptions(fmt_writeoptionsabs *);  \
        virtual s32     write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt); \
        virtual s32     write_next();                \
        virtual s32     write_next_pass();           \
        virtual s32     write_scanline(RGBA *scan);  \
        virtual void    write_close();

#endif


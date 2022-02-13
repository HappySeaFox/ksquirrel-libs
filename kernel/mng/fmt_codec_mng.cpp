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

#include <iostream>

#include "fmt_types.h"
#include "fileio.h"

#include <libmng.h>

#include "fmt_codec_mng_defs.h"
#include "fmt_codec_mng.h"

#include "error.h"

/* structure for keeping track of our mng stream inside the callbacks */
struct mngstuff
{
    FILE        *file;    /* pointer to the file we're decoding */
    std::string filename; /* pointer to the file's path/name */
    fmt_codec   *codec;
};

/*
 *
 * MNG  (Multiple-image Network Graphics) is the animation extension of the popular PNG image-format.
 * PNG (Portable Network Graphics) is an extensible file format for the lossless, portable,
 * well-compressed  storage  of  raster images.
 *
 * MNG  has  advanced  animation features which make it very useful as a full replacement
 * for GIF animations. These features allow animations that are impossible with GIF or
 * result in much smaller files as GIF.
 *
 * As MNG builds on the same structure as PNG, it is robust, extensible and free of
 * patents. It  retains  the  same clever file integrity checks as in PNG.
 *
 * MNG also embraces the lossy JPEG image-format in a sub-format named JNG, which
 * allows for alpha-transparency and color-correction on highly compressed (photographic) images.
 *
 */

/* ******************************************************************** */
/* callbacks from mngplay.c  (C) Ralph Giles <giles@ashlu.bc.ca>                  */
/* ******************************************************************** */

/* memory allocation; data must be zeroed */
mng_ptr mymngalloc(mng_uint32 size)
{
    return (mng_ptr)calloc(1, size);
}

/* memory deallocation */
void mymngfree(mng_ptr p, mng_uint32 /*size*/)
{
    free(p);
}

mng_bool mymngopenstream(mng_handle mng)
{
    mngstuff*mymng;

    /* look up our stream struct */
    mymng = (mngstuff*)mng_get_userdata(mng);

    /* open the file */
    mymng->file = fopen(mymng->filename.c_str(), "rb");

    if(mymng->file == NULL)
        return MNG_FALSE;

    return MNG_TRUE;
}

mng_bool mymngprocesstext(mng_handle mng, mng_uint8 /*iType*/, mng_pchar  zKeyword, mng_pchar  zText,
                            mng_pchar /*zLanguage*/, mng_pchar /*zTranslation*/)
{
    mngstuff*mymng;

    /* look up our stream struct */
    mymng = (mngstuff*)mng_get_userdata(mng);

    fmt_metaentry mt;

    mt.group = zKeyword;
    mt.data = zText;

    mymng->codec->addMeta(mt);

    return MNG_TRUE;
}

mng_bool mymngclosestream(mng_handle mng)
{
    mngstuff *mymng;

    /* look up our stream struct */
    mymng = (mngstuff *)mng_get_userdata(mng);

    /* close the file */
    fclose(mymng->file);
    mymng->file = NULL;/* for safety */

    mymng->filename.clear();

    return MNG_TRUE;
}

/* feed data to the decoder */
mng_bool mymngreadstream(mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32 *bytesread)
{
    mngstuff *mymng;

    /* look up our stream struct */
    mymng = (mngstuff *)mng_get_userdata(mng);

    /* read the requested amount of data from the file */
    *bytesread = fread(buffer, 1, size, mymng->file);

    return MNG_TRUE;
}

/* the header's been read. set up the display stuff */
mng_bool mymngprocessheader(mng_handle mng, mng_uint32 width, mng_uint32 height)
{
    mngstuff *mymng;

    mymng = (mngstuff *)mng_get_userdata(mng);

    mymng->codec->priv.w = width;

    mymng->codec->priv.frame = new RGBA [width * height];

    return (!mymng->codec->priv.frame) ? MNG_FALSE : MNG_TRUE;
}

/* return a row pointer for the decoder to fill */
mng_ptr mymnggetcanvasline(mng_handle mng, mng_uint32 line)
{
    mngstuff *mymng;
    mng_ptr row;

    mymng = (mngstuff *)mng_get_userdata(mng);

    row = mymng->codec->priv.frame + line * mymng->codec->priv.w;

    return row;
}

/* timer */
mng_uint32 mymnggetticks(mng_handle /*mng*/)
{
    return 0;
}

mng_bool mymngrefresh(mng_handle /*mng*/, mng_uint32 /*x*/, mng_uint32 /*y*/, mng_uint32 /*w*/, mng_uint32 /*h*/)
{
    return MNG_TRUE;
}

/* interframe delay callback */
mng_bool mymngsettimer(mng_handle mng, mng_uint32 msecs)
{
    mngstuff *mymng;

    mymng = (mngstuff *)mng_get_userdata(mng);

    mymng->codec->priv.ms = msecs;

    return MNG_TRUE;
}

/* ******************************************************************** */

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("0.3.3");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("Multiple Network Graphics");
}

std::string fmt_codec::fmt_filter()
{

// Do we have JNG support ?
#ifdef MNG_INCLUDE_JNG
    return std::string("*.mng *.jng ");
#else
    return std::string("*.mng ");
#endif

}

std::string fmt_codec::fmt_mime()
{
    return std::string();
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,0,0,0,76,76,76,194,78,26,174,174,174,176,176,176,177,177,177,200,200,200,221,221,221,243,243,243,255,255,255,69,69,69,245,92,21,237,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,84,73,68,65,84,8,215,99,16,4,1,7,6,6,6,177,180,180,180,196,98,16,99,230,204,153,137,237,1,80,134,160,0,132,145,150,6,100,72,173,130,0,6,33,37,48,208,98,16,154,164,169,57,73,115,38,144,49,115,166,166,166,38,84,4,8,48,24,32,53,48,93,112,115,64,150,130,77,102,4,187,67,128,1,0,20,80,34,197,108,8,4,202,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    frs.close();

    currentImage = -1;
    read_error = false;

    finfo.animated = false;

    /* allocate our stream data structure */
    mymng = new mngstuff;

    if(!mymng)
        return SQE_R_NOMEMORY;

    mymng->filename = file;
    mymng->codec = this;
    priv.frame = NULL;
    priv.ms = 10;

    /* set up the mng decoder for our stream */
    mng = mng_initialize(mymng, mymngalloc, mymngfree, MNG_NULL);

    if (mng == MNG_NULL)
        return SQE_R_NOMEMORY;

    /* set the callbacks */
    mng_setcb_openstream(mng, ::mymngopenstream);
    mng_setcb_closestream(mng, ::mymngclosestream);
    mng_setcb_readdata(mng, ::mymngreadstream);
    mng_setcb_gettickcount(mng, ::mymnggetticks);
    mng_setcb_settimer(mng, ::mymngsettimer);
    mng_setcb_processheader(mng, ::mymngprocessheader);
    mng_setcb_getcanvasline(mng, ::mymnggetcanvasline);
    mng_setcb_refresh(mng, ::mymngrefresh);
    mng_setcb_processtext(mng, ::mymngprocesstext);
    mng_set_suspensionmode(mng, MNG_TRUE);
    mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8);

    total = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if((total && currentImage == total) || (!total && currentImage))
        return SQE_NOTOK;

    int myretcode;

    if(!currentImage)
    {
        myretcode = mng_read(mng);

        if(myretcode != MNG_NOERROR)
            return SQE_R_BADFILE;

        total = mng_get_totallayers(mng);

        if(total > 1) total--;

        myretcode = mng_display(mng);

        if(myretcode != MNG_NOERROR && myretcode != MNG_NEEDTIMERWAIT)
            return SQE_R_BADFILE;
    }
    else
    {
        myretcode = mng_display_resume(mng);

        if(myretcode != MNG_NOERROR && myretcode != MNG_NEEDTIMERWAIT)
            return SQE_R_BADFILE;

        finfo.animated = true;
    }

    fmt_image image;

    image.w = mng_get_imagewidth(mng);
    image.h = mng_get_imageheight(mng);
    image.bpp = 32;
    image.compression = (mng_get_imagetype(mng) == MNG_IMAGETYPE_PNG) ? "Deflate method 8, 32K window" : "JPEG";
    image.hasalpha = true;

    int cc = mng_get_colortype(mng);

    switch(cc)
    {
        case MNG_COLORTYPE_GRAY:     image.colorspace = "Grayscale";            break;
        case MNG_COLORTYPE_RGB:      image.colorspace = "RGB";                  break;
        case MNG_COLORTYPE_INDEXED:  image.colorspace = "Indexed";              break;
        case MNG_COLORTYPE_GRAYA:    image.colorspace = "Grayscale with alpha"; break;
        case MNG_COLORTYPE_RGBA:     image.colorspace = "RGBA";                 break;

        default: image.colorspace = "Unknown";
    }

    image.delay = priv.ms;

    finfo.image.push_back(image);

    line = -1;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    line++;
    fmt_image *im = image(currentImage);

    memcpy(scan, priv.frame+im->w*line, im->w*sizeof(RGBA));

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    finfo.meta.clear();
    finfo.image.clear();

    mng_cleanup(&mng);

    if(priv.frame)
    {
        delete priv.frame;
        priv.frame = NULL;
    }
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->passes = 1;
    opt->compression_scheme = CompressionNo;
    opt->compression_min = 0;
    opt->compression_max = 0;
    opt->compression_def = 0;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
}

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    if(!image.w || !image.h || file.empty())
        return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    fws.open(file.c_str(), ios::binary | ios::out);

    if(!fws.good())
        return SQE_W_NOFILE;

    return SQE_OK;
}

s32 fmt_codec::fmt_write_next()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::fmt_write_scanline(RGBA * /*scan*/)
{
    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    fws.close();
}

bool fmt_codec::fmt_writable() const
{
    return false;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("");
}

#include "fmt_codec_cd_func.h"
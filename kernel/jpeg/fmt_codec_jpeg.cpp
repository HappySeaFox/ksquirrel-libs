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

#include <csetjmp>
#include <iostream>
#include <cstdio>

#include "fmt_types.h"
#include "fileio.h"
#include "error.h"

#include "fmt_codec_jpeg_defs.h"
#include "fmt_codec_jpeg.h"

/*
 *
 * JPEG (Joint Photographic Experts Group) refers to a
 * standards organization, a method of file compression, and sometimes a
 * file format. In fact, the JPEG specification
 * itself does not define a common file interchange
 * format to store and transport JPEG data between
 * computer platforms and operating systems. The JPEG
 * File Interchange Format (JFIF) is a development of
 * C-Cube Microsystems for the purpose of storing
 * JPEG-encoded data. JFIF is
 * designed to allow files containing JPEG-encoded
 * data streams to be exchanged between otherwise incompatible systems
 * and applications.
 *
 */

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr;

    cerr << "libSQ_read_jpeg: JPEG Error!" << endl;

    myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

std::string fmt_codec::fmt_version()
{
    return std::string("1.3.3");
}

std::string fmt_codec::fmt_quickinfo()
{
    return std::string("JPEG compressed");
}

std::string fmt_codec::fmt_filter()
{
    return std::string("*.jpg *.jpeg *.jpe ");
}

std::string fmt_codec::fmt_mime()
{
    return std::string("\x00FF\x00D8\x00FF");
}

std::string fmt_codec::fmt_pixmap()
{
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,33,80,76,84,69,207,0,8,176,176,176,200,200,200,221,221,221,174,174,174,255,255,255,243,243,243,177,177,177,69,69,69,128,128,0,76,76,76,137,239,99,177,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,93,73,68,65,84,120,218,99,88,5,2,2,12,12,12,139,148,148,148,180,76,64,140,208,208,80,173,228,2,40,99,213,2,8,67,73,9,200,88,209,1,1,12,43,103,130,193,12,134,149,83,167,134,206,156,26,10,102,204,140,140,132,137,68,2,69,150,2,69,102,70,78,5,50,34,129,12,176,26,152,46,184,57,32,75,193,38,115,129,221,177,128,1,0,131,30,58,190,241,2,42,229,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_read_init(const std::string &file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next()
{
    currentImage++;

    if(currentImage)
	return SQE_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if(setjmp(jerr.setjmp_buffer)) 
    {
//        printf("JUMP!!\n");
	jpeg_destroy_decompress(&cinfo);
	fclose(fptr);
	return SQE_NOTOK;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fptr);
    jpeg_save_markers(&cinfo, JPEG_COM, 0xffff);
    jpeg_read_header(&cinfo, TRUE);

    if(cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        cinfo.out_color_space = JCS_RGB;
	cinfo.desired_number_of_colors = 256;
	cinfo.quantize_colors = FALSE;
	cinfo.two_pass_quantize = FALSE;
    }

    jpeg_start_decompress(&cinfo);

    finfo.image[currentImage].w = cinfo.output_width;
    finfo.image[currentImage].h = cinfo.output_height;
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    std::string type;

    switch(cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE: type =  "Grayscale"; finfo.image[currentImage].bpp = 8;  break;
	case JCS_RGB:       type =  "RGB";       finfo.image[currentImage].bpp = 24; break;
	case JCS_YCbCr:     type =  "YUV";       finfo.image[currentImage].bpp = 24; break;
	case JCS_CMYK:      type =  "CMYK";      finfo.image[currentImage].bpp = 32; break;
	case JCS_YCCK:      type =  "YCCK";      finfo.image[currentImage].bpp = 32; break;

	default:
	    type = "Unknown";
    }

    finfo.image[currentImage].compression = "JPEG";
    finfo.image[currentImage].colorspace = type;

    jpeg_saved_marker_ptr it = cinfo.marker_list;

    bool rr = false;

    while(true)
    {
        if(!it)
	    break;

	if(it->marker == JPEG_COM)
	{
	    finfo.meta.push_back(fmt_metaentry());

	    finfo.meta[0].group = "JPEG \"COM\" marker";

	    s8 data[it->data_length+1];
	    memcpy(data, it->data, it->data_length);
	    data[it->data_length] = '\0';
//	    memcpy(data, it->data, it->data_length);
	    finfo.meta[0].data = data;

	    rr = true;
    	    break;
	}

	it = it->next;
    }

    if(!rr)
    {
	finfo.meta.clear();
    }

    finfo.images++;

    return SQE_OK;
}

s32 fmt_codec::fmt_read_next_pass()
{
    return SQE_OK;
}
	
s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 i;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    for(i = 0;i < finfo.image[0].w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

    return SQE_OK;
}

void fmt_codec::fmt_read_close()
{
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(fptr);
}

void fmt_codec::fmt_getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 100;
    opt->compression_def = 25;
    opt->passes = 1;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
}

s32 fmt_codec::fmt_write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    if(!image.w || !image.h || file.empty())
	return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
        return SQE_W_NOFILE;

    m_cinfo.err = jpeg_std_error(&m_jerr);

    jpeg_create_compress(&m_cinfo);

    jpeg_stdio_dest(&m_cinfo, m_fptr);

    m_cinfo.image_width = image.w;
    m_cinfo.image_height = image.h;
    m_cinfo.input_components = 3;
    m_cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&m_cinfo);

    jpeg_set_quality(&m_cinfo, 100-opt.compression_level, true);

    jpeg_start_compress(&m_cinfo, true);

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

s32 fmt_codec::fmt_write_scanline(RGBA *scan)
{
    RGB sr[writeimage.w];

    for(s32 s = 0;s < writeimage.w;s++)
    {
        memcpy(sr+s, scan+s, sizeof(RGB));
    }

    row_pointer = (JSAMPLE *)sr;

    (void)jpeg_write_scanlines(&m_cinfo, &row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::fmt_write_close()
{
    jpeg_finish_compress(&m_cinfo);

    fclose(m_fptr);

    jpeg_destroy_compress(&m_cinfo);
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

bool fmt_codec::fmt_readable() const
{
    return true;
}

std::string fmt_codec::fmt_extension(const s32 /*bpp*/)
{
    return std::string("jpeg");
}

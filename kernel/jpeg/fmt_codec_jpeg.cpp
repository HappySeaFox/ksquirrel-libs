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


#include <setjmp.h>
#include <sstream>
#include <iostream>
#include <cstdio>

#include "fmt_types.h"
#include "fmt_codec_jpeg_defs.h"
#include "fmt_codec_jpeg.h"

#include "error.h"

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
    return std::string("137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,4,132,132,4,4,4,192,192,192,255,255,255,0,0,0,128,128,0,134,134,134,79,212,14,132,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,1,98,75,71,68,0,136,5,29,72,0,0,0,9,112,72,89,115,0,0,11,17,0,0,11,17,1,127,100,95,145,0,0,0,7,116,73,77,69,7,212,10,17,19,35,2,169,169,215,166,0,0,0,82,73,68,65,84,120,156,61,142,193,13,192,48,8,3,89,33,125,244,15,43,88,108,144,76,64,217,127,149,18,135,214,175,147,101,108,68,100,108,73,233,50,51,37,0,208,209,80,214,108,201,74,234,145,5,71,134,23,68,186,123,18,80,120,156,244,216,153,2,102,190,171,191,231,52,67,123,148,171,124,227,126,1,202,77,25,76,179,115,138,216,0,0,0,0,73,69,78,68,174,66,96,130");
}

s32 fmt_codec::fmt_init(std::string file)
{
    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
        return SQERR_NOFILE;

    currentImage = -1;

    finfo.animated = false;
    finfo.images = 0;

    return SQERR_OK;
}

s32 fmt_codec::fmt_next()
{
    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    finfo.image.push_back(fmt_image());

    finfo.image[currentImage].passes = 1;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if(setjmp(jerr.setjmp_buffer)) 
    {
//        printf("JUMP!!\n");
	jpeg_destroy_decompress(&cinfo);
	fclose(fptr);
	return SQERR_NOTOK;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fptr);
    jpeg_save_markers(&cinfo, JPEG_COM, 0xffff);
    jpeg_read_header(&cinfo, TRUE);

    if(cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
	finfo.image[currentImage].bpp = 8;
        cinfo.out_color_space = JCS_RGB;
	cinfo.desired_number_of_colors = 256;
	cinfo.quantize_colors = FALSE;
	cinfo.two_pass_quantize = FALSE;
    }
    else
	finfo.image[currentImage].bpp = 24;

    jpeg_start_decompress(&cinfo);

    finfo.image[currentImage].w = cinfo.output_width;
    finfo.image[currentImage].h = cinfo.output_height;
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    stringstream type;

    switch(cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE:   type <<  "Grayscale (black&white)"; break;
	case JCS_RGB:         type <<  "RGB";                     break;
	case JCS_YCbCr:       type <<  "YUV";                     break;
	case JCS_CMYK:        type <<  "CMYK";                    break;
	case JCS_YCCK:        type <<  "YCCK";                    break;

	default:
	    type <<  "Unknown";
    }

    s32 bytes = finfo.image[currentImage].w * finfo.image[currentImage].h * sizeof(RGBA);

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << finfo.image[currentImage].w << "x"
        << finfo.image[currentImage].h << "\n"
        << finfo.image[currentImage].bpp << "\n"
        << type.str() << "\n"
        << "JPEG\n"
        << bytes;

    finfo.image[currentImage].dump = s.str();

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

    return SQERR_OK;
}

s32 fmt_codec::fmt_next_pass()
{
    return SQERR_OK;
}
	
s32 fmt_codec::fmt_read_scanline(RGBA *scan)
{
    s32 i;

    memset(scan, 255, finfo.image[currentImage].w * sizeof(RGBA));

    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    for(i = 0;i < finfo.image[0].w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

    return SQERR_OK;
}

s32 fmt_codec::fmt_readimage(std::string file, RGBA **image, std::string &dump)
{
    struct jpeg_decompress_struct	m_cinfo;
    struct my_error_mgr 		m_jerr;
    s32 				i, j;
    JSAMPARRAY 				m_buffer;
    FILE				*m_fptr;
    s32					w, h, bpp;
    s32 				m_bytes;

    m_fptr = fopen(file.c_str(), "rb");

    if(!m_fptr)
	return SQERR_NOFILE;

    m_cinfo.err = jpeg_std_error(&m_jerr.pub);
    m_jerr.pub.error_exit = my_error_exit;

    if(setjmp(m_jerr.setjmp_buffer)) 
    {
	jpeg_destroy_decompress(&m_cinfo);
	fclose(m_fptr);
	return SQERR_NOTOK;
    }

    jpeg_create_decompress(&m_cinfo);
    jpeg_stdio_src(&m_cinfo, m_fptr);
    jpeg_read_header(&m_cinfo, TRUE);

    if(m_cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
	bpp = 8;
        m_cinfo.out_color_space = JCS_RGB;
	m_cinfo.desired_number_of_colors = 256;
	m_cinfo.quantize_colors = FALSE;
	m_cinfo.two_pass_quantize = FALSE;
    }
    else
	bpp = 24;

    jpeg_start_decompress(&m_cinfo);

    w = m_cinfo.output_width;
    h = m_cinfo.output_height;

    m_buffer = (*m_cinfo.mem->alloc_sarray)((j_common_ptr)&m_cinfo, 
	    JPOOL_IMAGE, m_cinfo.output_width * m_cinfo.output_components, 1);
	    
    stringstream m_type;

    switch(m_cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE:  m_type << "Grayscale (black&white)"; break;
	case JCS_RGB:        m_type << "RGB";                     break;
	case JCS_YCbCr:      m_type << "YUV";                     break;
	case JCS_CMYK:       m_type << "CMYK";                    break;
	case JCS_YCCK:       m_type << "YCCK";                    break;

	default:
	    m_type << "Unknown";
    }

    m_bytes = w * h * sizeof(RGBA);    

    stringstream s;

    s   << fmt_quickinfo() << "\n"
        << w << "\n"
        << h << "\n"
        << bpp << "\n"
        << m_type.str() << "\n"
        << "JPEG" << "\n"
        << 1 << "\n"
        << m_bytes;

    dump = s.str();

    *image = (RGBA*)realloc(*image, m_bytes);
    
    if(!*image)
    {
	fclose(m_fptr);
	return SQERR_NOMEMORY;
    }

    memset(*image, 255, m_bytes);

    i = 0;

    while(m_cinfo.output_scanline < m_cinfo.output_height)
    {
	(void)jpeg_read_scanlines(&m_cinfo, m_buffer, 1);

        for(j = 0;j < w;j++)
	    memcpy(*image + i*w + j, m_buffer[0] + j*3, 3);

	i++;
    }

    (void)jpeg_finish_decompress(&m_cinfo);
    jpeg_destroy_decompress(&m_cinfo);
    fclose(m_fptr);

    return SQERR_OK;
}

void fmt_codec::fmt_close()
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
}

s32 fmt_codec::fmt_writeimage(std::string file, RGBA *image, s32 w, s32 h, const fmt_writeoptions &opt)
{
    FILE 	*m_fptr;
		    
    if(!image || !w || !h)
	return SQERR_NOMEMORY;
			
    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
        return SQERR_NOFILE;

    struct jpeg_compress_struct m_cinfo;
    struct jpeg_error_mgr m_jerr;

    JSAMPROW row_pointer[1];

    m_cinfo.err = jpeg_std_error(&m_jerr);

    jpeg_create_compress(&m_cinfo);

    jpeg_stdio_dest(&m_cinfo, m_fptr);

    J_COLOR_SPACE cs = JCS_RGB;

    m_cinfo.image_width = w;
    m_cinfo.image_height = h;
    m_cinfo.input_components = 3;
    m_cinfo.in_color_space = cs;

    jpeg_set_defaults(&m_cinfo);

    jpeg_set_quality(&m_cinfo, 100-opt.compression_level, true);

    jpeg_start_compress(&m_cinfo, true);

    RGB scan[w];
    RGBA *srgba;
    s32 line = 0;

    while(m_cinfo.next_scanline < m_cinfo.image_height)
    {
	srgba = image + line * w;

	for(s32 s = 0;s < w;s++)
	{
	    memcpy(scan+s, srgba + s, sizeof(RGB));
	}

	row_pointer[0] = (JSAMPLE *)scan;

	(void)jpeg_write_scanlines(&m_cinfo, row_pointer, 1);

	line++;
    }

    jpeg_finish_compress(&m_cinfo);

    fclose(m_fptr);

    jpeg_destroy_compress(&m_cinfo);

    return SQERR_OK;
}

bool fmt_codec::fmt_writable() const
{
    return true;
}

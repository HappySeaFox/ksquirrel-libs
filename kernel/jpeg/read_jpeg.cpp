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
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "read_jpeg.h"

struct jpeg_decompress_struct	cinfo;
struct my_error_mgr 		jerr;
int 				row_stride;
JSAMPARRAY 			buffer;
FILE				*fptr;
int				currentImage, bytes;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr;

    printf("libSQ_read_jpeg: JPEG Error!\n");

    myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

const char* fmt_version()
{
    return (const char*)"1.1.3";
}

const char* fmt_quickinfo()
{
    return (const char*)"JPEG compressed";
}

const char* fmt_filter()
{
    return (const char*)"*.jpg *.jpeg *.jpe ";
}

const char* fmt_mime()
{
/*  QRegExp pattern  */
    return (const char*)"\x00FF\x00D8\x00FF";
}

const char* fmt_pixmap()
{
    return (const char*)"137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,4,3,0,0,0,237,221,226,82,0,0,0,21,80,76,84,69,4,132,132,4,4,4,192,192,192,255,255,255,0,0,0,128,128,0,134,134,134,79,212,14,132,0,0,0,1,116,82,78,83,0,64,230,216,102,0,0,0,1,98,75,71,68,0,136,5,29,72,0,0,0,9,112,72,89,115,0,0,11,17,0,0,11,17,1,127,100,95,145,0,0,0,7,116,73,77,69,7,212,10,17,19,35,2,169,169,215,166,0,0,0,82,73,68,65,84,120,156,61,142,193,13,192,48,8,3,89,33,125,244,15,43,88,108,144,76,64,217,127,149,18,135,214,175,147,101,108,68,100,108,73,233,50,51,37,0,208,209,80,214,108,201,74,234,145,5,71,134,23,68,186,123,18,80,120,156,244,216,153,2,102,190,171,191,231,52,67,123,148,171,124,227,126,1,202,77,25,76,179,115,138,216,0,0,0,0,73,69,78,68,174,66,96,130";
}

int fmt_init(fmt_info *finfo, const char *file)
{
    if(!finfo)
        return SQERR_NOMEMORY;

    fptr = fopen(file, "rb");

    if(!fptr)
        return SQERR_NOFILE;

    currentImage = -1;
    finfo->passes = 1;
	
    return SQERR_OK;
}

int fmt_next(fmt_info *finfo)
{
    char type[30];
    
    currentImage++;

    if(currentImage)
	return SQERR_NOTOK;

    if(!finfo->image)
        return SQERR_NOMEMORY;

    memset(&finfo->image[currentImage], 0, sizeof(fmt_image));

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    
    if(setjmp(jerr.setjmp_buffer)) 
    {
        printf("JUMP!!\n");
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
	finfo->image[currentImage].bpp = 8;
        cinfo.out_color_space = JCS_RGB;
	cinfo.desired_number_of_colors = 256;
	cinfo.quantize_colors = FALSE;
	cinfo.two_pass_quantize = FALSE;
    }
    else
	finfo->image[currentImage].bpp = 24;

    jpeg_start_decompress(&cinfo);

    finfo->image[currentImage].w = cinfo.output_width;
    finfo->image[currentImage].h = cinfo.output_height;
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    switch(cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE: sprintf(type, "%s", "Grayscale (black&white)");break;
	case JCS_RGB: sprintf(type, "%s", "RGB");break;
	case JCS_YCbCr: sprintf(type, "%s", "YUV");break;
	case JCS_CMYK: sprintf(type, "%s", "CMYK");break;
	case JCS_YCCK: sprintf(type, "%s", "YCCK");break;

	default:
	    sprintf(type, "%s", "Unknown");
    }

    bytes = finfo->image[currentImage].w * finfo->image[currentImage].h * sizeof(RGBA);

    asprintf(&finfo->image[currentImage].dump, "%s\n%dx%d\n%d\n%s\nJPEG\n%d\n",
	fmt_quickinfo(),
	finfo->image[currentImage].w,
	finfo->image[currentImage].h,
	finfo->image[currentImage].bpp,
	type,
	bytes);

    row_stride = cinfo.output_width * cinfo.output_components;

    jpeg_saved_marker_ptr it = cinfo.marker_list;

    finfo->image[currentImage].meta = (fmt_metainfo *)calloc(1, sizeof(fmt_metainfo));

    bool rr = false;

    if(finfo->image[currentImage].meta)
    {
	while(1)
        {
    	    if(!it)
		break;

	    if(it->marker == JPEG_COM)
	    {
		finfo->image[currentImage].meta->entries++;
		finfo->image[currentImage].meta->m = (fmt_meta_entry *)calloc(1, sizeof(fmt_meta_entry));
		fmt_meta_entry *entry = finfo->image[currentImage].meta->m;

		if(entry)
		{
		    entry[currentImage].datalen = it->data_length;
		    strcpy(entry[currentImage].group, "JPEG \"COM\" marker");
		    entry[currentImage].data = (char *)malloc(it->data_length);

		    if(entry->data)
			memcpy(entry[currentImage].data, it->data, it->data_length);
		}

		rr = true;

		break;
	    }

	    it = it->next;
	}
    }
    
    if(!rr && finfo->image[currentImage].meta)
    {
	free(finfo->image[currentImage].meta);
	finfo->image[currentImage].meta = 0;
    }

    finfo->images++;

    return SQERR_OK;

}

int fmt_next_pass(fmt_info *)
{
    return SQERR_OK;
}
	
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    int i;

    memset(scan, 255, finfo->image[currentImage].w * sizeof(RGBA));

    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    for(i = 0;i < finfo->image[0].w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

    return SQERR_OK;
}

int fmt_readimage(const char *file, RGBA **image, char **dump)
{
    struct jpeg_decompress_struct	m_cinfo;
    struct my_error_mgr 		m_jerr;
    int 				m_row_stride;
    int 				i, j;
    JSAMPARRAY 				m_buffer;
    char 				m_type[25];
    FILE				*m_fptr;
    int					w, h, bpp;

    m_fptr = fopen(file, "rb");

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

    switch(m_cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE: sprintf(m_type, "%s", "Grayscale (black&white)");break;
	case JCS_RGB: sprintf(m_type, "%s", "RGB");break;
	case JCS_YCbCr: sprintf(m_type, "%s", "YUV");break;
	case JCS_CMYK: sprintf(m_type, "%s", "CMYK");break;
	case JCS_YCCK: sprintf(m_type, "%s", "YCCK");break;

	default:
	    sprintf(m_type, "%s", "Unknown");
    }

    const int m_bytes = w * h * sizeof(RGBA);    

    asprintf(dump, "%s\n%d\n%d\n%d\n%s\nJPEG\n%d\n%d\n",
    fmt_quickinfo(),
    w,
    h,
    bpp,
    m_type,
    1,
    m_bytes);

    m_row_stride = m_cinfo.output_width * m_cinfo.output_components;

    *image = (RGBA*)realloc(*image, m_bytes);
    
    if(!*image)
    {
	fprintf(stderr, "libSQ_read_jpeg: Image is null!\n");
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

int fmt_close()
{
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(fptr);

    return SQERR_OK;
}

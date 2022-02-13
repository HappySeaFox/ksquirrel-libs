#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "read_png.h"

#include "png.h"

#ifndef png_jmpbuf
    #define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif


int fmt_readable()
{
    return SQERR_OK;
}

char* fmt_version()
{
    return "0.9.1";
}

char* fmt_quickinfo()
{
    return "Portable Network Graphics";
}

char* fmt_extension()
{
    return "*.png";
}

int fmt_writeable()
{
    return SQERR_NOTOK;
}

/* inits decoding of 'file': opens it, fills struct fmt_info  */
int fmt_init(fmt_info **finfo, const char *file)
{
    *finfo = (fmt_info*)calloc(1, sizeof(fmt_info));
    
    if(!finfo)
	return SQERR_NOMEMORY;
	
    (*finfo)->w = 0;
    (*finfo)->h = 0;
    (*finfo)->bpp = 0;
    (*finfo)->hasalpha = FALSE;

    (*finfo)->fptr = fopen(file, "rb");
    
    if(!((*finfo)->fptr))
    {
	free(*finfo);
	return SQERR_NOFILE;
    }
    
    return SQERR_OK;
}


#define PNG_BYTES_TO_CHECK 4

png_structp	png_ptr;
png_infop	info_ptr;
png_uint_32	width, height, number_passes;
int		color_type;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    char	buf[PNG_BYTES_TO_CHECK+1];
    int		bit_depth, interlace_type;

    if(fread(buf, 1, PNG_BYTES_TO_CHECK, finfo->fptr) != PNG_BYTES_TO_CHECK)
	return SQERR_BADFILE;

    if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == NULL)
    {
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
	fclose(finfo->fptr);
	png_destroy_read_struct(&png_ptr, 0, 0);
	return SQERR_NOMEMORY;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(finfo->fptr);
	return SQERR_NOMEMORY;
    }

    png_init_io(png_ptr, finfo->fptr);
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, (int*)NULL, (int*)NULL);

    finfo->w = width;
    finfo->h = height;
    finfo->bpp = bit_depth;

    if(finfo->bpp == 16)
	png_set_strip_16(png_ptr);

    if(finfo->bpp < 8)
	png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && finfo->bpp < 8)
	png_set_gray_1_2_4_to_8(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
    {
	png_set_palette_to_rgb(png_ptr);
	png_get_PLTE(png_ptr, info_ptr, (png_colorp*)&finfo->pal, (int*)&finfo->pal_entr);
    }
    else
	finfo->pal = 0;
	
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb(png_ptr);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(png_ptr);

    number_passes = png_set_interlace_handling(png_ptr);
    
    png_read_update_info(png_ptr, info_ptr);

    return SQERR_OK;
}


/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    png_bytep rows[1];
    int i;

    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    rows[0] = png_malloc(png_ptr, row_bytes);
    
    png_read_rows(png_ptr, &rows[0], png_bytepp_NULL, 1);
    
    if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	memcpy(scan, rows[0], finfo->w * 4);

    else
	for(i = 0;i < finfo->w;i++)
	    memcpy(scan+i, rows[0]+i*3, 3);

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fclose(finfo->fptr);

    return SQERR_OK;
}

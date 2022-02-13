#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "read_tiff.h"

#include <tiffio.h>


char* fmt_version()
{
    return "0.9";
}

char* fmt_quickinfo()
{
    return "Tagged Image File Format";
}

char* fmt_extension()
{
    return "*.tif *.tiff";
}


TIFF	*ftiff;
int	i;
uint32	*buf;
TIFFRGBAImage img;

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
    
    if((ftiff = TIFFOpen(file,"r")) == NULL)
	return SQERR_BADFILE;

    TIFFSetWarningHandler(NULL);

    return SQERR_OK;
}



/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    int bps, spp;

    TIFFGetField(ftiff, TIFFTAG_IMAGEWIDTH, &finfo->w);
    TIFFGetField(ftiff, TIFFTAG_IMAGELENGTH, &finfo->h);
    TIFFGetField(ftiff, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(ftiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
    
    finfo->bpp = (long)bps * spp;
    
    i = 1;

    TIFFRGBAImageBegin(&img, ftiff, 1, 0);

    buf = (uint32*)calloc(finfo->w, 4);

    return SQERR_OK;
}



/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
 
int fmt_read_scanline(fmt_info *finfo, RGBA *scan)
{
    TIFFRGBAImageGet(&img, buf, finfo->w, 1);

    memcpy(scan, buf, finfo->w * 4);

    img.row_offset++;

    return SQERR_OK;
}

int fmt_close(fmt_info *finfo)
{
    free(buf);
    TIFFRGBAImageEnd(&img);

    fclose(finfo->fptr);

    TIFFClose(ftiff);

    return SQERR_OK;
}

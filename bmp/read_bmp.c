#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_bmp.h"


char* fmt_version()
{
    return "0.8.5";
}

char* fmt_quickinfo()
{
    return "Windows Bitmap";
}

char* fmt_extension()
{
    return "*.bmp *.dib";
}


/* inits decoding of 'file': opens it, inits struct fmt_info  */
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


short	FILLER;

/*  init info about file, e.g. width, height, bpp, alpha, 'fseek' to image bits  */
int fmt_read_info(fmt_info *finfo)
{
    BITMAPFILE_HEADER	bfh;
    BITMAPINFO_HEADER	bih;
    RGBA		rgba;
    long		i, j, scanShouldBe;
    

    fread(&bfh, sizeof(BITMAPFILE_HEADER), 1, finfo->fptr);
    fread(&bih, sizeof(BITMAPINFO_HEADER), 1, finfo->fptr);

    if(bih.Size != 40)
    	return SQERR_BADFILE;

    if(bih.BitCount < 16)
    	finfo->pal_entr = 1 << bih.BitCount;
    else
	finfo->pal_entr = 0;

    scanShouldBe = finfo->w = bih.Width;
    finfo->h = bih.Height;
    finfo->bpp = bih.BitCount;

    switch(finfo->bpp)
    {
	case 1:
	{
	    long tmp = scanShouldBe;
	    scanShouldBe /= 8;
	    scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
	}
	break;
	
	case 4:  scanShouldBe = ((finfo->w)%2)?((scanShouldBe+1)/2):(scanShouldBe/2); break;
	case 8:  break;
	case 16: scanShouldBe *= 2; break;
	case 24: scanShouldBe *= 3; break;
	case 32: break;
    
    }
    
    for(j = 0;j < 4;j++)
	if((scanShouldBe+j)%4 == 0) 
	{
	    FILLER = j;
	    break;
	}

    if(finfo->bpp < 16)
    {
	if((finfo->pal = (RGB*)calloc(finfo->pal_entr, sizeof(RGB))) == 0)
	{
		fclose(finfo->fptr);
		free(finfo);
		return SQERR_NOMEMORY;
	}

	/*  read palette  */
	for(i = 0;i < finfo->pal_entr;i++)
	{
		fread(&rgba, sizeof(RGBA), 1, finfo->fptr);
		(finfo->pal)[i].r = rgba.b;
		(finfo->pal)[i].g = rgba.g;
		(finfo->pal)[i].b = rgba.r;
		
	}
    }
    else
	finfo->pal = 0;

    /*  fseek to image bits  */
    fseek(finfo->fptr, bfh.OffBits, SEEK_SET);


    return SQERR_OK;
}



/*  
 *    reads scanline
 *    scan should exist, e.g. RGBA scan[N], not RGBA *scan  
 */
int fmt_read_scanline(fmt_info *finfo, RGBA scan[])
{

    long remain, scanShouldBe, j, counter = 0;
    unsigned char bt;
    
    switch(finfo->bpp)
    {
    	case 1:
	{
		unsigned char	index;
		remain=((finfo->w)<=8)?(0):((finfo->w)%8);
		scanShouldBe = finfo->w;;

		long tmp = scanShouldBe;
		scanShouldBe /= 8;
		scanShouldBe = scanShouldBe + ((tmp%8)?1:0);
 
		// @todo get rid of miltiple 'if'
		for(j = 0;j < scanShouldBe;j++)
		{
			fread(&bt, 1, 1, finfo->fptr);
			if(j==scanShouldBe-1 && (remain-0)<=0 && remain)break; index = (bt & 128) >> 7; memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-1)<=0 && remain)break; index = (bt & 64) >> 6;  memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-2)<=0 && remain)break; index = (bt & 32) >> 5;  memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-3)<=0 && remain)break; index = (bt & 16) >> 4;  memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-4)<=0 && remain)break; index = (bt & 8) >> 3;   memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-5)<=0 && remain)break; index = (bt & 4) >> 2;   memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-6)<=0 && remain)break; index = (bt & 2) >> 1;   memcpy(scan+(counter++), (finfo->pal)+index, 3);
			if(j==scanShouldBe-1 && (remain-7)<=0 && remain)break; index = (bt & 1);        memcpy(scan+(counter++), (finfo->pal)+index, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(finfo->fptr);
	}
	break;

	case 4:
	{
		unsigned char	index;
		remain = (finfo->w)%2;

		int ck = (finfo->w%2)?(finfo->w + 1):(finfo->w);
		ck /= 2;

		for(j = 0;j < ck-1;j++)
		{
			fread(&bt, 1, 1, finfo->fptr);
			index = (bt & 0xf0) >> 4;
			memcpy(scan+(counter++), (finfo->pal)+index, 3);
			index = bt & 0xf;
			memcpy(scan+(counter++), (finfo->pal)+index, 3);
		}

		fread(&bt, 1, 1, finfo->fptr);
		index = (bt & 0xf0) >> 4;
		memcpy(scan+(counter++), (finfo->pal)+index, 3);

		if(!remain)
		{
			index = bt & 0xf;
			memcpy(scan+(counter++), (finfo->pal)+index, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(finfo->fptr);
	}
	break;

	case 8:
	{
		
		for(j = 0;j < finfo->w;j++)
		{
			fread(&bt, 1, 1, finfo->fptr);
			memcpy(scan+(counter++), (finfo->pal)+bt, 3);
		}

		for(j = 0;j < FILLER;j++) fgetc(finfo->fptr);
	}
	break;

	case 16:
	{
		unsigned short word;

		for(j = 0;j < finfo->w;j++)
		{
			fread(&word, 2, 1, finfo->fptr);
			scan[counter].b = (word&0x1f) << 3;
			scan[counter].g = ((word&0x3e0) >> 5) << 3;
			scan[counter++].r = ((word&0x7c00)>>10) << 3;
		}

		for(j = 0;j < FILLER;j++) fgetc(finfo->fptr);
	}
	break;

	case 24:
	{
		RGB rgb;

		for(j = 0;j < finfo->w;j++)
		{
			fread(&rgb, sizeof(RGB), 1, finfo->fptr);
			scan[counter].r = rgb.b;
			scan[counter].g = rgb.g;
			scan[counter++].b = rgb.r;
		}

		for(j = 0;j < FILLER;j++) fgetc(finfo->fptr);
	}
	break;

	case 32:
	{
		RGBA rgba;

		for(j = 0;j < finfo->w;j++)
		{
			fread(&rgba, sizeof(RGBA), 1, finfo->fptr);
			scan[j].r = rgba.b;
			scan[j].g = rgba.g;
			scan[j].b = rgba.r;
		}
	}
	break;

	default:
	{
		//@TODO:  free memory !!
		return SQERR_BADFILE;
	}
    }

    return SQERR_OK;
}


int fmt_close(fmt_info *finfo)
{
    fclose(finfo->fptr);

    return SQERR_OK;
}

/*
 *  jbgtopbm - JBIG to Portable Bitmap converter
 *
 *  Markus Kuhn -- http://www.cl.cam.ac.uk/~mgk25/jbigkit/
 *
 *  $Id: jbgtopbm.c,v 1.11 2004-06-11 15:17:49+01 mgk25 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "jbig.h"

double koeff = 1.0;

void write_it(unsigned char *data, size_t len, void *file)
{
    int bb;
    unsigned char cc;

    FILE *ff = (FILE *)file;

    for(size_t i = 0;i < len;i++)
    {
        bb = int(koeff * (*(data+i)));

        if(bb > 255) cc = 255;
        else cc = bb;

        fwrite(&cc, 1, 1, ff);
        fwrite(&cc, 1, 1, ff);
        fwrite(&cc, 1, 1, ff);
    }
}

int read_file(unsigned char **buf, size_t *buflen, size_t *len, FILE *f)
{
  if (*buflen == 0) {
    *buflen = 4000;
    *len = 0;
    *buf = (unsigned char *) malloc(*buflen);
    if (!*buf) {
      fprintf(stderr, "Sorry, not enough memory available!\n");
      return 0;
    }
  }
  do {
    *len += fread(*buf + *len, 1, *buflen - *len, f);
    if (*len == *buflen) {
      *buflen *= 2;
      *buf = (unsigned char *) realloc(*buf, *buflen);
      if (!*buf) {
	fprintf(stderr, "Sorry, not enough memory available!\n");
	return 0;
      }
    }
    if (ferror(f)) {
      perror("Problem while reading input file");
      return 0;
    }
  } while (!feof(f));
  *buflen = *len;
  *buf = (unsigned char *) realloc(*buf, *buflen);
  if (!*buf) {
    fprintf(stderr, "Oops, realloc failed when shrinking buffer!\n");
    return 0;
  }
  
  return 1;
}


int call (char **argv)
{
  FILE *fin = stdin, *fout = stdout;
  const char *fnin = NULL, *fnout = NULL;
  int i, result;
  struct jbg_dec_state s;
  unsigned char *buffer, *p;
  size_t buflen, len, cnt;
  unsigned long xmax = 4294967295UL, ymax = 4294967295UL, max;
  int plane = -1, use_graycode = 1, multi = 0;

  buflen = 8000;
  buffer = (unsigned char *) malloc(buflen);
  if (!buffer) {
    printf("Sorry, not enough memory available!\n");
    return 1;
  }

fnin = argv[0];
fnout = argv[1];

    fin = fopen(fnin, "rb");
    if (!fin) {
      fprintf(stderr, "Can't open input file '%s", fnin);
      perror("'");
      exit(1);
    }

    fout = fopen(fnout, "wb");
    if (!fout) {
      fprintf(stderr, "Can't open input file '%s", fnout);
      perror("'");
      exit(1);
    }

  /* send input file to decoder */
  jbg_dec_init(&s);
  jbg_dec_maxsize(&s, xmax, ymax);
  /* read BIH first to check VLENGTH */
  len = fread(buffer, 1, 20, fin);

  if(len < 20)
  {
    fclose(fout);
    remove(fnout);
    return 1;
  }

  if (buffer[19] & JBG_VLENGTH) {
    /* VLENGTH = 1 => we might encounter a NEWLEN, therefore read entire
     * input file into memory and run two passes over it */
    if(!read_file(&buffer, &buflen, &len, fin))
        return 1;
    /* scan for NEWLEN marker segments and update BIE header accordingly */
    result = jbg_newlen(buffer, len);
    /* feed data to decoder */
    if (result == JBG_EOK) {
      p = (unsigned char *) buffer;
      result = JBG_EAGAIN;
      while (len > 0 &&
	     (result == JBG_EAGAIN || (result == JBG_EOK && multi))) {
	result = jbg_dec_in(&s, p, len, &cnt);
	p += cnt;
	len -= cnt;
      }
    }
  } else {
    /* VLENGTH = 0 => we can simply pass the input file directly to decoder */
    result = JBG_EAGAIN;
    do {
      cnt = 0;
      p = (unsigned char *) buffer;
      while (len > 0 &&
	     (result == JBG_EAGAIN || (result == JBG_EOK && multi))) {
	result = jbg_dec_in(&s, p, len, &cnt);
	p += cnt;
	len -= cnt;
      }
      if (!(result == JBG_EAGAIN || (result == JBG_EOK && multi)))
	break;
      len = fread(buffer, 1, buflen, fin);
    } while (len > 0);

    if (ferror(fin)) {
      fprintf(stderr, "Problem while reading input file '%s", fnin);
      perror("'");
        fclose(fout);
	remove(fnout);
      return 1;
    }
  }
  if (result != JBG_EOK && result != JBG_EOK_INTR) {
    fprintf(stderr, "Problem with input file '%s': %s\n",
	    fnin, jbg_strerror(result, JBG_EN));

      fclose(fout);
      remove(fnout);
    return 1;
  }
  if (plane >= 0 && jbg_dec_getplanes(&s) <= plane) {
    fprintf(stderr, "Image has only %d planes!\n", jbg_dec_getplanes(&s));
      fclose(fout);
      remove(fnout);
    return 1;
  }

  if (jbg_dec_getplanes(&s) == 1 || plane >= 0)
  {
    int w, h, bpp = 24;

    w = jbg_dec_getwidth(&s);
    h = jbg_dec_getheight(&s);
    
    fwrite(&w,   sizeof(int), 1, fout);
    fwrite(&h,   sizeof(int), 1, fout);
    fwrite(&bpp, sizeof(int), 1, fout);

    unsigned char *d = jbg_dec_getimage(&s, plane < 0 ? 0 : plane), bt;

    int G = 0, index;
    bool brk = false;
    unsigned char S1;

    for(int f = 0;f < h;f++)
    {
            for(int i = 0, g = 0;;g++)
            {
                bt = *(d + G);
                G++;

                for(int F = 256;F >= 2;F /= 2)
                {
                        S1 = (unsigned char)(F / 2);
                        index = (bt&S1) ? 0 : 255;

                        fwrite(&index, 1, 1, fout);
                        fwrite(&index, 1, 1, fout);
                        fwrite(&index, 1, 1, fout);

                        if(++i >= w)
                        {
                            brk = true;
                            break;
                        }
                }

                if(brk)
                {
                    brk = false;
                    break;
                }
            }
    }
  } else {
    /* write PGM output file */
    if ((size_t) jbg_dec_getplanes(&s) > sizeof(unsigned long) * 8)
    {
      fprintf(stderr, "Image has too many planes (%d)!\n",  jbg_dec_getplanes(&s));
	fclose(fout);
      jbg_dec_free(&s);
      return 1;
    }
    max = 0;
    for (i = jbg_dec_getplanes(&s); i > 0; i--)
      max = (max << 1) | 1;

    int w, h, bpp = 24;

    w = jbg_dec_getwidth(&s);
    h = jbg_dec_getheight(&s);
    
    fwrite(&w,   sizeof(int), 1, fout);
    fwrite(&h,   sizeof(int), 1, fout);
    fwrite(&bpp, sizeof(int), 1, fout);
    
    koeff = 255.0 / max;

    jbg_dec_merge_planes(&s, use_graycode, write_it, fout);
  }

  /* check for file errors and close fout */
  if (ferror(fout) || fclose(fout)) {
    fprintf(stderr, "Problem while writing output file '%s", fnout);
    perror("'");
    jbg_dec_free(&s);
    return 1;
  }

  jbg_dec_free(&s);

  return 0;
}

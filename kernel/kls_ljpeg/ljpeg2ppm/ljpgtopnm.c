/*
 * ljpgtopnm.c --
 *
 * This is the main routine for the lossless JPEG decoder.  Large
 * parts are stolen from the IJG code, so:
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * Part of the Independent JPEG Group's software.
 * See the file Copyright for more details.
 *
 * Copyright (c) 1993 Brian C. Smith, The Regents of the University
 * of California
 * All rights reserved.
 * 
 * Copyright (c) 1994 Kongji Huang and Brian C. Smith.
 * Cornell University
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF CORNELL
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * CORNELL UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "jpeg.h"
#include "mcu.h"
#include "proto.h"

/* 
 * input and output file pointers 
 */
FILE *inFile, *outFile;
void FreeArray2D(char **);

void WritePmHeader(DecompressInfo dcInfo);

/*
 *--------------------------------------------------------------
 *
 * ReadJpegData --
 *
 *	This is an interface routine to the JPEG library.  The
 *	JPEG library calls this routine to "get more data"
 *
 * Results:
 *	Number of bytes actually returned.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
int
ReadJpegData (buffer, numBytes)
    char *buffer;		/* Place to put new data */
    int numBytes;		/* Number of bytes to put */
{
    return fread(buffer, 1, numBytes, inFile);
}

/*
 *--------------------------------------------------------------
 *
 * WritePmHeader --
 *
 *	Output Portable Pixmap (PPM) or Portable
 *	Graymap (PGM) image header.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      The PPM or PGM header is written to file
 *	pointed by outFile.
 *
 *--------------------------------------------------------------
 */
void
WritePmHeader(dcInfo)
DecompressInfo dcInfo;
{
    switch(dcInfo.numComponents) {
    case 1: /* pgm */
       if (dcInfo.dataPrecision==8) {
          fprintf(outFile,"P5\n%d %d\n255\n",
                  dcInfo.imageWidth,dcInfo.imageHeight);
       } else {
          fprintf(outFile,"P5\n%d %d\n%d\n",
                  dcInfo.imageWidth,dcInfo.imageHeight,
                  ((1<<dcInfo.dataPrecision)-1));
       }
       break;
    case 3: /* ppm */
       if (dcInfo.dataPrecision==8) {
          fprintf(outFile,"P6\n%d %d\n255\n",
                  dcInfo.imageWidth,dcInfo.imageHeight);
       } else {
          fprintf(outFile,"P6\n%d %d\n%d\n",
                  dcInfo.imageWidth,dcInfo.imageHeight,
                  ((1<<dcInfo.dataPrecision)-1));
       }
       break;
    default:
      fprintf(stderr,"Error: Unsupported image format.\n");
      exit(-1);
    }
}

/*
 *--------------------------------------------------------------
 *
 * ArgParser --
 *
 *      Command line parser.
 *
 * Results:
 *      Command line parameters and options are passed out.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
static void
ArgParser(argc,argv,verbose,linFile,loutFile)
    int argc;
    char **argv;
    int *verbose;
    FILE **linFile, **loutFile;
{
    int argn;
    char *arg;
    const char *usage="ppmtoljpeg [ -v -h ] [ inFile [outFile] ]";
    int NumOfFile=0;

    /*
     * default values
     */
    *linFile=stdin;
    *loutFile=stdout;
    *verbose=0;

    for (argn = 1; argn < argc; argn++) {
        arg=argv[argn];
        if (*arg != '-') { /* process a file name */
           if (NumOfFile==0) {
              if ((*linFile=fopen(arg,"r"))==NULL) {
                 fprintf(stderr,"Can't open %s\n",arg);
                 exit(-1);
              }
           }
           if (NumOfFile==1) {
              if ((*loutFile=fopen(arg,"w"))==NULL) {
                 fprintf(stderr,"Can't open %s\n",arg);
                 exit(-1);
              }
           }
           if (NumOfFile>1) {
              fprintf(stderr,"%s\n",usage);
              exit(-1);
           }
           NumOfFile++;
        }
        else { /* precess a option */
           arg++;
           switch (*arg) {
             case 'h':
                       /* help flag */
                       fprintf(stderr,"Decode a lossless JPEG image into ");
                       fprintf(stderr,"a PPM or PGM image.\n");
                       fprintf(stderr,"Usage:\n");
                       fprintf(stderr,"%s\n",usage);
                       fprintf(stderr,"Default  input: stdin\n");
                       fprintf(stderr,"Default output: stdout\n");
                       fprintf(stderr,"-h  help\n");
                       fprintf(stderr,"-v  verbose\n");
                       exit(1);
                       break;
             case 'v':
                       /* verbose flag */
                       *verbose=1;
                       break;
             default:
                       fprintf(stderr,"%s\n",usage);
                       exit(-1);
           }
        }
    }
}

int
main(argc, argv)
    int argc;
    char **argv;
{
    DecompressInfo dcInfo;
    int verbose;

    /*
     * Process command line parameters.
     */
    MEMSET(&dcInfo, 0, sizeof(dcInfo));
    ArgParser(argc,argv,&verbose,&inFile,&outFile);

    /*
     * Read the JPEG File header, up to scan header, and initialize all
     * the variables in the decompression information structure.
     */
    ReadFileHeader (&dcInfo);

    /*
     * Loop through each scan in image. ReadScanHeader returns
     * 0 once it consumes and EOI marker.
     */
    if (!ReadScanHeader (&dcInfo)) {
	fprintf (stderr, "Empty JPEG file\n");
	exit (1);
    }

    /*
     * Output image parameter if verbose flag is on.
     */
    if (verbose) {
       fprintf(stderr,"sample precision=%d\n",dcInfo.dataPrecision);
       fprintf(stderr,"image height=%d\n",dcInfo.imageHeight);
       fprintf(stderr,"image width=%d\n",dcInfo.imageWidth);
       fprintf(stderr,"component=%d\n",dcInfo.numComponents);
    }

    /* 
     * Write PPM or PGM image header. Decode the image bits
     * stream. Clean up everything when finished decoding.
     */
    WritePmHeader(dcInfo);
    DecoderStructInit(&dcInfo);
    HuffDecoderInit(&dcInfo);
    DecodeImage(&dcInfo);
    FreeArray2D(mcuROW1);
    FreeArray2D(mcuROW2);

    if (ReadScanHeader (&dcInfo)) {
	fprintf (stderr, "Warning: multiple scans detected in JPEG file\n");
	fprintf (stderr, "         not currently supported\n");
	fprintf (stderr, "         ignoring extra scans\n");
    }

    return 0;
}

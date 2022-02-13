/*
 * util.c --
 *
 * Various utility routines used in the jpeg encoder/decoder.  Large parts
 * are stolen from the IJG code, so:
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
#include <string.h>
#include <malloc.h>
#include "jpeg.h"
#include "mcu.h"
#include "proto.h"

void FreeArray2D(char **);
void FixHuffTbl(HuffmanTable *);

unsigned int bitMask[] = {  0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
                            0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
                            0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
                            0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
                            0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
                            0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
                            0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
                            0x0000000f, 0x00000007, 0x00000003, 0x00000001};
/*
 *--------------------------------------------------------------
 *
 * JroundUp --
 *
 *	Compute a rounded up to next multiple of b; a >= 0, b > 0 
 *
 * Results:
 *	Rounded up value.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
int
JroundUp (a, b)
    int a, b;
{
    a += b - 1;
    return a - (a % b);
}

/*
 *--------------------------------------------------------------
 *
 * DecoderStructInit --
 *
 *	Initalize the rest of the fields in the decompression
 *	structure.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
DecoderStructInit (dcPtr)
    DecompressInfo *dcPtr;

{
    short ci,i;
    JpegComponentInfo *compPtr;
    char *buf1,*buf2;
    int mcuSize;

    /*
     * Check sampling factor validity.
     */
    for (ci = 0; ci < dcPtr->numComponents; ci++) {
	compPtr = &dcPtr->compInfo[ci];
	if ((compPtr->hSampFactor != 1) || (compPtr->vSampFactor != 1)) {
	   fprintf (stderr, "Error: Downsampling is not supported.\n");
	   exit(-1);
	}
    }

    /*
     * Prepare array describing MCU composition
     */
    if (dcPtr->compsInScan == 1) {
	dcPtr->MCUmembership[0] = 0;
    } else {
	short lci;

	if (dcPtr->compsInScan > 4) {
	    fprintf (stderr, "Too many components for interleaved scan");
	    exit (1);
	}

	for (lci = 0; lci < dcPtr->compsInScan; lci++) {
            dcPtr->MCUmembership[lci] = lci;
	}
    }

    /*
     * Initialize mucROW1 and mcuROW2 which buffer two rows of
     * pixels for predictor calculation.
     */

    if ((mcuROW1 = (MCU *)malloc(dcPtr->imageWidth*sizeof(MCU)))==NULL) {
       fprintf(stderr,"Not enough memory for mcuROW1\n");
    }
    if ((mcuROW2 = (MCU *)malloc(dcPtr->imageWidth*sizeof(MCU)))==NULL) {
       fprintf(stderr,"Not enough memory for mcuROW2\n");
    }

    mcuSize=dcPtr->compsInScan * sizeof(ComponentType);
    if ((buf1 = (char *)malloc(dcPtr->imageWidth*mcuSize))==NULL) {
       fprintf(stderr,"Not enough memory for buf1\n");
    }
    if ((buf2 = (char *)malloc(dcPtr->imageWidth*mcuSize))==NULL) {
       fprintf(stderr,"Not enough memory for buf2\n");
    }

    for (i=0;i<dcPtr->imageWidth;i++) {
        mcuROW1[i]=(MCU)(buf1+i*mcuSize);
        mcuROW2[i]=(MCU)(buf2+i*mcuSize);
    }
}

/*
 *--------------------------------------------------------------
 *
 * FixHuffTbl --
 *
 *      Compute derived values for a Huffman table one the DHT marker
 *      has been processed.  This generates both the encoding and
 *      decoding tables.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
void
FixHuffTbl (htbl)
    HuffmanTable *htbl;
{
    int p, i, l, lastp, si;
    char huffsize[257];
    Ushort huffcode[257];
    Ushort code;
    int size;
    int value, ll, ul;

    /*
     * Figure C.1: make table of Huffman code length for each symbol
     * Note that this is in code-length order.
     */
    p = 0;
    for (l = 1; l <= 16; l++) {
        for (i = 1; i <= (int)htbl->bits[l]; i++)
            huffsize[p++] = (char)l;
    }
    huffsize[p] = 0;
    lastp = p;


    /*
     * Figure C.2: generate the codes themselves
     * Note that this is in code-length order.
     */
    code = 0;
    si = huffsize[0];
    p = 0;
    while (huffsize[p]) {
        while (((int)huffsize[p]) == si) {
            huffcode[p++] = code;
            code++;
        }
        code <<= 1;
        si++;
    }

    /*
     * Figure C.3: generate encoding tables
     * These are code and size indexed by symbol value
     * Set any codeless symbols to have code length 0; this allows
     * EmitBits to detect any attempt to emit such symbols.
     */
    MEMSET(htbl->ehufsi, 0, sizeof(htbl->ehufsi));

    for (p = 0; p < lastp; p++) {
        htbl->ehufco[htbl->huffval[p]] = huffcode[p];
        htbl->ehufsi[htbl->huffval[p]] = huffsize[p];
    }

    /*
     * Figure F.15: generate decoding tables
     */
    p = 0;
    for (l = 1; l <= 16; l++) {
        if (htbl->bits[l]) {
            htbl->valptr[l] = p;
            htbl->mincode[l] = huffcode[p];
            p += htbl->bits[l];
            htbl->maxcode[l] = huffcode[p - 1];
        } else {
            htbl->maxcode[l] = -1;
        }
    }

    /*
     * We put in this value to ensure HuffDecode terminates.
     */
    htbl->maxcode[17] = 0xFFFFFL;

    /*
     * Build the numbits, value lookup tables.
     * These table allow us to gather 8 bits from the bits stream,
     * and immediately lookup the size and value of the huffman codes.
     * If size is zero, it means that more than 8 bits are in the huffman
     * code (this happens about 3-4% of the time).
     */
    bzero (htbl->numbits, sizeof(htbl->numbits));
    for (p=0; p<lastp; p++) {
        size = huffsize[p];
        if (size <= 8) {
            value = htbl->huffval[p];
            code = huffcode[p];
            ll = code << (8-size);
            if (size < 8) {
                ul = ll | bitMask[24+size];
            } else {
                ul = ll;
            }
            for (i=ll; i<=ul; i++) {
                htbl->numbits[i] = size;
                htbl->value[i] = value;
            }
        }
    }
}

/*
 *--------------------------------------------------------------
 *
 * FreeArray2D --
 *
 *	Free the memory of a 2-D array pointed by arrayPtr.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The memory pointed by arrayPtr is freed.
 *
 *--------------------------------------------------------------
 */
void
FreeArray2D(arrayPtr)
    char **arrayPtr;
{
    free(arrayPtr[0]);
    free(arrayPtr);
}

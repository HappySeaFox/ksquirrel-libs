/*
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

#ifndef _IO
#define _IO

/*
 * Size of the input and output buffer
 */
#define JPEG_BUF_SIZE   4096

/*
 * The following variables keep track of the input and output
 * buffer for the JPEG data.
 */
extern char outputBuffer[JPEG_BUF_SIZE];     /* output buffer              */
extern int numOutputBytes;                   /* bytes in the output buffer */
extern Uchar inputBuffer[JPEG_BUF_SIZE];     /* Input buffer for JPEG data */
extern int numInputBytes;                    /* bytes in inputBuffer       */
extern int maxInputBytes;                    /* Size of inputBuffer        */
extern int inputBufferOffset;                /* Offset of current byte     */

/*
 * the output file pointer. 
 */
extern FILE *outFile;

/*
 *--------------------------------------------------------------
 *
 * EmitByte --
 *
 *	Write a single byte out to the output buffer, and
 *	flush if it's full.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The outp[ut buffer may get flushed.
 *
 *--------------------------------------------------------------
 */
#define EmitByte(val)  {						\
    if (numOutputBytes >= JPEG_BUF_SIZE) {				\
	FlushBytes();							\
    }									\
    outputBuffer[numOutputBytes++] = (char)(val);			\
}

/*
 *--------------------------------------------------------------
 *
 * GetJpegChar, UnGetJpegChar --
 *
 *      Macros to get the next character from the input stream.
 *
 * Results:
 *      GetJpegChar returns the next character in the stream, or EOF
 *      UnGetJpegChar returns nothing.
 *
 * Side effects:
 *      A byte is consumed or put back into the inputBuffer.
 *
 *--------------------------------------------------------------
 */
#define GetJpegChar()                                                   \
    ((inputBufferOffset < numInputBytes)?                               \
     inputBuffer[inputBufferOffset++]:                                  \
     (numInputBytes = 2+ReadJpegData(inputBuffer+2,JPEG_BUF_SIZE-2),    \
      inputBufferOffset = 2,                                            \
      ((inputBufferOffset < numInputBytes)?                             \
       inputBuffer[inputBufferOffset++]:                                \
       EOF)))

#define UnGetJpegChar(ch)       (inputBuffer[--inputBufferOffset]=(ch))

#endif /* _IO */

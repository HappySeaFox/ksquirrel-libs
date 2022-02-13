/*  This file is part of SQuirrel (http://ksquirrel.sf.net) libraries

    Copyright (c) 2004 Dmitry Baryshev <ckult@yandex.ru>

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

#ifndef _SQUIRREL_READ_IMAGE_tga
#define _SQUIRREL_READ_IMAGE_tga

#include "../defs.h"
#include "../err.h"

typedef struct
{
	unsigned char	IDlength;
	unsigned char	ColorMapType; /*  1==we have colormap, 0==no  */
	unsigned char	ImageType;
							/*	0==no image data
								1==uncompressed+color map
								2==uncompressed+true color
								3==uncompressed+B&W
								9==RLE+color map
								10==RLE+true color
								11==RLE+B&W
							*/
	unsigned short 	ColorMapSpecFirstEntryIndex;
	unsigned short 	ColorMapSpecLength;
	unsigned char 	ColorMapSpecEntrySize; /*  15,16,24,32  */
	unsigned short 	ImageSpecX;
	unsigned short 	ImageSpecY;
	unsigned short 	ImageSpecW;
	unsigned short 	ImageSpecH;
	unsigned char 	ImageSpecDepth; /*  8 or 16 or 24 or 32  */
	unsigned char 	ImageSpecDescriptor; /*  7 6 5 4 3 2 1 0   */

}ATTR_ TGA_FILEHEADER;

typedef struct
{
	unsigned char	*ID; /*  length of this == TGA_FILEHEADER::IDlength  */
	unsigned char	*ColorMapData; /*  -//-   */
	unsigned char	*Image; /*  widthxheigth pixels */

}ATTR_ TGA_IMAGEDATA;

typedef struct
{
	unsigned long	ExtensionAreaOffset;
	unsigned long	DeveloperDirectoryOffset;
	unsigned char	Signature[16];	/*  "TRUEVISION-XFILE"  */
	unsigned char	punct;		/*  '.'  */
	unsigned char	TGAnull;		/* 0x0  */

}ATTR_ TGA_FOOTER;

/*  we'll skip "developer area" & "extension area", we don't need to read them.  */

#endif

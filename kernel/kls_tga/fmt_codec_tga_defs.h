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

#ifndef KSQUIRREL_READ_IMAGE_tga
#define KSQUIRREL_READ_IMAGE_tga

struct TGA_FILEHEADER
{
	u8	IDlength;
	u8	ColorMapType; /*  1==we have colormap, 0==no  */
	u8	ImageType;
							/*	0==no image data
								1==uncompressed+color map
								2==uncompressed+true color
								3==uncompressed+B&W
								9==RLE+color map
								10==RLE+true color
								11==RLE+B&W
							*/
	u16 	ColorMapSpecFirstEntryIndex;
	u16 	ColorMapSpecLength;
	u8 	ColorMapSpecEntrySize; /*  15,16,24,32  */
	u16 	ImageSpecX;
	u16 	ImageSpecY;
	u16 	ImageSpecW;
	u16 	ImageSpecH;
	u8 	ImageSpecDepth; /*  8 or 16 or 24 or 32  */
	u8 	ImageSpecDescriptor; /*  7 6 5 4 3 2 1 0   */

}PACKED;

struct TGA_IMAGEDATA
{
	u8	*ID; /*  length of this == TGA_FILEHEADER::IDlength  */
	u8	*ColorMapData; /*  -//-   */
	u8	*Image; /*  widthxheigth pixels */

}PACKED;

struct TGA_FOOTER
{
	u32	ExtensionAreaOffset;
	u32	DeveloperDirectoryOffset;
	u8	Signature[16];	/*  "TRUEVISION-XFILE"  */
	u8	punct;		/*  '.'  */
	u8	TGAnull;		/* 0x0  */

}PACKED;

/*  we'll skip "developer area" & "extension area", we don't need to read them.  */

#endif

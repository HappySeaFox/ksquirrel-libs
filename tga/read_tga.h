#ifndef _READ_WRITE_IMAGE_tga
#define _READ_WRITE_IMAGE_tga

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

/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2005 Dmitry Baryshev <ksquirrel@tut.by>

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

#ifndef KSQUIRREL_CODEC_DEFS_psp
#define KSQUIRREL_CODEC_DEFS_psp

static const u8 PSPSignature[32] =
{
    0x50, 0x61, 0x69, 0x6E, 0x74, 0x20, 0x53, 0x68, 0x6F, 0x70, 0x20, 0x50, 0x72, 0x6F, 0x20, 0x49,
    0x6D, 0x61, 0x67, 0x65, 0x20, 0x46, 0x69, 0x6C, 0x65, 0x0A, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u8 GenAttHead[4] = { 0x7E, 0x42, 0x4B, 0x00 };

typedef u8     ILubyte;
typedef u16    ILushort;
typedef u32    ILuint;
typedef s32    ILint;
typedef double ILdouble;

//-----------------------------------------------------------------------------
//
// ImageLib Sources
// Copyright (C) 2000-2002 by Denton Woods
// Last modified: 05/02/2002 <--Y2K Compliant! =]
//
// Filename: src-IL/include/il_psp.h
//
// Description: Reads a Paint Shop Pro file.
//
//-----------------------------------------------------------------------------


// Block identifiers
enum PSPBlockID {
	PSP_IMAGE_BLOCK = 0,			// (0)  General Image Attributes Block (main)
	PSP_CREATOR_BLOCK,				// (1)  Creator Data Block (main)
	PSP_COLOR_BLOCK,				// (2)  Color Palette Block (main and sub)
	PSP_LAYER_START_BLOCK,			// (3)  Layer Bank Block (main)
	PSP_LAYER_BLOCK,				// (4)  Layer Block (sub)
	PSP_CHANNEL_BLOCK,				// (5)  Channel Block (sub)
	PSP_SELECTION_BLOCK,			// (6)  Selection Block (main)
	PSP_ALPHA_BANK_BLOCK,			// (7)  Alpha Bank Block (main)
	PSP_ALPHA_CHANNEL_BLOCK,		// (8)  Alpha Channel Block (sub)
	PSP_COMPOSITE_IMAGE_BLOCK,		// (9)  Composite Image Block (sub)
	PSP_EXTENDED_DATA_BLOCK,		// (10) Extended Data Block (main)
	PSP_TUBE_BLOCK,					// (11) Picture Tube Data Block (main)
	PSP_ADJUSTMENT_EXTENSION_BLOCK,	// (12) Adjustment Layer Block (sub)
	PSP_VECTOR_EXTENSION_BLOCK,		// (13) Vector Layer Block (sub)
	PSP_SHAPE_BLOCK,				// (14) Vector Shape Block (sub)
	PSP_PAINTSTYLE_BLOCK,			// (15) Paint Style Block (sub)
	PSP_COMPOSITE_IMAGE_BANK_BLOCK, // (16) Composite Image Bank (main)
	PSP_COMPOSITE_ATTRIBUTES_BLOCK, // (17) Composite Image Attr. (sub)
	PSP_JPEG_BLOCK,					// (18) JPEG Image Block (sub)
	PSP_LINESTYLE_BLOCK,			// (19) Line Style Block (sub)
	PSP_TABLE_BANK_BLOCK,			// (20) Table Bank Block (main)
	PSP_TABLE_BLOCK,				// (21) Table Block (sub)
	PSP_PAPER_BLOCK,				// (22) Vector Table Paper Block (sub)
	PSP_PATTERN_BLOCK,				// (23) Vector Table Pattern Block (sub)
};


// Bitmap type
enum PSPDIBType {
	PSP_DIB_IMAGE = 0,	// Layer color bitmap
	PSP_DIB_TRANS_MASK,	// Layer transparency mask bitmap
	PSP_DIB_USER_MASK,	// Layer user mask bitmap
	PSP_DIB_SELECTION,	// Selection mask bitmap
	PSP_DIB_ALPHA_MASK,	// Alpha channel mask bitmap
	PSP_DIB_THUMBNAIL	// Thumbnail bitmap
};

// Channel types
enum PSPChannelType {
	PSP_CHANNEL_COMPOSITE = 0,	// Channel of single channel bitmap
	PSP_CHANNEL_RED,			// Red channel of 24 bit bitmap
	PSP_CHANNEL_GREEN,			// Green channel of 24 bit bitmap
	PSP_CHANNEL_BLUE			// Blue channel of 24 bit bitmap
};

// Possible metrics used to measure resolution
enum PSP_METRIC { 
	PSP_METRIC_UNDEFINED = 0,	// Metric unknown
	PSP_METRIC_INCH,			// Resolution is in inches
	PSP_METRIC_CM				// Resolution is in centimeters
};


// Possible types of compression.
enum PSPCompression {
	PSP_COMP_NONE = 0,	// No compression
	PSP_COMP_RLE,		// RLE compression
	PSP_COMP_LZ77,		// LZ77 compression
	PSP_COMP_JPEG		// JPEG compression (only used by thumbnail and composite image)
};

// Picture tube placement mode.
enum TubePlacementMode {
	tpmRandom,		// Place tube images in random intervals
	tpmConstant		// Place tube images in constant intervals
};

// Picture tube selection mode.
enum TubeSelectionMode {
	tsmRandom,		// Randomly select the next image in tube to display
	tsmIncremental,	// Select each tube image in turn
	tsmAngular,		// Select image based on cursor direction
	tsmPressure,	// Select image based on pressure (from pressure-sensitive pad)
	tsmVelocity		// Select image based on cursor speed
};

// Extended data field types.
enum PSPExtendedDataID {
	PSP_XDATA_TRNS_INDEX = 0	// Transparency index field
};

// Creator field types.
enum PSPCreatorFieldID {
	PSP_CRTR_FLD_TITLE = 0,		// Image document title field
	PSP_CRTR_FLD_CRT_DATE,		// Creation date field
	PSP_CRTR_FLD_MOD_DATE,		// Modification date field
	PSP_CRTR_FLD_ARTIST,		// Artist name field
	PSP_CRTR_FLD_CPYRGHT,		// Copyright holder name field
	PSP_CRTR_FLD_DESC,			// Image document description field
	PSP_CRTR_FLD_APP_ID,		// Creating app id field
	PSP_CRTR_FLD_APP_VER,		// Creating app version field
};

// Creator application identifiers.
enum PSPCreatorAppID {
	PSP_CREATOR_APP_UNKNOWN = 0,	// Creator application unknown
	PSP_CREATOR_APP_PAINT_SHOP_PRO	// Creator is Paint Shop Pro
};

// Layer types.
enum PSPLayerType {
	PSP_LAYER_NORMAL = 0,			// Normal layer
	PSP_LAYER_FLOATING_SELECTION	// Floating selection layer
};

struct PSPRECT
{
	ILuint x1,y1,x2,y2;
} PACKED;

struct PSPHEAD
{
	char		FileSig[32];
	ILushort	MajorVersion;
	ILushort	MinorVersion;
} PACKED;

struct BLOCKHEAD
{
	ILubyte		HeadID[4];
	ILushort	BlockID;
	ILuint		BlockLen;
} PACKED;

struct GENATT_CHUNK
{
	ILint		Width;
	ILint		Height;
	ILdouble	Resolution;
	ILubyte		ResMetric;
	ILushort	Compression;
	ILushort	BitDepth;
	ILushort	PlaneCount;
	ILuint		ColourCount;
	ILubyte		GreyscaleFlag;
	ILuint		SizeOfImage;
	ILint		ActiveLayer;
	ILushort	LayerCount;
	ILuint		GraphicContents;
} PACKED;

struct LAYERINFO_CHUNK
{
	ILubyte		LayerType;
	PSPRECT		ImageRect;
	PSPRECT		SavedImageRect;
	ILubyte		Opacity;
	ILubyte		BlendingMode;
	ILubyte		LayerFlags;
	ILubyte		TransProtFlag;
	ILubyte		LinkID;
	PSPRECT		MaskRect;
	PSPRECT		SavedMaskRect;
	ILubyte		MaskLinked;
	ILubyte		MaskDisabled;
	ILubyte		InvertMaskBlend;
	ILushort	BlendRange;
	ILubyte		SourceBlend1[4];
	ILubyte		DestBlend1[4];
	ILubyte		SourceBlend2[4];
	ILubyte		DestBlend2[4];
	ILubyte		SourceBlend3[4];
	ILubyte		DestBlend3[4];
	ILubyte		SourceBlend4[4];
	ILubyte		DestBlend4[4];
	ILubyte		SourceBlend5[4];
	ILubyte		DestBlend5[4];
} PACKED;

struct LAYERBITMAP_CHUNK
{
	ILushort	NumBitmaps;
	ILushort	NumChannels;
} PACKED;

struct CHANNEL_CHUNK
{
	ILuint		CompLen;
	ILuint		Length;
	ILushort	BitmapType;
	ILushort	ChanType;
} PACKED;

struct ALPHAINFO_CHUNK
{
	PSPRECT		AlphaRect;
	PSPRECT		AlphaSavedRect;
} PACKED;

struct ALPHA_CHUNK
{
	ILushort	BitmapCount;
	ILushort	ChannelCount;
} PACKED;

#endif

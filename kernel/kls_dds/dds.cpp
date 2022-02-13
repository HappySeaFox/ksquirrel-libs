/*
 *  (C) 2007 Dmitry Baryshev, KSquirrel project
 */

/* This file is part of the KDE project
   Copyright (C) 2003 Ignacio Castaño <castano@ludicon.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   Almost all this code is based on nVidia's DDS-loading example
   and the DevIl's source code by Denton Woods.
*/

/* this code supports:
 * reading:
 *     rgb and dxt dds files
 *     cubemap dds files
 *     volume dds files -- TODO
 * writing:
 *     rgb dds files only -- TODO
 */

#include <cmath> // sqrtf
#include <fstream>

#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fmt_defs.h"

#include "dds.h"

#ifndef __USE_ISOC99
#define sqrtf(x) ((float)sqrt(x))
#endif

typedef u32 uint;
typedef u16 ushort;
typedef u8 uchar;

typedef RGBA* RGBAP;

namespace {	// Private.

static bool MALLOC_ROWS(RGBAP **A, const int RB, const int H)
{
    *A = (RGBAP *)malloc(H * sizeof(RGBA*));

    if(!*A)
        return false;

    for(s32 row = 0; row < H; row++)
        (*A)[row] = 0;

    for(s32 row = 0; row < (s32)H; row++)
    {
        (*A)[row] = (RGBA *)malloc(RB);

        if(!(*A)[row])
            return false;

        memset((*A)[row], 0, RB);
    }

    return true;
}
/*
static void FREE_ROWS(RGBAP **A, const int H)
{
    if(*A)
    {
        for(s32 i = 0;i < H;i++)
        {
            if((*A)[i])
                free((*A)[i]);
        }

        free(*A);
        *A = 0;
    }
}
*/
#if !defined(MAKEFOURCC)
#	define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		(uint(uchar(ch0)) | (uint(uchar(ch1)) << 8) | \
		(uint(uchar(ch2)) << 16) | (uint(uchar(ch3)) << 24 ))
#endif

//#ifdef max
#undef max
#define max(a,b)	((a)>(b)?(a):(b))
//#endif

#define HORIZONTAL 1
#define VERTICAL 2
#define CUBE_LAYOUT	HORIZONTAL

	struct Color8888
	{
		uchar r, g, b, a;
	};

	union Color565
	{
		struct {
			ushort b : 5;
			ushort g : 6;
			ushort r : 5;
		} c;
		ushort u;
	};

	union Color1555 {
		struct {
			ushort b : 5;
			ushort g : 5;
			ushort r : 5;
			ushort a : 1;
		} c;
		ushort u;
	};

	union Color4444 {
		struct {
			ushort b : 4;
			ushort g : 4;
			ushort r : 4;
			ushort a : 4;
		} c;
		ushort u;
	};


	static const uint FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' ');
	static const uint FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
	static const uint FOURCC_DXT2 = MAKEFOURCC('D', 'X', 'T', '2');
	static const uint FOURCC_DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
	static const uint FOURCC_DXT4 = MAKEFOURCC('D', 'X', 'T', '4');
	static const uint FOURCC_DXT5 = MAKEFOURCC('D', 'X', 'T', '5');
	static const uint FOURCC_RXGB = MAKEFOURCC('R', 'X', 'G', 'B');
	static const uint FOURCC_ATI2 = MAKEFOURCC('A', 'T', 'I', '2');

	static const uint DDSD_CAPS = 0x00000001l;
	static const uint DDSD_PIXELFORMAT = 0x00001000l;
	static const uint DDSD_WIDTH = 0x00000004l;
	static const uint DDSD_HEIGHT = 0x00000002l;
	static const uint DDSD_PITCH = 0x00000008l;

	static const uint DDSCAPS_TEXTURE = 0x00001000l;
	static const uint DDSCAPS2_VOLUME = 0x00200000l;
	static const uint DDSCAPS2_CUBEMAP = 0x00000200l;

	static const uint DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400l;
	static const uint DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800l;
	static const uint DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000l;
	static const uint DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000l;
	static const uint DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000l;
	static const uint DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000l;

	static const uint DDPF_RGB = 0x00000040l;
 	static const uint DDPF_FOURCC = 0x00000004l;
 	static const uint DDPF_ALPHAPIXELS = 0x00000001l;

	enum DDSType {
		DDS_A8R8G8B8 = 0,
		DDS_A1R5G5B5 = 1,
		DDS_A4R4G4B4 = 2,
		DDS_R8G8B8 = 3,
		DDS_R5G6B5 = 4,
		DDS_DXT1 = 5,
		DDS_DXT2 = 6,
		DDS_DXT3 = 7,
		DDS_DXT4 = 8,
		DDS_DXT5 = 9,
		DDS_RXGB = 10,
		DDS_ATI2 = 11,
		DDS_UNKNOWN
	};


	struct DDSPixelFormat {
		uint size;
		uint flags;
		uint fourcc;
		uint bitcount;
		uint rmask;
		uint gmask;
		uint bmask;
		uint amask;
	};

	static ifstreamK & operator>> ( ifstreamK & s, DDSPixelFormat & pf )
	{
		s.readK(&pf.size, sizeof(pf.size));
		s.readK(&pf.flags, sizeof(pf.flags));
		s.readK(&pf.fourcc, sizeof(pf.fourcc));
		s.readK(&pf.bitcount, sizeof(pf.bitcount));
		s.readK(&pf.rmask, sizeof(pf.rmask));
		s.readK(&pf.gmask, sizeof(pf.gmask));
		s.readK(&pf.bmask, sizeof(pf.bmask));
		s.readK(&pf.amask, sizeof(pf.amask));

		return s;
	}

	struct DDSCaps {
		uint caps1;
		uint caps2;
		uint caps3;
		uint caps4;
	};

	static ifstreamK & operator>> ( ifstreamK & s, DDSCaps & caps )
	{
		s.readK(&caps.caps1, sizeof(caps.caps1));
		s.readK(&caps.caps2, sizeof(caps.caps2));
		s.readK(&caps.caps3, sizeof(caps.caps3));
		s.readK(&caps.caps4, sizeof(caps.caps4));

		return s;
	}

	struct DDSHeader {
		uint size;
		uint flags;
		uint height;
		uint width;
		uint pitch;
		uint depth;
		uint mipmapcount;
		uint reserved[11];
		DDSPixelFormat pf;
		DDSCaps caps;
		uint notused;
	};

	static ifstreamK & operator>> ( ifstreamK & s, DDSHeader & header )
	{
		s.readK(&header.size, sizeof(header.size));
		s.readK(&header.flags, sizeof(header.flags));
		s.readK(&header.height, sizeof(header.height));
		s.readK(&header.width, sizeof(header.width));
		s.readK(&header.pitch, sizeof(header.pitch));
		s.readK(&header.depth, sizeof(header.depth));
		s.readK(&header.mipmapcount, sizeof(header.mipmapcount));

		for( int i = 0; i < 11; i++ )
                {
			s.readK(&header.reserved[i], sizeof(header.reserved[i]));
		}

		s >> header.pf;
		s >> header.caps;

		s.readK(&header.notused, sizeof(header.notused));

		return s;
	}

	static bool IsValid( const DDSHeader & header )
	{
		if( header.size != 124 || !header.width || !header.height) {
			return false;
		}

		const uint required = (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT);

		if( (header.flags & required) != required ) {
			return false;
		}
		if( header.pf.size != 32 ) {
			return false;
		}
		if( !(header.caps.caps1 & DDSCAPS_TEXTURE) ) {
			return false;
		}
		return true;
	}

	// Get supported type. We currently support 10 different types.
	static DDSType GetType( const DDSHeader & header )
	{
		if( header.pf.flags & DDPF_RGB ) {
			if( header.pf.flags & DDPF_ALPHAPIXELS ) {
				switch( header.pf.bitcount ) {
					case 16:
						return (header.pf.amask == 0x7000) ? DDS_A1R5G5B5 : DDS_A4R4G4B4;
					case 32:
						return DDS_A8R8G8B8;
				}
			}
			else {
				switch( header.pf.bitcount ) {
					case 16:
						return DDS_R5G6B5;
					case 24:
						return DDS_R8G8B8;
				}
			}
		}
		else if( header.pf.flags & DDPF_FOURCC ) {
			switch( header.pf.fourcc ) {
				case FOURCC_DXT1:
					return DDS_DXT1;
				case FOURCC_DXT2:
					return DDS_DXT2;
				case FOURCC_DXT3:
					return DDS_DXT3;
				case FOURCC_DXT4:
					return DDS_DXT4;
				case FOURCC_DXT5:
					return DDS_DXT5;
				case FOURCC_RXGB:
					return DDS_RXGB;
				case FOURCC_ATI2:
					return DDS_ATI2;
			}
		}
		return DDS_UNKNOWN;
	}

	static bool IsCubeMap( const DDSHeader & header )
	{
		return header.caps.caps2 & DDSCAPS2_CUBEMAP;
	}

	static bool IsSupported( const DDSHeader & header )
	{
		if( header.caps.caps2 & DDSCAPS2_VOLUME ) {
			return false;
		}
		if( GetType(header) == DDS_UNKNOWN ) {
			return false;
		}
		return true;
	}

	static bool LoadA8R8G8B8( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		for( uint y = 0; y < h; y++ ) {
			RGBA *scanline = img[y];
			for( uint x = 0; x < w; x++ ) {
				uchar r, g, b, a;

				s.readK(&b, sizeof(b));
				s.readK(&g, sizeof(g));
				s.readK(&r, sizeof(r));
				s.readK(&a, sizeof(a));

				scanline[x] = RGBA(r, g, b, a);
			}
		}

		return true;
	}

	static bool LoadR8G8B8( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		for( uint y = 0; y < h; y++ ) {
			RGBA * scanline = img[y];
			for( uint x = 0; x < w; x++ )
                        {
				uchar r, g, b;

				s.readK(&b, sizeof(b));
				s.readK(&g, sizeof(g));
				s.readK(&r, sizeof(r));

				scanline[x] = RGBA(r, g, b, 255);
			}
		}

		return true;
	}

	static bool LoadA1R5G5B5( ifstreamK & s, const DDSHeader & header, RGBA **img )
	{
		const uint w = header.width;
		const uint h = header.height;

		for( uint y = 0; y < h; y++ ) {
			RGBA * scanline = img[y];
			for( uint x = 0; x < w; x++ ) {
				Color1555 color;
				s.readK(&color.u, sizeof(color.u));
				uchar a = (color.c.a != 0) ? 0xFF : 0;
				uchar r = (color.c.r << 3) | (color.c.r >> 2);
				uchar g = (color.c.g << 3) | (color.c.g >> 2);
				uchar b = (color.c.b << 3) | (color.c.b >> 2);
				scanline[x] = RGBA(r, g, b, a);
			}
		}

		return true;
	}

	static bool LoadA4R4G4B4( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		for( uint y = 0; y < h; y++ ) {
			RGBA * scanline = img[y];
			for( uint x = 0; x < w; x++ ) {
				Color4444 color;
				s.readK(&color.u, sizeof(color.u));
				uchar a = (color.c.a << 4) | color.c.a;
				uchar r = (color.c.r << 4) | color.c.r;
				uchar g = (color.c.g << 4) | color.c.g;
				uchar b = (color.c.b << 4) | color.c.b;
				scanline[x] = RGBA(r, g, b, a);
			}
		}

		return true;
	}

	static bool LoadR5G6B5( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		for( uint y = 0; y < h; y++ ) {
			RGBA * scanline = img[y];
			for( uint x = 0; x < w; x++ ) {
				Color565 color;
				s.readK(&color.u, sizeof(color.u));
				uchar r = (color.c.r << 3) | (color.c.r >> 2);
				uchar g = (color.c.g << 2) | (color.c.g >> 4);
				uchar b = (color.c.b << 3) | (color.c.b >> 2);
				scanline[x] = RGBA(r, g, b, 255);
			}
		}

		return true;
	}

	static ifstreamK & operator>> ( ifstreamK & s, Color565 & c )
	{
		s.readK(&c.u, sizeof(c.u));

                return s;
	}

	struct BlockDXT
	{
		Color565 col0;
		Color565 col1;
		uchar row[4];

		void GetColors( Color8888 color_array[4] )
		{
			color_array[0].r = (col0.c.r << 3) | (col0.c.r >> 2);
			color_array[0].g = (col0.c.g << 2) | (col0.c.g >> 4);
			color_array[0].b = (col0.c.b << 3) | (col0.c.b >> 2);
			color_array[0].a = 0xFF;

			color_array[1].r = (col1.c.r << 3) | (col1.c.r >> 2);
			color_array[1].g = (col1.c.g << 2) | (col1.c.g >> 4);
			color_array[1].b = (col1.c.b << 3) | (col1.c.b >> 2);
			color_array[1].a = 0xFF;

			if( col0.u > col1.u ) {
				// Four-color block: derive the other two colors.
				color_array[2].r = (2 * color_array[0].r + color_array[1].r) / 3;
				color_array[2].g = (2 * color_array[0].g + color_array[1].g) / 3;
				color_array[2].b = (2 * color_array[0].b + color_array[1].b) / 3;
				color_array[2].a = 0xFF;

				color_array[3].r = (2 * color_array[1].r + color_array[0].r) / 3;
				color_array[3].g = (2 * color_array[1].g + color_array[0].g) / 3;
				color_array[3].b = (2 * color_array[1].b + color_array[0].b) / 3;
				color_array[3].a = 0xFF;
			}
			else {
				// Three-color block: derive the other color.
				color_array[2].r = (color_array[0].r + color_array[1].r) / 2;
				color_array[2].g = (color_array[0].g + color_array[1].g) / 2;
				color_array[2].b = (color_array[0].b + color_array[1].b) / 2;
				color_array[2].a = 0xFF;

				// Set all components to 0 to match DXT specs.
				color_array[3].r = 0x00; // color_array[2].r;
				color_array[3].g = 0x00; // color_array[2].g;
				color_array[3].b = 0x00; // color_array[2].b;
				color_array[3].a = 0x00;
			}
		}
	};

	static ifstreamK & operator>> ( ifstreamK & s, BlockDXT & c )
	{
    	    s >> c.col0 >> c.col1;

            s.readK(&c.row[0], sizeof(uchar));
            s.readK(&c.row[1], sizeof(uchar));
            s.readK(&c.row[2], sizeof(uchar));
            s.readK(&c.row[3], sizeof(uchar));

            return s;
	}

	struct BlockDXTAlphaExplicit {
		ushort row[4];
	};

	static ifstreamK & operator>> ( ifstreamK & s, BlockDXTAlphaExplicit & c )
	{
                s.readK(&c.row[0], sizeof(ushort));
                s.readK(&c.row[1], sizeof(ushort));
                s.readK(&c.row[2], sizeof(ushort));
                s.readK(&c.row[3], sizeof(ushort));

                return s;
	}

	struct BlockDXTAlphaLinear {
		uchar alpha0;
		uchar alpha1;
		uchar bits[6];

		void GetAlphas( uchar alpha_array[8] )
		{
			alpha_array[0] = alpha0;
			alpha_array[1] = alpha1;

			// 8-alpha or 6-alpha block?
			if( alpha_array[0] > alpha_array[1] )
			{
				// 8-alpha block:  derive the other 6 alphas.
				// 000 = alpha_0, 001 = alpha_1, others are interpolated

				alpha_array[2] = ( 6 * alpha0 +     alpha1) / 7;	// bit code 010
				alpha_array[3] = ( 5 * alpha0 + 2 * alpha1) / 7;	// Bit code 011
				alpha_array[4] = ( 4 * alpha0 + 3 * alpha1) / 7;	// Bit code 100
				alpha_array[5] = ( 3 * alpha0 + 4 * alpha1) / 7;	// Bit code 101
				alpha_array[6] = ( 2 * alpha0 + 5 * alpha1) / 7;	// Bit code 110
				alpha_array[7] = (     alpha0 + 6 * alpha1) / 7;	// Bit code 111
			}
			else
			{
				// 6-alpha block:  derive the other alphas.
				// 000 = alpha_0, 001 = alpha_1, others are interpolated

				alpha_array[2] = (4 * alpha0 +     alpha1) / 5;		// Bit code 010
				alpha_array[3] = (3 * alpha0 + 2 * alpha1) / 5;		// Bit code 011
				alpha_array[4] = (2 * alpha0 + 3 * alpha1) / 5;		// Bit code 100
				alpha_array[5] = (    alpha0 + 4 * alpha1) / 5;		// Bit code 101
				alpha_array[6] = 0x00;								// Bit code 110
				alpha_array[7] = 0xFF;								// Bit code 111
			}
		}

		void GetBits( uchar bit_array[16] )
		{
			uint b = (uint &) bits[0];
			bit_array[0] = uchar(b & 0x07); b >>= 3;
			bit_array[1] = uchar(b & 0x07); b >>= 3;
			bit_array[2] = uchar(b & 0x07); b >>= 3;
			bit_array[3] = uchar(b & 0x07); b >>= 3;
			bit_array[4] = uchar(b & 0x07); b >>= 3;
			bit_array[5] = uchar(b & 0x07); b >>= 3;
			bit_array[6] = uchar(b & 0x07); b >>= 3;
			bit_array[7] = uchar(b & 0x07); b >>= 3;

			b = (uint &) bits[3];
			bit_array[8] = uchar(b & 0x07); b >>= 3;
			bit_array[9] = uchar(b & 0x07); b >>= 3;
			bit_array[10] = uchar(b & 0x07); b >>= 3;
			bit_array[11] = uchar(b & 0x07); b >>= 3;
			bit_array[12] = uchar(b & 0x07); b >>= 3;
			bit_array[13] = uchar(b & 0x07); b >>= 3;
			bit_array[14] = uchar(b & 0x07); b >>= 3;
			bit_array[15] = uchar(b & 0x07); b >>= 3;
		}
	};

	static ifstreamK & operator>> ( ifstreamK & s, BlockDXTAlphaLinear & c )
	{
		s.readK(&c.alpha0, sizeof(c.alpha0));
		s.readK(&c.alpha1, sizeof(c.alpha1));

                s.readK(&c.bits[0], sizeof(uchar));
                s.readK(&c.bits[1], sizeof(uchar));
                s.readK(&c.bits[2], sizeof(uchar));
                s.readK(&c.bits[3], sizeof(uchar));
                s.readK(&c.bits[4], sizeof(uchar));
                s.readK(&c.bits[5], sizeof(uchar));

                return s;
	}

	static bool LoadDXT1( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		BlockDXT block;
		RGBA * scanline[4];

		for( uint y = 0; y < h; y += 4 ) {
			for( uint j = 0; j < 4; j++ ) {
				scanline[j] = img[y + j];
			}

			for( uint x = 0; x < w; x += 4 ) {

				// Read 64bit color block.
				s >> block;

				// Decode color block.
				Color8888 color_array[4];
				block.GetColors(color_array);

				// bit masks = 00000011, 00001100, 00110000, 11000000
				const uint masks[4] = { 3, 3<<2, 3<<4, 3<<6 };
				const int shift[4] = { 0, 2, 4, 6 };

				// Write color block.
				for( uint j = 0; j < 4; j++ ) {
					for( uint i = 0; i < 4; i++ ) {
						/*if( img.valid( x+i, y+j ) ) */{

							uint idx = (block.row[j] & masks[i]) >> shift[i];
							scanline[j][x+i] = RGBA(color_array[idx].r, color_array[idx].g, color_array[idx].b, color_array[idx].a);
						}
					}
				}
			}
		}

		return true;
	}

	static bool LoadDXT3( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		BlockDXT block;
		BlockDXTAlphaExplicit alpha;
		RGBA * scanline[4];

		for( uint y = 0; y < h; y += 4 ) {
			for( uint j = 0; j < 4; j++ ) {
				scanline[j] = img[y + j];
			}
			for( uint x = 0; x < w; x += 4 ) {

				// Read 128bit color block.
				s >> alpha;
				s >> block;

				// Decode color block.
				Color8888 color_array[4];
				block.GetColors(color_array);

				// bit masks = 00000011, 00001100, 00110000, 11000000
				const uint masks[4] = { 3, 3<<2, 3<<4, 3<<6 };
				const int shift[4] = { 0, 2, 4, 6 };

				// Write color block.
				for( uint j = 0; j < 4; j++ ) {
					ushort a = alpha.row[j];
					for( uint i = 0; i < 4; i++ ) {
						/*if( img.valid( x+i, y+j ) ) */{
							uint idx = (block.row[j] & masks[i]) >> shift[i];
							color_array[idx].a = a & 0x0f;
							color_array[idx].a = color_array[idx].a | (color_array[idx].a << 4);
							scanline[j][x+i] = RGBA(color_array[idx].r, color_array[idx].g, color_array[idx].b, color_array[idx].a);
						}
						a >>= 4;
					}
				}
			}
		}
		return true;
	}

	static bool LoadDXT2( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		if( !LoadDXT3(s, header, img) ) return false;
		//UndoPremultiplyAlpha(img);
		return true;
	}

	static bool LoadDXT5( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		BlockDXT block;
		BlockDXTAlphaLinear alpha;
		RGBA * scanline[4];

		for( uint y = 0; y < h; y += 4 ) {
			for( uint j = 0; j < 4; j++ ) {
				scanline[j] = img[y + j];
			}
			for( uint x = 0; x < w; x += 4 ) {

				// Read 128bit color block.
				s >> alpha;
				s >> block;

				// Decode color block.
				Color8888 color_array[4];
				block.GetColors(color_array);

				uchar alpha_array[8];
				alpha.GetAlphas(alpha_array);

				uchar bit_array[16];
				alpha.GetBits(bit_array);

				// bit masks = 00000011, 00001100, 00110000, 11000000
				const uint masks[4] = { 3, 3<<2, 3<<4, 3<<6 };
				const int shift[4] = { 0, 2, 4, 6 };

				// Write color block.
				for( uint j = 0; j < 4; j++ ) {
					for( uint i = 0; i < 4; i++ ) {
						/*if( img.valid( x+i, y+j ) ) */{
							uint idx = (block.row[j] & masks[i]) >> shift[i];
							color_array[idx].a = alpha_array[bit_array[j*4+i]];
							scanline[j][x+i] = RGBA(color_array[idx].r, color_array[idx].g, color_array[idx].b, color_array[idx].a);
						}
					}
				}
			}
		}

		return true;
	}
	static bool LoadDXT4( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		if( !LoadDXT5(s, header, img) ) return false;
		//UndoPremultiplyAlpha(img);
		return true;
	}

	static bool LoadRXGB( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		BlockDXT block;
		BlockDXTAlphaLinear alpha;
		RGBA * scanline[4];

		for( uint y = 0; y < h; y += 4 ) {
			for( uint j = 0; j < 4; j++ ) {
				scanline[j] = img[y + j];
			}
			for( uint x = 0; x < w; x += 4 ) {

				// Read 128bit color block.
				s >> alpha;
				s >> block;

				// Decode color block.
				Color8888 color_array[4];
				block.GetColors(color_array);

				uchar alpha_array[8];
				alpha.GetAlphas(alpha_array);

				uchar bit_array[16];
				alpha.GetBits(bit_array);

				// bit masks = 00000011, 00001100, 00110000, 11000000
				const uint masks[4] = { 3, 3<<2, 3<<4, 3<<6 };
				const int shift[4] = { 0, 2, 4, 6 };

				// Write color block.
				for( uint j = 0; j < 4; j++ ) {
					for( uint i = 0; i < 4; i++ ) {
						/*if( img.valid( x+i, y+j ) ) */{
							uint idx = (block.row[j] & masks[i]) >> shift[i];
							color_array[idx].a = alpha_array[bit_array[j*4+i]];
							scanline[j][x+i] = RGBA(color_array[idx].a, color_array[idx].g, color_array[idx].b, 255);
						}
					}
				}
			}
		}

		return true;
	}

	static bool LoadATI2( ifstreamK & s, const DDSHeader & header, RGBA **img)
	{
		const uint w = header.width;
		const uint h = header.height;

		BlockDXTAlphaLinear xblock;
		BlockDXTAlphaLinear yblock;
		RGBA * scanline[4];

		for( uint y = 0; y < h; y += 4 ) {
			for( uint j = 0; j < 4; j++ ) {
				scanline[j] = img[y + j];
			}
			for( uint x = 0; x < w; x += 4 ) {

				// Read 128bit color block.
				s >> xblock;
				s >> yblock;

				// Decode color block.
				uchar xblock_array[8];
				xblock.GetAlphas(xblock_array);

				uchar xbit_array[16];
				xblock.GetBits(xbit_array);

				uchar yblock_array[8];
				yblock.GetAlphas(yblock_array);

				uchar ybit_array[16];
				yblock.GetBits(ybit_array);

				// Write color block.
				for( uint j = 0; j < 4; j++ ) {
					for( uint i = 0; i < 4; i++ ) {
						/*if( img.valid( x+i, y+j ) ) */{
							const uchar nx = xblock_array[xbit_array[j*4+i]];
							const uchar ny = yblock_array[ybit_array[j*4+i]];
							
							const float fx = float(nx) / 127.5f - 1.0f;
							const float fy = float(ny) / 127.5f - 1.0f;
							const float fz = sqrtf(1.0f - fx*fx - fy*fy);
							const uchar nz = uchar((fz + 1.0f) * 127.5f);
							
							scanline[j][x+i] = RGBA(nx, ny, nz, 255);
						}
					}
				}
			}
		}

		return true;
	}

	typedef bool (* TextureLoader)( ifstreamK & s, const DDSHeader & header, RGBA **);

	// Get an appropiate texture loader for the given type.
	static TextureLoader GetTextureLoader( DDSType type ) {
		switch( type ) {
			case DDS_A8R8G8B8:
				return LoadA8R8G8B8;
			case DDS_A1R5G5B5:
				return LoadA1R5G5B5;
			case DDS_A4R4G4B4:
				return LoadA4R4G4B4;
			case DDS_R8G8B8:
				return LoadR8G8B8;
			case DDS_R5G6B5:
				return LoadR5G6B5;
			case DDS_DXT1:
				return LoadDXT1;
			case DDS_DXT2:
				return LoadDXT2;
			case DDS_DXT3:
				return LoadDXT3;
			case DDS_DXT4:
				return LoadDXT4;
			case DDS_DXT5:
				return LoadDXT5;
			case DDS_RXGB:
				return LoadRXGB;
			case DDS_ATI2:
				return LoadATI2;
			default:
				return NULL;
		};
	}


	// Load a 2d texture.
	static bool LoadTexture( ifstreamK & s, const DDSHeader & header, RGBA ***img)
	{
                if(!MALLOC_ROWS(img, header.width * sizeof(RGBA), header.height))
                    return false;
        
		// Read image.
		DDSType type = GetType( header );

		TextureLoader loader = GetTextureLoader( type );
		if( loader == NULL ) {
			return false;
		}

		return loader( s, header, *img );
	}


	static int FaceOffset( const DDSHeader & header ) {

		DDSType type = GetType( header );

		int mipmap = max(header.mipmapcount, 1);
		int size = 0;
		int w = header.width;
		int h = header.height;
		
		if( type >= DDS_DXT1 ) {
			int multiplier = (type == DDS_DXT1) ? 8 : 16;
			do {
				int face_size = max(w/4,1) * max(h/4,1) * multiplier;
				size += face_size;
				w >>= 1;
				h >>= 1;
			} while( --mipmap );
		}
		else {
			int multiplier = header.pf.bitcount / 8;
			do {
				int face_size = w * h * multiplier;
				size += face_size;
				w = max( w>>1, 1 );
				h = max( h>>1, 1 );
			} while( --mipmap );
		}

		return size;
	}

#if CUBE_LAYOUT == HORIZONTAL
	static int face_offset[6][2] = { {2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1} };
#elif CUBE_LAYOUT == VERTICAL
	static int face_offset[6][2] = { {2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {1, 3} };
#endif
	static int face_flags[6] = {
		DDSCAPS2_CUBEMAP_POSITIVEX,
		DDSCAPS2_CUBEMAP_NEGATIVEX,
		DDSCAPS2_CUBEMAP_POSITIVEY,
		DDSCAPS2_CUBEMAP_NEGATIVEY,
		DDSCAPS2_CUBEMAP_POSITIVEZ,
		DDSCAPS2_CUBEMAP_NEGATIVEZ
	};

	// Load unwrapped cube map.
	static bool LoadCubeMap( ifstreamK & s, const DDSHeader & header, RGBA ***img)
	{
            int dimw = 0, dimh = 0;
#if CUBE_LAYOUT == HORIZONTAL
		dimw = 4 * header.width;
                dimh = 3 * header.height;
#elif CUBE_LAYOUT == VERTICAL
		dimw = 3 * header.width;
                dimh = 4 * header.height;
#endif

                if(!MALLOC_ROWS(img, dimw * sizeof(RGBA), dimh))
                    return false;

		// Create dst image.

		DDSType type = GetType( header );
		
		// Select texture loader.
		TextureLoader loader = GetTextureLoader( type );
		if( loader == NULL ) {
			return false;
		}

                RGBA **face;
                if(!MALLOC_ROWS(&face, header.width * sizeof(RGBA), header.height))
                    return false;

		// Create face image.
		int offset = s.tellg();
		int size = FaceOffset( header );

		for( int i = 0; i < 6; i++ ) {

			if( !(header.caps.caps2 & face_flags[i]) ) {
				// Skip face.
				continue;
			}

			// Seek device.
			s.seekg(offset);
			offset += size;

			// Load face from stream.
			if( !loader( s, header, face ) ) {
				return false;
			}

#if CUBE_LAYOUT == VERTICAL
			if( i == 5 ) {
//				face = face.mirror(true, true);
			}
#endif

			// Compute face offsets.
			int offset_x = face_offset[i][0] * header.width;
			int offset_y = face_offset[i][1] * header.height;

			// Copy face on the image.
			for( uint y = 0; y < header.height; y++ )
                        {
			    RGBA * src = face[y];
    			    RGBA * dst = (*img)[y + offset_y] + offset_x;
			    memcpy( dst, src, sizeof(RGBA) * header.width );
			}
		}

		return true;
	}

}

bool dds_read(const std::string &file, DDSINFO &dds)
{
	ifstreamK s;
        s.open(file.c_str(), std::ios::binary | std::ios::in);

        if(!s.good())
            return false;

	// Validate header.
	uint fourcc;
        s.readK(&fourcc, sizeof(uint));

	if(fourcc != FOURCC_DDS)
	    return false;

	// Read image header.
	DDSHeader header;
	s >> header;

	// Check image file format.
	if(!s.good() || !IsValid(header) || !IsSupported(header))
		return false;

	RGBA **img = 0;
	bool result;

	if(IsCubeMap(header))
		result = LoadCubeMap(s, header, &img);
	else
		result = LoadTexture(s, header, &img);

    if(result)
    {
        dds.w = header.width;
        dds.h = header.height;
        dds.img = img;
    }

    return result;
}

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

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_utils.h"

#include "fmt_codec_psp_defs.h"
#include "fmt_codec_psp.h"

#include "ksquirrel-libs/error.h"

#include "../xpm/codec_psp.xpm"

fmt_codec::fmt_codec() : fmt_codec_base()
{}

fmt_codec::~fmt_codec()
{}

void fmt_codec::options(codec_options *o)
{
    o->version = "0.1.0";
    o->name = "PaintShop Pro";
    o->filter = "*.psp ";
    o->config = "";
    o->mime = "";
    o->mimetype = "image/x-psp";
    o->pixmap = codec_psp;
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = false;
    o->writeanimated = false;
    o->needtempfile = false;
}

/************** Utility functions from DevIL *********************/

//-----------------------------------------------------------------------------
//
// ImageLib Sources
// Copyright (C) 2000-2002 by Denton Woods
// Last modified: 05/04/2002 <--Y2K Compliant! =]
//
// Filename: src-IL/src/il_psp.c
//
// Description: Reads a Paint Shop Pro file.
//
//-----------------------------------------------------------------------------


bool fmt_codec::iGetPspHead()
{
    if(!frs.readK(Header.FileSig, 32))
        return false;

    if(!frs.readK(&Header.MajorVersion, sizeof(Header.MajorVersion)))
        return false;

    if(!frs.readK(&Header.MinorVersion, sizeof(Header.MinorVersion)))
        return false;

        return true;
}

bool fmt_codec::iCheckPsp()
{
    if (strcmp(Header.FileSig, "Paint Shop Pro Image File\n\x1a")) // "i"
            return false;
    if (Header.MajorVersion < 3 || Header.MajorVersion > 5)
            return false;
    if (Header.MinorVersion != 0)
            return false;

    return true;
}

bool fmt_codec::ReadGenAttributes()
{
        BLOCKHEAD               AttHead;
        ILint                   Padding;
        ILuint                  ChunkLen;

        if(!frs.readK(&AttHead, sizeof(AttHead)))
            return false;

        if (AttHead.HeadID[0] != 0x7E || AttHead.HeadID[1] != 0x42 || AttHead.HeadID[2] != 0x4B || AttHead.HeadID[3] != 0x00)
                return false;

        if (AttHead.BlockID != PSP_IMAGE_BLOCK)
                return false;

        if(!frs.readK(&ChunkLen, sizeof(ChunkLen)))
            return false;

        if (Header.MajorVersion != 3)
                ChunkLen -= 4;

        if (!frs.readK(&AttChunk, std::min(sizeof(AttChunk), size_t(ChunkLen))))
                return false;

        // Can have new entries in newer versions of the spec (4.0).
        Padding = (ChunkLen) - sizeof(AttChunk);

        if (Padding > 0)
                frs.seekg(Padding, ios::cur);

        // @TODO:  Anything but 24 not supported yet...
        if (AttChunk.BitDepth != 24 && AttChunk.BitDepth != 8)
                return false;

        // @TODO;  Add support for compression...
        if (AttChunk.Compression != PSP_COMP_NONE && AttChunk.Compression != PSP_COMP_RLE)
                return false;

        // @TODO: Check more things in the general attributes chunk here.
        return true;
}

bool fmt_codec::ParseChunks()
{
        BLOCKHEAD       Block;
        size_t          Pos;

        do {
                if (!frs.readK(&Block, sizeof(Block)))
                    return true;

                if (Header.MajorVersion == 3)
                    frs.readK(&Block.BlockLen, sizeof(Block.BlockLen));

                if (Block.HeadID[0] != 0x7E || Block.HeadID[1] != 0x42 || Block.HeadID[2] != 0x4B || Block.HeadID[3] != 0x00)
                    return true;

                Pos = frs.tellg();

                switch (Block.BlockID)
                {
                        case PSP_LAYER_START_BLOCK:
                                if (!ReadLayerBlock())
                                        return false;
                                break;

                        case PSP_ALPHA_BANK_BLOCK:
                                if (!ReadAlphaBlock())
                                        return false;
                                break;

                        case PSP_COLOR_BLOCK:
                                if (!ReadPalette())
                                        return false;
                                break;
                }

                // Skip to next block just in case we didn't read the entire block.
                frs.seekg(Pos + Block.BlockLen, ios::beg);

        }
        while (1);

        return true;
}

bool fmt_codec::ReadLayerBlock()
{
        BLOCKHEAD               Block;
        LAYERINFO_CHUNK         LayerInfo;
        LAYERBITMAP_CHUNK       Bitmap;
        ILuint                  ChunkSize, Padding, i;
        ILushort                NumChars;

        // Layer sub-block header
        if (!frs.readK(&Block, sizeof(Block)))
                return false;

        if(Header.MajorVersion == 3)
            frs.readK(&Block.BlockLen, sizeof(Block.BlockLen));

        if (Block.HeadID[0] != 0x7E || Block.HeadID[1] != 0x42 || Block.HeadID[2] != 0x4B || Block.HeadID[3] != 0x00)
            return false;

        if (Block.BlockID != PSP_LAYER_BLOCK)
            return false;

        if (Header.MajorVersion == 3)
        {
                frs.seekg(256, ios::cur);  // We don't care about the name of the layer.
                frs.readK(&LayerInfo, sizeof(LayerInfo));

                if(!frs.readK(&Bitmap, sizeof(Bitmap)))
                    return false;
        }
        else
        {  // Header.MajorVersion >= 4
                frs.readK(&ChunkSize, sizeof(ChunkSize));
                frs.readK(&NumChars, sizeof(NumChars));
                frs.seekg(NumChars, ios::cur);  // We don't care about the layer's name.

                ChunkSize -= (2 + 4 + NumChars);

                if (!frs.readK(&LayerInfo, std::min(sizeof(LayerInfo), size_t(ChunkSize))))
                        return false;

                // Can have new entries in newer versions of the spec (5.0).
                Padding = (ChunkSize) - sizeof(LayerInfo);

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);

                frs.readK(&ChunkSize, sizeof(ChunkSize));

                if (!frs.readK(&Bitmap, sizeof(Bitmap)))
                        return false;

                Padding = (ChunkSize - 4) - sizeof(Bitmap);

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);
        }

        Channels = new ILubyte* [Bitmap.NumChannels];

        if (!Channels)
            return false;

        NumChannels = Bitmap.NumChannels;

        for (i = 0; i < NumChannels; i++)
            Channels[i] = 0;

        for (i = 0; i < NumChannels; i++)
        {
                Channels[i] = GetChannel();

                if(!Channels[i])
                        return false;
        }

        return true;
}

bool fmt_codec::ReadAlphaBlock()
{
        BLOCKHEAD               Block;
        ALPHAINFO_CHUNK AlphaInfo;
        ALPHA_CHUNK             AlphaChunk;
        ILushort                NumAlpha, StringSize;
        ILuint                  ChunkSize, Padding;

        if (Header.MajorVersion == 3) {
                frs.readK(&NumAlpha, sizeof(NumAlpha));
        }
        else {
                frs.readK(&ChunkSize, sizeof(ChunkSize));
                frs.readK(&NumAlpha, sizeof(NumAlpha));

                Padding = (ChunkSize - 4 - 2);

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);
        }

        // Alpha channel header
        if (!frs.readK(&Block, sizeof(Block)))
                return false;

        if (Header.MajorVersion == 3)
                frs.readK(&Block.BlockLen, sizeof(Block.BlockLen));

        if (Block.HeadID[0] != 0x7E || Block.HeadID[1] != 0x42 ||
                Block.HeadID[2] != 0x4B || Block.HeadID[3] != 0x00) {
                        return false;
        }
        if (Block.BlockID != PSP_ALPHA_CHANNEL_BLOCK)
                return false;

        if (Header.MajorVersion >= 4)
        {
                frs.readK(&ChunkSize, sizeof(ChunkSize));
                frs.readK(&StringSize, sizeof(StringSize));

                frs.seekg(StringSize, ios::cur);

                if (!frs.readK(&AlphaInfo, sizeof(AlphaInfo)))
                        return false;

                Padding = (ChunkSize - 4 - 2 - StringSize - sizeof(AlphaInfo));

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);

                frs.readK(&ChunkSize, sizeof(ChunkSize));

                if (!frs.readK(&AlphaChunk, sizeof(AlphaChunk)))
                        return false;

                Padding = (ChunkSize - 4 - sizeof(AlphaChunk));

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);
        }
        else {
                frs.seekg(256, ios::cur);
                frs.readK(&AlphaInfo, sizeof(AlphaInfo));

                if(!frs.readK(&AlphaChunk, sizeof(AlphaChunk)))
                        return false;
        }

        Alpha = GetChannel();

        if (!Alpha)
                return false;

        return true;
}

ILubyte *fmt_codec::GetChannel()
{
        BLOCKHEAD       Block;
        CHANNEL_CHUNK   Channel;
        ILubyte         *CompData = 0, *Data = 0;
        ILuint          ChunkSize, Padding;

        if (!frs.readK(&Block, sizeof(Block)))
                return 0;

        if (Header.MajorVersion == 3)
            frs.readK(&Block.BlockLen, sizeof(Block.BlockLen));

        if (Block.HeadID[0] != 0x7E || Block.HeadID[1] != 0x42 ||  Block.HeadID[2] != 0x4B || Block.HeadID[3] != 0x00)
            return 0;

        if (Block.BlockID != PSP_CHANNEL_BLOCK)
            return 0;

        if (Header.MajorVersion >= 4)
        {
                frs.readK(&ChunkSize, sizeof(ChunkSize));

                if (!frs.readK(&Channel, sizeof(Channel)))
                        return 0;

                Padding = (ChunkSize - 4) - sizeof(Channel);

                if (Padding > 0)
                        frs.seekg(Padding, ios::cur);
        }
        else
        {
                if (!frs.readK(&Channel, sizeof(Channel)))
                        return 0;
        }

        CompData = new ILubyte [Channel.CompLen];

        if (!CompData)
            return 0;

        if (!frs.readK(CompData, Channel.CompLen))
        {
                delete CompData;
                return 0;
        }

        if(AttChunk.Compression != PSP_COMP_NONE)
        {
            Data = new ILubyte [AttChunk.Width * AttChunk.Height];

            if (!Data)
            {
                    delete CompData;
                    return 0;
            }
        }

        switch (AttChunk.Compression)
        {
                case PSP_COMP_NONE:
                    return CompData;

                case PSP_COMP_RLE:
                    if (!UncompRLE(CompData, Data, Channel.CompLen))
                    {
                        delete Data;
                        delete CompData;
                        return 0;
                    }
                break;

                default:
                    delete Data;
                    delete CompData;
                return 0;
        }

        delete CompData;

        return Data;
}

bool fmt_codec::UncompRLE(ILubyte *CompData, ILubyte *Data, ILuint CompLen)
{
        ILubyte Run, Colour;
        ILint   i, Count;

        for (i = 0, Count = 0; i < (ILint)CompLen; ) {
                Run = *CompData++;
                i++;
                if (Run > 128) {
                        Run -= 128;
                        Colour = *CompData++;
                        i++;
                        memset(Data, Colour, Run);
                }
                else {
                        memcpy(Data, CompData, Run);
                        CompData += Run;
                        i += Run;
                }
                Data += Run;
                Count += Run;
        }

        return true;
}

bool fmt_codec::ReadPalette()
{
        ILuint ChunkSize, PalCount, Padding;
        RGBA rgba;

        if (Header.MajorVersion >= 4)
        {
            frs.readK(&ChunkSize, sizeof(ChunkSize));
            frs.readK(&PalCount, sizeof(PalCount));

            Padding = (ChunkSize - 4 - 4);

            if (Padding > 0)
                frs.seekg(Padding, ios::cur);
        }
        else
            frs.readK(&PalCount, sizeof(PalCount));

        pal = new RGBA [PalCount];

        if(!pal)
            return false;

        RGBA *ppal = pal;

        for(u32 i = 0;i < PalCount;i++)
        {
            if(!frs.readK(&rgba, sizeof(RGBA)))
                return false;

            ppal->r = rgba.b;
            ppal->g = rgba.g;
            ppal->b = rgba.r;
            ppal->a = rgba.a;

            ppal++;
        }

        return true;
}

/******************** ksquirrel-libs stuff ****************************/

s32 fmt_codec::read_init(const std::string &file)
{
    Channels = 0;
    Alpha = 0;
    pal = 0;

    frs.open(file.c_str(), ios::binary | ios::in);

    if(!frs.good())
        return SQE_R_NOFILE;

    currentImage = -1;
    read_error = false;
    finfo.animated = false;

    if(!iGetPspHead())
        return SQE_R_BADFILE;

    if(!iCheckPsp())
        return SQE_R_BADFILE;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;

    if(currentImage)
        return SQE_NOTOK;

    if(!ReadGenAttributes())
        return SQE_R_BADFILE;

    if(!ParseChunks())
        return SQE_R_BADFILE;

//    if(!AssembleImage())
//        return SQE_R_BADFILE;

    fmt_image image;

    image.w = AttChunk.Width;
    image.h = AttChunk.Height;
    image.compression = (AttChunk.Compression == PSP_COMP_RLE ? "RLE" : "-");
    image.bpp = (NumChannels == 1 ? 8 : ((Alpha || NumChannels == 4) ? 32 : 24));
    image.colorspace = fmt_utils::colorSpaceByBpp(image.bpp);

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    line = -1;

    return SQE_OK;
}

s32 fmt_codec::read_scanline(RGBA *scan)
{
    line++;

    fmt_image *im = image(currentImage);

    u32 i, j;
    u32 ifrom = line * im->w;
    u32 ito = ifrom + im->w;

    if (NumChannels == 1)
    {
        memset(scan, 0, im->w * sizeof(RGBA));

        for (i = ifrom, j = 0; i < ito; i++, j++)
        {
            scan[j] = pal[Channels[0][i]];
            scan[j].a = 255;
        }
    }
    else
    {
        if (Alpha)
        {
            memset(scan, 0, im->w * sizeof(RGBA));
            u8 *data = (u8 *)scan;

            for (i = ifrom, j = 0; i < ito; i++, j += 4)
            {
                data[j  ] = Channels[0][i];
                data[j+1] = Channels[1][i];
                data[j+2] = Channels[2][i];
                data[j+3] = Alpha[i];
            }
        }
        // 3 channels, or 4 without alpha
        else if (NumChannels == 3 || NumChannels == 4)
        {
            memset(scan, 0, im->w * sizeof(RGBA));
            u8 *data = (u8 *)scan;

            for (i = ifrom, j = 0; i < ito; i++, j += 4)
            {
                data[j  ] = Channels[0][i];
                data[j+1] = Channels[1][i];
                data[j+2] = Channels[2][i];
                data[j+3] = 255;
            }
        }
        else
            return SQE_R_BADFILE;
    }

    return SQE_OK;
}

void fmt_codec::read_close()
{
    frs.close();

    if(Channels)
    {
        for (u32 i = 0; i < NumChannels; i++)
            delete Channels[i];

        delete Channels;
    }

    delete Alpha;
    delete pal;

    Channels = 0;
    Alpha = 0;
    pal = 0;

    finfo.meta.clear();
    finfo.image.clear();
}

#include "fmt_codec_cd_func.h"

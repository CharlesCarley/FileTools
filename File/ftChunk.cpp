/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include "ftChunk.h"
#include <cstdio>
#include "Utils/skPlatformHeaders.h"
#include "ftEndianUtils.h"
#include "ftStreams.h"

using namespace ftEndianUtils;
using namespace ftFlags;

const ftChunk ftChunkUtils::BlankChunk = {};

SKsize ftChunkUtils::write(ftChunk* src, skStream* stream)
{
    SKsize size = 0;
    size += stream->write(src, BlockSize);
    size += stream->write((void*)src->address, src->length);
    return size;
}

SKsize ftChunkUtils::scan(ftChunkScan* dest, skStream* stream, int flags)
{
    SKsize bytesRead = 0;
    SKsize blockLen  = BlockSize;

    if (FT_VOID8)
    {
        if (flags & FH_VAR_BITS)
            blockLen = Block32;
    }
    else
    {
        if (flags & FH_VAR_BITS)
            blockLen = Block64;
    }

    if ((bytesRead = stream->read(dest, BlockScan)) <= 0)
        return FS_INV_READ;

    bytesRead += blockLen - BlockScan;
    if (!stream->seek(blockLen - BlockScan, SEEK_CUR))
        return FS_INV_READ;

    if (flags & FH_ENDIAN_SWAP)
    {
        if ((dest->code & 0xFFFF) == 0)
            dest->code >>= 16;
        dest->length = swap32(dest->length);
    }
    return bytesRead;
}

SKsize ftChunkUtils::read(ftChunk* dest, skStream* stream, int flags)
{
    SKsize   bytesRead = 0;
    ftChunk* tmp;

    if (FT_VOID8)
    {
        ftChunk64 c64{};
        if (flags & FH_VAR_BITS)
        {
            ftChunk32 src{};
            if ((bytesRead = stream->read(&src, Block32)) <= 0)
                return FS_INV_READ;

            ftByteInteger ptr = {0};

            c64.code     = src.code;
            c64.length   = src.length;
            ptr.m_int[0] = src.address;
            c64.address  = ptr.m_ptr;
            c64.structId = src.structId;
            c64.count    = src.count;
        }
        else
        {
            if ((bytesRead = stream->read(&c64, BlockSize)) <= 0)
                return FS_INV_READ;
        }

        tmp = (ftChunk*)(&c64);
    }
    else
    {
        ftChunk32 c32{};
        if (flags & FH_VAR_BITS)
        {
            ftChunk64 src{};
            if ((bytesRead = stream->read(&src, Block64)) <= 0)
                return FS_INV_READ;

            ftByteInteger ptr  = {0};
            c32.code           = src.code;
            c32.length         = src.length;
            c32.structId       = src.structId;
            c32.count          = src.count;

            ptr.m_ptr = src.address;
            if (ptr.m_int[0] != 0)
                c32.address = ptr.m_int[0];
            else
                c32.address = ptr.m_int[1];
        }
        else
        {
            if ((bytesRead = stream->read(&c32, BlockSize)) <= 0)
                return FS_INV_READ;
        }

        tmp = (ftChunk*)(&c32);
    }

    if (flags & FH_ENDIAN_SWAP)
    {
        if ((tmp->code & 0xFFFF) == 0)
            tmp->code >>= 16;

        tmp->length   = swap32(tmp->length);
        tmp->count    = swap32(tmp->count);
        tmp->structId = swap32(tmp->structId);
    }

    if (tmp->length == SK_NPOS32)
        return FS_INV_LENGTH;

    if (tmp->count < 1 || tmp->count == SK_NPOS32)
        return FS_INV_LENGTH;

    memcpy(dest, tmp, BlockSize);
    return bytesRead;
}

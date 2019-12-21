/*
-------------------------------------------------------------------------------

    Copyright (c) Charles Carley.

    Contributor(s): none yet.

-------------------------------------------------------------------------------
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
#include <stdio.h>

#include "ftChunk.h"
#include "ftEndianUtils.h"
#include "ftFile.h"
#include "ftLogger.h"
#include "ftStreams.h"

using namespace ftEndianUtils;


const ftChunk ftChunkUtils::BLANK_CHUNK = {
    0,  // m_code
    0,  // m_sizeInBytes
    0,  // m_old
    0,  // m_typeid
    0,  // m_nr
};


FBTsize ftChunkUtils::write(ftChunk* src, skStream* stream)
{
    FBTsize size = 0;
    size += stream->write(src, BlockSize);
    size += stream->write((void*)src->m_old, src->m_len);
    return size;
}



FBTsize ftChunkUtils::scan(ftChunkScan* dest, skStream* stream, int flags)
{
    FBTsize bytesRead  = 0;
    
    if (FT_VOID8)
    {
        if (flags & ftFile::FH_VAR_BITS)
        {
            if ((bytesRead = stream->read(dest, BlockScan)) <= 0)
                return ftFile::FS_INV_READ;

            bytesRead += Block32 - BlockScan;
            stream->seek(Block32 - BlockScan, SEEK_CUR);
        }
        else
        {
            if ((bytesRead = stream->read(dest, BlockScan)) <= 0)
                return ftFile::FS_INV_READ;

            bytesRead += BlockSize - BlockScan;
            stream->seek(BlockSize - BlockScan, SEEK_CUR);
        }
    }
    else
    {
        if (flags & ftFile::FH_VAR_BITS)
        {
            if ((bytesRead = stream->read(dest, BlockScan)) <= 0)
                return ftFile::FS_INV_READ;

            bytesRead += Block64 - BlockScan;
            stream->seek(Block64 - BlockScan, SEEK_CUR);
        }
        else
        {
            if ((bytesRead = stream->read(dest, BlockScan)) <= 0)
                return ftFile::FS_INV_READ;

            bytesRead += BlockSize - BlockScan;
            stream->seek(BlockSize - BlockScan, SEEK_CUR);
        }
    }

    if (flags & ftFile::FH_ENDIAN_SWAP)
    {
        if ((dest->m_code & 0xFFFF) == 0)
            dest->m_code >>= 16;

        dest->m_len = swap32(dest->m_len);
    }
    return bytesRead;
}



FBTsize ftChunkUtils::read(ftChunk* dest, skStream* stream, int flags)
{
    FBTsize bytesRead  = 0;
    bool    bitsVary   = (flags & ftFile::FH_VAR_BITS) != 0;
    bool    swapEndian = (flags & ftFile::FH_ENDIAN_SWAP) != 0;

    ftChunk* cpy;
    if (FT_VOID8)
    {
        ftChunk64 c64;

        if (bitsVary)
        {
            ftChunk32 src;
            if ((bytesRead = stream->read(&src, Block32)) <= 0)
                return ftFile::FS_INV_READ;

            c64.m_code = src.m_code;
            c64.m_len  = src.m_len;
            union {
                FBTuint64 m_ptr;
                FBTuint32 m_doublePtr[2];
            } ptr;
            ptr.m_doublePtr[0] = src.m_old;
            ptr.m_doublePtr[1] = 0;

            c64.m_old    = ptr.m_ptr;
            c64.m_typeid = src.m_typeid;
            c64.m_nr     = src.m_nr;
        }
        else
        {
            if ((bytesRead = stream->read(&c64, BlockSize)) <= 0)
                return ftFile::FS_INV_READ;
        }

        if (swapEndian)
        {
            if ((c64.m_code & 0xFFFF) == 0)
                c64.m_code >>= 16;
            c64.m_len    = swap32(c64.m_len);
            c64.m_nr     = swap32(c64.m_nr);
            c64.m_typeid = swap32(c64.m_typeid);
        }
        cpy = (ftChunk*)(&c64);
    }
    else
    {
        ftChunk32 c32;

        if (bitsVary)
        {
            ftChunk64 src;
            if ((bytesRead = stream->read(&src, Block64)) <= 0)
                return ftFile::FS_INV_READ;

            c32.m_code   = src.m_code;
            c32.m_len    = src.m_len;
            c32.m_typeid = src.m_typeid;
            c32.m_nr     = src.m_nr;

            union {
                FBTuint64 m_ptr;
                FBTuint32 m_doublePtr[2];
            } ptr;

            ptr.m_doublePtr[0] = 0;
            ptr.m_doublePtr[1] = 0;

            ptr.m_ptr = src.m_old;

            if (ptr.m_doublePtr[0] != 0)
                c32.m_old = ptr.m_doublePtr[0];
            else
                c32.m_old = ptr.m_doublePtr[1];
        }
        else
        {
            if ((bytesRead = stream->read(&c32, BlockSize)) <= 0)
                return ftFile::FS_INV_READ;
        }


        if (swapEndian)
        {
            if ((c32.m_code & 0xFFFF) == 0)
                c32.m_code >>= 16;

            c32.m_len    = swap32(c32.m_len);
            c32.m_nr     = swap32(c32.m_nr);
            c32.m_typeid = swap32(c32.m_typeid);
        }

        cpy = (ftChunk*)(&c32);
    }

    if (cpy->m_len == SK_NPOS32)
        return ftFile::FS_INV_LENGTH;

    ::memcpy(dest, cpy, BlockSize);
    return bytesRead;
}

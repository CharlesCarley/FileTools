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
#define FT_IN_SOURCE_FILE
#include "ftScanDNA.h"
#include "ftChunk.h"
#include "ftEndianUtils.h"
#include "ftPlatformHeaders.h"
#include "ftTables.h"

using namespace ftFlags;
using namespace ftEndianUtils;


ftScanDNA::ftScanDNA() :
    m_foundBlock(0),
    m_foundLen(0),
    m_headerFlags(0)
{
}


ftScanDNA::~ftScanDNA()
{
}

int ftScanDNA::findHeaderFlags(skStream *stream)
{
    int status = FS_OK;

    char  buf[13] = {};
    char *magic;

    SKsize read = stream->read(buf, 12);
    if (read != SK_NPOS && read > 0)
    {
        magic = &buf[7];

        if (*magic == FM_64_BIT || *magic == FM_32_BIT)
        {
            if (*(magic++) == FM_64_BIT)
            {
                m_headerFlags |= FH_CHUNK_64;
                if (FT_VOID4)
                    m_headerFlags |= FH_VAR_BITS;
            }
            else if (FT_VOID8)
                m_headerFlags |= FH_VAR_BITS;

            char endian = *(magic++);
            if (endian == FM_BIG_ENDIAN || endian == FM_LITTLE_ENDIAN)
            {
                int current = (int)getEndian();

                if (endian == FM_BIG_ENDIAN)
                {
                    if (current == ftEndian::FT_ENDIAN_IS_LITTLE)
                        m_headerFlags |= FH_ENDIAN_SWAP;
                }
                else if (endian == FM_LITTLE_ENDIAN)
                {
                    if (current == ftEndian::FT_ENDIAN_IS_BIG)
                        m_headerFlags |= FH_ENDIAN_SWAP;
                }
                else
                    status = FS_INV_HEADER_STR;
            }
        }
        else
            status = FS_INV_HEADER_STR;
    }
    else
        status = FS_INV_READ;
    return status;
}

int ftScanDNA::scan(skStream *stream)
{
    int         status = FS_OK;
    FBTsize     bytesRead;
    ftChunkScan scan = {0, 0};

    while (scan.m_code != ftIdNames::ENDB &&
           scan.m_code != ftIdNames::DNA1 &&
           status == FS_OK && !stream->eof())
    {
        bytesRead = ftChunkUtils::scan(&scan, stream, m_headerFlags);
        if (bytesRead <= 0 || bytesRead == SK_NPOS)
            status = FS_INV_READ;
        else if (scan.m_code != ftIdNames::ENDB)
        {
            if (scan.m_code == ftIdNames::DNA1)
            {
                // This block needs to stay alive as long as m_file is valid.
                // The names of the types and the names of the type-name
                // declarations are referenced out of this block.
                void *found = ::malloc(scan.m_len);
                if (!found)
                    status = FS_BAD_ALLOC;
                else
                {
                    if (stream->read(found, scan.m_len) <= 0)
                    {
                        free(found);
                        status = FS_INV_READ;
                    }
                    else
                    {
                        m_foundBlock = found;
                        m_foundLen   = scan.m_len;
                    }
                }
            }
            else
            {
                if (scan.m_len > 0 && scan.m_len != SK_NPOS32)
                {
                    if (!stream->seek(scan.m_len, SEEK_CUR))
                        status = FS_INV_READ;
                }
                else
                    status = FS_INV_LENGTH;
            }
        }
    }
    return status;
}
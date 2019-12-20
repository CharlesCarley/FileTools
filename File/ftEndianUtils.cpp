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
#include "ftEndianUtils.h"


ftEndian ftEndianUtils::getEndian(void)
{
    ftEndianTest e;
    e.test = (FBTint32)FT_ENDIAN_IS_LITTLE;
    return static_cast<ftEndian>(e.bo[0]);
}


bool ftEndianUtils::isEndian(const ftEndian& endian)
{
    ftEndianTest e;
    e.test = endian;
    return static_cast<ftEndian>(e.bo[0]) == endian;
}


FBTuint16 ftEndianUtils::swap16(FBTuint16 in)
{
    return static_cast<FBTuint16>(((in & 0xFF00) >> 8) | ((in & 0x00FF) << 8));
}

FBTuint32 ftEndianUtils::swap32(const FBTuint32& in)
{
    return (((in & 0xFF000000) >> 24) | ((in & 0x00FF0000) >> 8) | ((in & 0x0000FF00) << 8) | ((in & 0x000000FF) << 24));
}

FBTint16 ftEndianUtils::swap16(FBTint16 in)
{
    return swap16(static_cast<FBTuint16>(in));
}

FBTint32 ftEndianUtils::swap32(const FBTint32& in)
{
    return swap32(static_cast<FBTuint32>(in));
}

FBTuint64 ftEndianUtils::swap64(const FBTuint64& in)
{
    FBTuint64       r   = 0;
    const FBTubyte* src = reinterpret_cast<const FBTubyte*>(&in);
    FBTubyte*       dst = reinterpret_cast<FBTubyte*>(&r);

    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
    return r;
}


void ftEndianUtils::swap16(FBTuint16* sp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *sp = swap16(*sp);
        ++sp;
    }
}

void ftEndianUtils::swap32(FBTuint32* ip, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *ip = swap32(*ip);
        ++ip;
    }
}


void ftEndianUtils::swap64(FBTuint64* dp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *dp = swap64(*dp);
        ++dp;
    }
}

FBTint64 ftEndianUtils::swap64(const FBTint64& in)
{
    return swap64(static_cast<FBTuint64>(in));
}

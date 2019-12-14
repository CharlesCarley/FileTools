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
#ifndef _ftTypes_h_
#define _ftTypes_h_

#ifndef ftHAVE_CONFIG
#include "ftConfig.h"
#endif

#include "Utils/skArray.h"
#include "Utils/skFixedString.h"
#include "memory.h"
#include "string.h"

#define ftID(a, b, c, d) SK_ID(a, b, c, d)
#define ftID2(a, b) SK_ID2(a, b)
#define FT_INLINE SK_INLINE


// Integer types
typedef SKlong   FBTlong;
typedef SKulong  FBTulong;
typedef SKint32  FBTint32;
typedef SKuint32 FBTuint32;
typedef SKint16  FBTint16;
typedef SKuint16 FBTuint16;
typedef SKint8   FBTint8;
typedef SKuint8  FBTuint8;
typedef SKuint8  FBTubyte;
typedef SKint8   FBTbyte;
typedef SKint64  FBTint64;
typedef SKuint64 FBTuint64;

typedef SKuintPtr  FBTuintPtr;
typedef SKintPtr   FBTintPtr;
typedef FBTuintPtr FBTsize;

// Scalar types are defined as [a-z]"Scalar"
#ifdef ftSCALAR_DOUBLE
#define scalar_t double
#else
#define scalar_t float
#endif

typedef FBTuint32 FBTsizeType;
typedef FBTuint32 FBThash;
typedef FBTuint16 FBTtype;

#define _ftCACHE_LIMIT 999



typedef enum ftEndian
{
    FT_ENDIAN_IS_BIG    = 0,
    FT_ENDIAN_IS_LITTLE = 1,
    FT_ENDIAN_NATIVE,
} ftEndian;

typedef union ftEndianTest {
    FBTbyte  bo[4];
    FBTint32 test;
} ftEndianTest;



FT_INLINE ftEndian ftGetEndian(void)
{
    ftEndianTest e;
    e.test = FT_ENDIAN_IS_LITTLE;
    return static_cast<ftEndian>(e.bo[0]);
}


FT_INLINE bool ftIsEndian(const ftEndian& endian)
{
    ftEndianTest e;
    e.test = endian;
    return static_cast<ftEndian>(e.bo[0]) == endian;
}

FT_INLINE FBTuint16 ftSwap16(FBTuint16 in)
{
    return static_cast<FBTuint16>(((in & 0xFF00) >> 8) | ((in & 0x00FF) << 8));
}

FT_INLINE FBTuint32 ftSwap32(const FBTuint32& in)
{
    return (((in & 0xFF000000) >> 24) | ((in & 0x00FF0000) >> 8) | ((in & 0x0000FF00) << 8) | ((in & 0x000000FF) << 24));
}

FT_INLINE FBTint16 ftSwap16(FBTint16 in)
{
    return ftSwap16(static_cast<FBTuint16>(in));
}

FT_INLINE FBTint32 ftSwap32(const FBTint32& in)
{
    return ftSwap32(static_cast<FBTuint32>(in));
}


FT_INLINE FBTuint64 ftSwap64(const FBTuint64& in)
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


FT_INLINE void ftSwap16(FBTuint16* sp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *sp = ftSwap16(*sp);
        ++sp;
    }
}

FT_INLINE void ftSwap32(FBTuint32* ip, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *ip = ftSwap32(*ip);
        ++ip;
    }
}


FT_INLINE void ftSwap64(FBTuint64* dp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *dp = ftSwap64(*dp);
        ++dp;
    }
}


FT_INLINE FBTint64 ftSwap64(const FBTint64& in)
{
    return ftSwap64(static_cast<FBTuint64>(in));
}



enum ftPointerLen
{
    FT_VOIDP = sizeof(void*),
    FT_VOID4 = FT_VOIDP == 4,
    FT_VOID8 = FT_VOIDP == 8,
};



class ftList
{
public:
    struct Link
    {
        Link* next;
        Link* prev;
    };


    Link* first;
    Link* last;

public:
    ftList() :
        first(0),
        last(0)
    {
    }
    ~ftList()
    {
        clear();
    }

    void clear(void)
    {
        first = last = 0;
    }

    void push_back(void* v)
    {
        Link* link = ((Link*)v);
        if (!link)
            return;

        link->prev = last;
        if (last)
            last->next = link;

        if (!first)
            first = link;

        last = link;
    }
};



#define ftCharNEq(a, b, n) ((a && b) && !strncmp(a, b, n))
#define ftCharEq(a, b) ((a && b) && (*a == *b) && !strcmp(a, b))
#define ftCharEqL(a, b, l) ((a && b) && (*a == *b) && !memcmp(a, b, l))
#define ftStrLen(a) ::strlen(a)


#define ftFixedString skFixedString


#endif  //_ftTypes_h_

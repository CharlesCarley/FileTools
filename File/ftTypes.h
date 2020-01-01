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

#define FT_TYPEID(a, b, c, d) SK_ID(a, b, c, d)
#define FT_TYPEID2(a, b) SK_ID2(a, b)


#define FT_INLINE SK_INLINE

typedef SKlong    FBTlong;
typedef SKulong   FBTulong;
typedef SKint32   FBTint32;
typedef SKuint32  FBTuint32;
typedef SKint16   FBTint16;
typedef SKuint16  FBTuint16;
typedef SKint8    FBTint8;
typedef SKuint8   FBTuint8;
typedef SKuint8   FBTubyte;
typedef SKbyte    FBTbyte;
typedef SKint64   FBTint64;
typedef SKuint64  FBTuint64;
typedef SKuintPtr FBTuintPtr;
typedef SKintPtr  FBTintPtr;
typedef SKuintPtr FBTsize;
typedef FBTsize   FBThash;
typedef FBTuint16 FBTtype;


#ifdef ftSCALAR_DOUBLE
#define scalar_t double
#else
#define scalar_t float
#endif

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

    bool erase(void* vp)
    {
        Link* link = ((Link*)vp);
        if (!link)
            return false;

        if (link->next)
            link->next->prev = link->prev;
        if (link->prev)
            link->prev->next = link->next;
        if (last == link)
            last = link->prev;
        if (first == link)
            first = link->next;
        return true;
    }

    bool remove(void* vp)
    {
        return erase(vp);
    }
};


#define ftCharNEq(a, b, n) ((a && b) && !strncmp(a, b, n))
#define ftCharEq(a, b) ((a && b) && (*a == *b) && !strcmp(a, b))
#define ftCharEqL(a, b, l) ((a && b) && (*a == *b) && !memcmp(a, b, l))
#define ftStrLen(a) ::strlen(a)


#define ftFixedString skFixedString

typedef skFixedString<12> ftHeader;


typedef union FBTByteInteger {
    FBTuint64 m_ptr;
    FBTuint32 m_int[2];
    FBTuint16 m_short[4];
    FBTuint8  m_byte[8];
} FBTByteInteger;

class skStream;
class ftTables;
class ftStruct;
struct ftName;
class ftMember;
class ftTables;
class ftStruct;



namespace ftFlags
{
    enum FileMagic
    {
        FM_BIG_ENDIAN    = 0x56,
        FM_LITTLE_ENDIAN = 0x76,
        FM_32_BIT        = 0x5F,
        FM_64_BIT        = 0x2D,
        HEADER_OFFSET    = 0x0C,
    };

    enum FileStatus
    {
        FS_STATUS_MIN = -18,
        FS_LINK_FAILED,
        FS_INV_INSERT,
        FS_BAD_ALLOC,
        FS_INV_VALUE,
        FS_INV_SIZE,
        FS_DUPLICATE_BLOCK,
        FS_INV_READ,
        FS_INV_LENGTH,
        FS_INV_HEADER_STR,
        FS_TABLE_INIT_FAILED,
        FS_OVERFLOW,
        FS_FAILED,

        // Table codes
        RS_INVALID_PTR,
        RS_INVALID_CODE,
        RS_LIMIT_REACHED,
        RS_BAD_ALLOC,
        RS_MIS_ALIGNED,

        // This should always be zero
        // until < 0 tests are removed
        // and replaced with != FS_OK
        FS_OK,
    };

    enum ParseMode
    {
        PM_UNCOMPRESSED = 0,
        PM_COMPRESSED,
        PM_READTOMEMORY,
    };

    enum FileHeader
    {
        FH_ENDIAN_SWAP = 1 << 0,
        FH_CHUNK_64    = 1 << 1,
        FH_VAR_BITS    = 1 << 2,
    };

    enum LogFlags
    {
        LF_NONE             = 0,
        LF_ONLY_ERR         = 1 << 0,
        LF_READ_CHUNKS      = 1 << 1,
        LF_WRITE_CHUNKS     = 1 << 2,
        LF_WRITE_LINK       = 1 << 3,
        LF_DO_CHECKS        = 1 << 4,
        LF_DIAGNOSTICS      = 1 << 5,
        LF_DUMP_NAME_TABLE  = 1 << 6,
        LF_DUMP_TYPE_TABLE  = 1 << 7,
        LF_DUMP_SIZE_TABLE  = 1 << 8,
        LF_DUMP_SKIP        = 1 << 9,
        LF_DUMP_CAST        = 1 << 10,
        LF_UNRESOLVED       = 1 << 11,
        LF_DUMP_MEMBER_HASH = 1 << 12,
    };
}


#endif  //_ftTypes_h_

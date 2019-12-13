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



#if (defined(DEBUG) || defined(_DEBUG)) && ftUSE_DEBUG_ASSERTS == 1
#include <assert.h>  // Keep this the only std include in headers
#define FT_DEBUG 1
#define ftASSERT(x) assert(x)
#else
#define ftASSERT(x) ((void)0)
#endif

#ifdef ftUSE_COMPILER_CHECKS
#define ftASSERTCOMP(x, n) typedef bool x[(n) ? 1 : -1];
#else
#define ftASSERTCOMP(x, n)
#endif


#define ftPLATFORM_WIN32 0
#define ftPLATFORM_LINUX 2
#define ftPLATFORM_APPLE 3

#if defined(_WIN32)
#define ftPLATFORM ftPLATFORM_WIN32
#elif defined(__APPLE__)
#define ftPLATFORM ftPLATFORM_APPLE
#else
#define ftPLATFORM ftPLATFORM_LINUX
#endif

#define ftCOMPILER_MSVC 0
#define ftCOMPILER_GNU 1

#if defined(_MSC_VER)
#define ftCOMPILER ftCOMPILER_MSVC
#elif defined(__GNUC__)
#define ftCOMPILER ftCOMPILER_GNU
#else
#error unknown compiler
#endif

#define ftENDIAN_LITTLE 0
#define ftENDIAN_BIG 1

#if defined(__sgi) || defined(__sparc) ||     \
    defined(__sparc__) || defined(__PPC__) || \
    defined(__ppc__) || defined(__BIG_ENDIAN__)
#define ftENDIAN ftENDIAN_BIG
#else
#define ftENDIAN ftENDIAN_LITTLE
#endif

#if ftENDIAN == ftENDIAN_BIG
#define ftID(a, b, c, d) ((int)(a) << 24 | (int)(b) << 16 | (c) << 8 | (d))
#define ftID2(c, d) ((c) << 8 | (d))
#else
#define ftID(a, b, c, d) ((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))
#define ftID2(c, d) ((d) << 8 | (c))
#endif

#define ftARCH_32 0
#define ftARCH_64 1

#if defined(__x86_64__) || defined(_M_X64) ||       \
    defined(__powerpc64__) || defined(__alpha__) || \
    defined(__ia64__) || defined(__s390__) ||       \
    defined(__s390x__)
#define ftARCH ftARCH_64
#else
#define ftARCH ftARCH_32
#endif


#if ftPLATFORM == ftPLATFORM_WIN32
#if defined(__MINGW32__) || \
    defined(__CYGWIN__) ||  \
    (defined(_MSC_VER) && _MSC_VER < 1300)
#define FT_INLINE inline
#else
#define FT_INLINE __forceinline
#endif
#else
#define FT_INLINE inline
#endif

// Integer types
typedef long           FBTlong;
typedef unsigned long  FBTulong;
typedef int            FBTint32;
typedef unsigned int   FBTuint32;
typedef short          FBTint16;
typedef unsigned short FBTuint16;
typedef signed char    FBTint8;
typedef unsigned char  FBTuint8;
typedef unsigned char  FBTubyte;
typedef signed char    FBTbyte;
typedef bool           FBTint1;


#if ftCOMPILER == ftCOMPILER_GNU
typedef long long          FBTint64;
typedef unsigned long long FBTuint64;
#else
typedef __int64          FBTint64;
typedef unsigned __int64 FBTuint64;
#endif

// Arch dependent types

#if ftARCH == ftARCH_64
typedef FBTuint64 FBTuintPtr;
typedef FBTint64  FBTintPtr;
#else
typedef FBTuint32        FBTuintPtr;
typedef FBTint32         FBTintPtr;
#endif

typedef FBTuintPtr FBTsize;

// Scalar types are defined as [a-z]"Scalar"
#ifdef ftSCALAR_DOUBLE
#define scalar_t double
#else
#define scalar_t float
#endif

// Type for arrays & tables (Always unsigned & 32bit)
typedef FBTuint32 FBTsizeType;
const FBTuint32   ftNPOS = (FBTuint32)-1;
typedef FBTuint32 FBThash;
typedef FBTuint16 FBTtype;

ftASSERTCOMP(FBTsizeType_NPOS_VALIDATE, ftNPOS == ((FBTuint32)-1));


#define _ftCACHE_LIMIT 999

template <typename T>
FT_INLINE void ftSwap(T& a, T& b)
{
    T t(a);
    a = b;
    b = t;
}


template <typename T>
FT_INLINE T ftMax(const T& a, const T& b)
{
    return a < b ? b : a;
}
template <typename T>
FT_INLINE T ftMin(const T& a, const T& b)
{
    return a < b ? a : b;
}
template <typename T>
FT_INLINE T ftClamp(const T& v, const T& a, const T& b)
{
    return v < a ? a : v > b ? b : v;
}



typedef enum ftEndian
{
    ftENDIAN_IS_BIG    = 0,
    ftENDIAN_IS_LITTLE = 1,
    ftENDIAN_NATIVE,
} ftEndian;

typedef union ftEndianTest {
    FBTbyte  bo[4];
    FBTint32 test;
} ftEndianTest;



FT_INLINE ftEndian ftGetEndian(void)
{
    ftEndianTest e;
    e.test = ftENDIAN_IS_LITTLE;
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
    ftVOID  = sizeof(void*),
    ftVOID4 = ftVOID == 4,
    ftVOID8 = ftVOID == 8,
};


#if ftARCH == ftARCH_64
ftASSERTCOMP(ftVOID8_ASSERT, ftVOID8);
#else
ftASSERTCOMP(ftVOID4_ASSERT, ftVOID4);
#endif



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



// magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/
#define _ftINITIAL_FNV 0x9E3779B1
#define _ftINITIAL_FNV2 0x9E3779B9
#define _ftMULTIPLE_FNV 0x1000193
#define _ftTWHASH(key)   \
    key += ~(key << 15); \
    key ^= (key >> 10);  \
    key += (key << 3);   \
    key ^= (key >> 6);   \
    key += ~(key << 11); \
    key ^= (key >> 16);


class ftCharHashKey
{
protected:
    char*           m_key;
    mutable FBThash m_hash;

public:
    ftCharHashKey() :
        m_key(0),
        m_hash(ftNPOS)
    {
    }
    ftCharHashKey(char* k) :
        m_key(k),
        m_hash(ftNPOS)
    {
        hash();
    }
    ftCharHashKey(const char* k) :
        m_key(const_cast<char*>(k)),
        m_hash(ftNPOS)
    {
    }
    ftCharHashKey(const ftCharHashKey& k) :
        m_key(k.m_key),
        m_hash(k.m_hash)
    {
        if (m_hash == ftNPOS)
            hash();
    }


    FBThash hash(void) const
    {
        if (!m_key)
            return ftNPOS;
        if (m_hash != ftNPOS)
            return m_hash;

        // Fowler / Noll / Vo (FNV) Hash
        m_hash = (FBThash)_ftINITIAL_FNV;
        for (int i = 0; m_key[i]; i++)
        {
            m_hash = m_hash ^ (m_key[i]);       // xor  the low 8 bits
            m_hash = m_hash * _ftMULTIPLE_FNV;  // multiply by the magic number
        }
        return m_hash;
    }

    FT_INLINE bool operator==(const ftCharHashKey& v) const
    {
        return hash() == v.hash();
    }
    FT_INLINE bool operator!=(const ftCharHashKey& v) const
    {
        return hash() != v.hash();
    }
    FT_INLINE bool operator==(const FBThash& v) const
    {
        return hash() == v;
    }
    FT_INLINE bool operator!=(const FBThash& v) const
    {
        return hash() != v;
    }
};

extern FBTuint32 skHash(const ftCharHashKey& hk);



class ftIntHashKey
{
protected:
    FBTint32 m_key;

public:
    ftIntHashKey() :
        m_key(0)
    {
    }
    ftIntHashKey(FBTint32 k) :
        m_key(k)
    {
    }
    ftIntHashKey(const ftIntHashKey& k) :
        m_key(k.m_key)
    {
    }

    FT_INLINE FBThash hash(void) const
    {
        return static_cast<FBThash>(m_key) * _ftINITIAL_FNV;
    }

    FT_INLINE bool operator==(const ftIntHashKey& v) const
    {
        return hash() == v.hash();
    }
    FT_INLINE bool operator!=(const ftIntHashKey& v) const
    {
        return hash() != v.hash();
    }
    FT_INLINE bool operator==(const FBThash& v) const
    {
        return hash() == v;
    }
    FT_INLINE bool operator!=(const FBThash& v) const
    {
        return hash() != v;
    }
};



class ftSizeHashKey
{
protected:
    FBTsize         m_key;
    mutable FBThash m_hash;

public:
    ftSizeHashKey() :
        m_hash(ftNPOS),
        m_key(0)
    {
    }

    ftSizeHashKey(const FBTsize& k) :
        m_hash(ftNPOS),
        m_key(k)
    {
        hash();
    }

    ftSizeHashKey(const ftSizeHashKey& k) :
        m_hash(ftNPOS),
        m_key(k.m_key)
    {
        hash();
    }


    FT_INLINE FBThash hash(void) const
    {
        if (m_hash != ftNPOS)
            return m_hash;
        m_hash = (FBThash)m_key;
        _ftTWHASH(m_hash);
        return m_hash;
    }

    FT_INLINE bool operator==(const ftSizeHashKey& v) const
    {
        return hash() == v.hash();
    }
    FT_INLINE bool operator!=(const ftSizeHashKey& v) const
    {
        return hash() != v.hash();
    }
    FT_INLINE bool operator==(const FBThash& v) const
    {
        return hash() == v;
    }
    FT_INLINE bool operator!=(const FBThash& v) const
    {
        return hash() != v;
    }
};


template <typename T>
class ftTHashKey
{
protected:
    T*              m_key;
    mutable FBThash m_hash;

public:
    ftTHashKey() :
        m_key(0),
        m_hash(ftNPOS)
    {
        hash();
    }
    ftTHashKey(T* k) :
        m_key(k),
        m_hash(ftNPOS)
    {
        hash();
    }
    ftTHashKey(const ftTHashKey& k) :
        m_key(k.m_key),
        m_hash(k.m_hash)
    {
        if (m_hash == ftNPOS)
            hash();
    }

    FT_INLINE T* key(void)
    {
        return m_key;
    }
    FT_INLINE const T* key(void) const
    {
        return m_key;
    }


    FT_INLINE FBThash hash(void) const
    {
        if (m_hash != ftNPOS)
            return m_hash;

        // Yields the least collisions.
        m_hash = static_cast<FBThash>(reinterpret_cast<FBTuintPtr>(m_key));
        _ftTWHASH(m_hash);
        return m_hash;
    }


    FT_INLINE bool operator==(const ftTHashKey& v) const
    {
        return hash() == v.hash();
    }
    FT_INLINE bool operator!=(const ftTHashKey& v) const
    {
        return hash() != v.hash();
    }
    FT_INLINE bool operator==(const FBThash& v) const
    {
        return hash() == v;
    }
    FT_INLINE bool operator!=(const FBThash& v) const
    {
        return hash() != v;
    }
};



typedef ftTHashKey<void> ftPointerHashKey;


template <typename Key, typename Value>
struct ftHashEntry
{
    Key   first;
    Value second;
    ftHashEntry()
    {
    }
    ftHashEntry(const Key& k, const Value& v) :
        first(k),
        second(v)
    {
    }

    FT_INLINE bool operator==(const ftHashEntry& rhs) const
    {
        return first == rhs.first && second == rhs.second;
    }
};



#define ftCharNEq(a, b, n) ((a && b) && !strncmp(a, b, n))
#define ftCharEq(a, b) ((a && b) && (*a == *b) && !strcmp(a, b))
#define ftCharEqL(a, b, l) ((a && b) && (*a == *b) && !memcmp(a, b, l))
#define ftStrLen(a) ::strlen(a)


#define ftFixedString skFixedString


#endif  //_ftTypes_h_

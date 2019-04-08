/*
-------------------------------------------------------------------------------
    Copyright (c) 2010 Charlie C & Erwin Coumans.

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

#include "string.h"
#include "memory.h"

#if (defined(DEBUG) || defined(_DEBUG)) && ftUSE_DEBUG_ASSERTS == 1
    #include <assert.h> // Keep this the only std include in headers
    #define ftDEBUG 1
    #define ftASSERT(x) assert(x)
#else
    #define ftASSERT(x) ((void)0)
#endif

#ifdef ftUSE_COMPILER_CHECKS
    #define ftASSERTCOMP(x, n) typedef bool x[(n) ? 1 : -1];
#else
    #define ftASSERTCOMP(x, n)
#endif


#define ftPLATFORM_WIN32    0
#define ftPLATFORM_LINUX    2
#define ftPLATFORM_APPLE    3

#if defined (_WIN32)
    #define ftPLATFORM ftPLATFORM_WIN32
#elif defined(__APPLE__)
    #define ftPLATFORM ftPLATFORM_APPLE
#else
    #define ftPLATFORM ftPLATFORM_LINUX
#endif

#define ftCOMPILER_MSVC    0
#define ftCOMPILER_GNU     1

#if defined(_MSC_VER)
    #define ftCOMPILER ftCOMPILER_MSVC
#elif defined(__GNUC__)
    #define ftCOMPILER ftCOMPILER_GNU
#else
    # error unknown compiler
#endif

#define ftENDIAN_LITTLE    0
#define ftENDIAN_BIG       1

#if defined(__sgi)      ||  defined (__sparc)        || \
    defined (__sparc__) ||  defined (__PPC__)        || \
    defined (__ppc__)   ||  defined (__BIG_ENDIAN__)
    #define ftENDIAN ftENDIAN_BIG
#else
    #define ftENDIAN ftENDIAN_LITTLE
#endif

#if ftENDIAN == ftENDIAN_BIG
    #define ftID(a,b,c,d) ( (int)(a)<<24 | (int)(b)<<16 | (c)<<8 | (d) )
    #define ftID2(c, d)   ( (c)<<8 | (d) )
#else
    #define ftID(a,b,c,d) ( (int)(d)<<24 | (int)(c)<<16 | (b)<<8 | (a) )
    #define ftID2(c, d)   ( (d)<<8 | (c) )
#endif

#define ftARCH_32 0
#define ftARCH_64 1

#if defined(__x86_64__)     || defined(_M_X64)      || \
    defined(__powerpc64__)  || defined(__alpha__)   || \
    defined(__ia64__)       || defined(__s390__)    || \
    defined(__s390x__)
    #define ftARCH ftARCH_64
#else
    #define ftARCH ftARCH_32
#endif


#if ftPLATFORM == ftPLATFORM_WIN32
    #if defined(__MINGW32__) || \
        defined(__CYGWIN__)  || \
        (defined (_MSC_VER) && _MSC_VER < 1300)
        #define ftINLINE inline
    #else
        #define ftINLINE __forceinline
    #endif
#else
    #define ftINLINE    inline
#endif

// Integer types
typedef long            FBTlong;
typedef unsigned long   FBTulong;
typedef int             FBTint32;
typedef unsigned int    FBTuint32;
typedef short           FBTint16;
typedef unsigned short  FBTuint16;
typedef signed char     FBTint8;
typedef unsigned char   FBTuint8;
typedef unsigned char   FBTubyte;
typedef signed char     FBTbyte;
typedef bool            FBTint1;


#if ftCOMPILER == ftCOMPILER_GNU
    typedef long long          FBTint64;
    typedef unsigned long long FBTuint64;
#else
    typedef __int64          FBTint64;
    typedef unsigned __int64 FBTuint64;
#endif

// Arch dependent types

#if ftARCH == ftARCH_64
    typedef FBTuint64   FBTuintPtr;
    typedef FBTint64    FBTintPtr;
#else
    typedef FBTuint32   FBTuintPtr;
    typedef FBTint32    FBTintPtr;
#endif

typedef FBTuintPtr  FBTsize;


// Type for arrays & tables (Always unsigned & 32bit)
typedef FBTuint32       FBTsizeType;
const   FBTuint32       ftNPOS = (FBTuint32) - 1;
typedef FBTuint32       FBThash;
typedef FBTuint16       FBTtype;


ftASSERTCOMP(FBTsizeType_NPOS_VALIDATE, ftNPOS == ((FBTuint32) - 1));

#define _ftCACHE_LIMIT 999

template <typename T> ftINLINE void    ftSwap(T& a, T& b)                              { T t(a); a = b; b = t; }
template <typename T> ftINLINE T       ftMax(const T& a, const T& b)                   { return a < b ? b : a; }
template <typename T> ftINLINE T       ftMin(const T& a, const T& b)                   { return a < b ? a : b; }
template <typename T> ftINLINE T       ftClamp(const T& v, const T& a, const T& b)     { return v < a ? a : v > b ? b : v; }


#define ftNameOf(x) #x
#define ftMalloc(size)             ::malloc(size)
#define ftCalloc(size, len)        ::calloc(size, len)
#define ftFree(ptr)                ::free(ptr)
#define ftMemset                   ::memset
#define ftMemcpy                   ::memcpy
#define ftMemmove                  ::memmove
#define ftMemcmp                   ::memcmp



class ftDebugger
{
public:
    typedef void (*ReportHook)(FBTuintPtr client, const char* buffer);

    struct Reporter
    {
        FBTuintPtr m_client;
        ReportHook m_hook;
    };

    static bool isDebugger(void);
    static void reportIDE(const char* src, long line, const char* msg, ...);
    static void errorIDE(const char* src, long line, const char* msg, ...);
    static void report(const char* msg, ...);
    static void breakProcess(void);

    static void setReportHook(Reporter& hook);

private:
    static Reporter m_report;

};

#define ftPrintf ftDebugger::report
#define ftTRACE  ftDebugger::reportIDE
#define ftERROR  ftDebugger::errorIDE

typedef enum ftEndian
{
    ftENDIAN_IS_BIG       = 0,
    ftENDIAN_IS_LITTLE    = 1,
    ftENDIAN_NATIVE,
} ftEndian;

typedef union ftEndianTest
{
    FBTbyte  bo[4];
    FBTint32 test;
} ftEndianTest;



ftINLINE ftEndian ftGetEndian(void)
{
    ftEndianTest e;
    e.test = ftENDIAN_IS_LITTLE;
    return static_cast<ftEndian>(e.bo[0]);
}


ftINLINE bool ftIsEndian(const ftEndian& endian)
{
    ftEndianTest e;
    e.test = endian;
    return static_cast<ftEndian>(e.bo[0]) == endian;
}

ftINLINE FBTuint16 ftSwap16(FBTuint16 in)
{
    return static_cast<FBTuint16>(((in & 0xFF00) >> 8) | ((in & 0x00FF) << 8));
}

ftINLINE FBTuint32 ftSwap32(const FBTuint32& in)
{
    return (((in & 0xFF000000) >> 24) | ((in & 0x00FF0000) >> 8) | ((in & 0x0000FF00) << 8)  | ((in & 0x000000FF) << 24));
}

ftINLINE FBTint16 ftSwap16(FBTint16 in)
{
    return ftSwap16(static_cast<FBTuint16>(in));
}

ftINLINE FBTint32 ftSwap32(const FBTint32& in)
{
    return ftSwap32(static_cast<FBTuint32>(in));
}


ftINLINE FBTuint64 ftSwap64(const FBTuint64& in)
{
    FBTuint64 r = 0;
    const FBTubyte* src = reinterpret_cast<const FBTubyte*>(&in);
    FBTubyte* dst = reinterpret_cast<FBTubyte*>(&r);

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


ftINLINE void ftSwap16(FBTuint16* sp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *sp = ftSwap16(*sp);
        ++sp;
    }
}

ftINLINE void ftSwap32(FBTuint32* ip, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *ip = ftSwap32(*ip);
        ++ip;
    }
}


ftINLINE void ftSwap64(FBTuint64* dp, FBTsize len)
{
    FBTsizeType i;
    for (i = 0; i < len; ++i)
    {
        *dp = ftSwap64(*dp);
        ++dp;
    }
}


ftINLINE FBTint64 ftSwap64(const FBTint64& in)
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
        Link*   next;
        Link*   prev;
    };


    Link*   first;
    Link*   last;

public:

    ftList() : first(0), last(0) {}
    ~ftList() { clear(); }

    void clear(void) { first = last = 0; }

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





template <typename T>
class ftArrayIterator
{
public:
    typedef typename T::Pointer             Iterator;
    typedef typename T::ReferenceType       ValueType;
    typedef typename T::ConstReferenceType  ConstValueType;

protected:

    mutable Iterator        m_iterator;
    mutable FBTsizeType     m_cur;
    mutable FBTsizeType     m_capacity;

public:


    ftArrayIterator() : m_iterator(0), m_cur(0), m_capacity(0) {}
    ftArrayIterator(Iterator begin, FBTsizeType size) : m_iterator(begin), m_cur(0), m_capacity(size) {}
    ftArrayIterator(T& v) : m_iterator(v.ptr()), m_cur(0), m_capacity(v.size()) { }

    ~ftArrayIterator() {}

    ftINLINE bool           hasMoreElements(void) const { return m_iterator && m_cur < m_capacity; }
    ftINLINE ValueType      getNext(void)               { ftASSERT(hasMoreElements()); return m_iterator[m_cur++];  }
    ftINLINE ConstValueType getNext(void) const         { ftASSERT(hasMoreElements()); return m_iterator[m_cur++];  }
    ftINLINE void           next(void) const            { ftASSERT(hasMoreElements()); ++m_cur; }
    ftINLINE ValueType      peekNext(void)              { ftASSERT(hasMoreElements()); return m_iterator[m_cur]; }
    ftINLINE ConstValueType peekNext(void) const        { ftASSERT(hasMoreElements()); return m_iterator[m_cur]; }

};

template <typename T>
class ftArray
{
public:
    typedef T*           Pointer;
    typedef const T*     ConstPointer;

    typedef T            ValueType;
    typedef const T      ConstValueType;

    typedef T&           ReferenceType;
    typedef const T&     ConstReferenceType;

    typedef ftArrayIterator<ftArray<T> >       Iterator;
    typedef const ftArrayIterator<ftArray<T> > ConstIterator;

public:
    ftArray() : m_size(0), m_capacity(0), m_data(0), m_cache(0)  {}

    ftArray(const ftArray<T>& o)
        : m_size(o.size()), m_capacity(0), m_data(0), m_cache(0)
    {
        reserve(m_size);
        copy(m_data, o.m_data, m_size);
    }

    ~ftArray() { clear(); }

    void clear(bool useCache = false)
    {
        if (!useCache)
        {
            if (m_data)
                delete []m_data;
            m_data = 0;
            m_capacity = 0;
            m_size = 0;
            m_cache = 0;
        }
        else
        {
            ++m_cache;
            if (m_cache > _ftCACHE_LIMIT)
                clear(false);
            else m_size = 0;
        }
    }

    FBTsizeType find(const T& v)
    {
        for (FBTsizeType i = 0; i < m_size; i++)
        {
            if (m_data[i] == v)
                return i;
        }
        return ftNPOS;
    }

    ftINLINE void push_back(const T& v)
    {
        if (m_size == m_capacity)
            reserve(m_size == 0 ? 8 : m_size * 2);

        m_data[m_size] = v;
        m_size++;
    }

    ftINLINE void pop_back(void)
    {
        m_size--;
        m_data[m_size].~T();
    }


    void erase(const T& v)
    {
        erase(find(v));
    }

    void erase(FBTsizeType pos)
    {
        if (m_size > 0)
        {
            if (pos != ftNPOS)
            {
                swap(pos, m_size - 1);
                m_size--;
                m_data[m_size].~T();
            }
        }
    }

    void resize(FBTsizeType nr)
    {
        if (nr < m_size)
        {
            for (FBTsizeType i = m_size; i < nr; i++)
                m_data[i].~T();
        }
        else
        {
            if (nr > m_size)
                reserve(nr);
        }
        m_size = nr;
    }

    void resize(FBTsizeType nr, const T& fill)
    {
        if (nr < m_size)
        {
            for (FBTsizeType i = m_size; i < nr; i++)
                m_data[i].~T();
        }
        else
        {
            if (nr > m_size)
                reserve(nr);
            for (FBTsizeType i = m_size; i < nr; i++)
                m_data[i] = fill;

        }
        m_size = nr;
    }

    void reserve(FBTsizeType nr)
    {

        if (m_capacity < nr)
        {
            T* p = new T[nr];
            if (m_data != 0)
            {
                copy(p, m_data, m_size);
                delete []m_data;
            }
            m_data = p;
            m_capacity = nr;
        }
    }

    ftINLINE T& operator[](FBTsizeType idx)               { ftASSERT(idx >= 0 && idx < m_capacity); return m_data[idx]; }
    ftINLINE const T& operator[](FBTsizeType idx) const   { ftASSERT(idx >= 0 && idx < m_capacity); return m_data[idx]; }
    ftINLINE T& at(FBTsizeType idx)                       { ftASSERT(idx >= 0 && idx < m_capacity); return m_data[idx]; }
    ftINLINE const T& at(FBTsizeType idx) const           { ftASSERT(idx >= 0 && idx < m_capacity); return m_data[idx]; }
    ftINLINE T& front(void)                               { ftASSERT(m_size > 0); return m_data[0]; }
    ftINLINE T& back(void)                                { ftASSERT(m_size > 0); return m_data[m_size - 1]; }

    ftINLINE ConstPointer ptr(void) const             { return m_data; }
    ftINLINE Pointer      ptr(void)                   { return m_data; }
    ftINLINE bool         valid(void) const           { return m_data != 0;}

    ftINLINE FBTsizeType capacity(void) const         { return m_capacity; }
    ftINLINE FBTsizeType size(void) const             { return m_size; }
    ftINLINE bool empty(void) const                   { return m_size == 0;}

    ftINLINE Iterator       iterator(void)       { return m_data && m_size > 0 ? Iterator(m_data, m_size) : Iterator(); }
    ftINLINE ConstIterator  iterator(void) const { return m_data && m_size > 0 ? ConstIterator(m_data, m_size) : ConstIterator(); }

    ftArray<T>& operator= (const ftArray<T>& rhs)
    {
        if (this != &rhs)
        {
            clear();
            FBTsizeType os = rhs.size();
            if (os > 0)
            {
                resize(os);
                copy(m_data, rhs.m_data, os);
            }
        }

        return *this;
    }

    ftINLINE void copy(Pointer dst, ConstPointer src, FBTsizeType size)
    {
        ftASSERT(size <= m_size);
        for (FBTsizeType i = 0; i < size; i++) dst[i] = src[i];
    }

    ftINLINE bool equal(const ftArray<T>& rhs)
    {
        if (rhs.size() != size()) return false;
        if (empty()) return true;
        return ftMemcmp(m_data, rhs.m_data, sizeof(T) * m_size) == 0;
    }

protected:

    void swap(FBTsizeType a, FBTsizeType b)
    {
        ValueType t = m_data[a];
        m_data[a] = m_data[b];
        m_data[b] = t;
    }

    FBTsizeType     m_size;
    FBTsizeType     m_capacity;
    Pointer         m_data;
    int             m_cache;
};



template <typename T>
class ftHashTableIterator
{
public:

    typedef typename T::Pointer        Iterator;
    typedef typename T::Entry&          Pair;
    typedef typename T::ConstEntry&     ConstPair;

    typedef typename T::ReferenceKeyType         KeyType;
    typedef typename T::ReferenceValueType       ValueType;
    typedef typename T::ConstReferenceKeyType    ConstKeyType;
    typedef typename T::ConstReferenceValueType  ConstValueType;

protected:

    mutable Iterator m_iterator;
    mutable FBTsizeType   m_cur;
    const FBTsizeType     m_capacity;


public:
    ftHashTableIterator() : m_iterator(0), m_cur(0), m_capacity(0)  {}
    ftHashTableIterator(Iterator begin, FBTsizeType size) : m_iterator(begin), m_cur(0), m_capacity(size) { }
    ftHashTableIterator(T& v) : m_iterator(v.ptr()), m_cur(0), m_capacity(v.size()) {}

    ~ftHashTableIterator() {}

    ftINLINE bool      hasMoreElements(void) const  { return (m_iterator && m_cur < m_capacity); }
    ftINLINE Pair      getNext(void)                { ftASSERT(hasMoreElements()); return m_iterator[m_cur++];}
    ftINLINE ConstPair getNext(void) const          { ftASSERT(hasMoreElements()); return m_iterator[m_cur++];}
    ftINLINE void      next(void) const             { ftASSERT(hasMoreElements()); ++m_cur; }


    ftINLINE Pair      peekNext(void)               { ftASSERT(hasMoreElements()); return m_iterator[m_cur]; }
    ftINLINE KeyType   peekNextKey(void)            { ftASSERT(hasMoreElements()); return m_iterator[m_cur].first;}
    ftINLINE ValueType peekNextValue(void)          { ftASSERT(hasMoreElements()); return m_iterator[m_cur].second; }

    ftINLINE ConstPair      peekNext(void)  const     { ftASSERT(hasMoreElements()); return m_iterator[m_cur]; }
    ftINLINE ConstKeyType   peekNextKey(void) const   { ftASSERT(hasMoreElements()); return m_iterator[m_cur].first;}
    ftINLINE ConstValueType peekNextValue(void) const { ftASSERT(hasMoreElements()); return m_iterator[m_cur].second; }
};



// magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/
#define _ftINITIAL_FNV  0x9E3779B1
#define _ftINITIAL_FNV2 0x9E3779B9
#define _ftMULTIPLE_FNV 0x1000193
#define _ftTWHASH(key)         \
        key += ~(key << 15);    \
        key ^=  (key >> 10);    \
        key +=  (key << 3);     \
        key ^=  (key >> 6);     \
        key += ~(key << 11);    \
        key ^=  (key >> 16);


class ftCharHashKey
{
protected:
    char* m_key;
    mutable FBThash m_hash;

public:
    ftCharHashKey() : m_key(0), m_hash(ftNPOS) {}
    ftCharHashKey(char* k) : m_key(k), m_hash(ftNPOS) {hash();}
    ftCharHashKey(const char* k) : m_key(const_cast<char*>(k)), m_hash(ftNPOS) {}
    ftCharHashKey(const ftCharHashKey& k) : m_key(k.m_key), m_hash(k.m_hash) { if (m_hash == ftNPOS) hash(); }


    FBThash hash(void) const
    {
        if (!m_key) return ftNPOS;
        if (m_hash != ftNPOS) return m_hash;

        // Fowler / Noll / Vo (FNV) Hash
        m_hash = (FBThash)_ftINITIAL_FNV;
        for (int i = 0; m_key[i]; i++)
        {
            m_hash = m_hash ^ (m_key[i]);       // xor  the low 8 bits
            m_hash = m_hash * _ftMULTIPLE_FNV;  // multiply by the magic number
        }
        return m_hash;
    }

    ftINLINE bool operator== (const ftCharHashKey& v) const    {return hash() == v.hash();}
    ftINLINE bool operator!= (const ftCharHashKey& v) const    {return hash() != v.hash();}
    ftINLINE bool operator== (const FBThash& v) const           {return hash() == v;}
    ftINLINE bool operator!= (const FBThash& v) const           {return hash() != v;}
};

class ftIntHashKey
{
protected:
    FBTint32 m_key;
public:
    ftIntHashKey() : m_key(0) {}
    ftIntHashKey(FBTint32 k) : m_key(k) {}
    ftIntHashKey(const ftIntHashKey& k) : m_key(k.m_key) { }

    ftINLINE FBThash hash(void) const  { return static_cast<FBThash>(m_key) * _ftINITIAL_FNV; }

    ftINLINE bool operator== (const ftIntHashKey& v) const {return hash() == v.hash();}
    ftINLINE bool operator!= (const ftIntHashKey& v) const {return hash() != v.hash();}
    ftINLINE bool operator== (const FBThash& v) const       {return hash() == v;}
    ftINLINE bool operator!= (const FBThash& v) const       {return hash() != v;}
};



class ftSizeHashKey
{
protected:

    FBTsize m_key;
    mutable FBThash m_hash;

public:
    ftSizeHashKey()
        : m_hash(ftNPOS), m_key(0)
    {
    }


    ftSizeHashKey(const FBTsize& k)
        :   m_hash(ftNPOS), m_key(k)
    {
        hash();
    }


    ftSizeHashKey(const ftSizeHashKey& k)
        :   m_hash(ftNPOS), m_key(k.m_key)
    {
        hash();
    }


    ftINLINE FBThash hash(void) const
    {
        if (m_hash != ftNPOS)
            return m_hash;
        m_hash = (FBThash)m_key;
        _ftTWHASH(m_hash);
        return m_hash;
    }

    ftINLINE bool operator== (const ftSizeHashKey& v) const { return hash() == v.hash(); }
    ftINLINE bool operator!= (const ftSizeHashKey& v) const { return hash() != v.hash(); }
    ftINLINE bool operator== (const FBThash& v) const       { return hash() == v; }
    ftINLINE bool operator!= (const FBThash& v) const       { return hash() != v; }
};


template<typename T>
class ftTHashKey
{
protected:
    T* m_key;
    mutable FBThash m_hash;

public:
    ftTHashKey() : m_key(0), m_hash(ftNPOS) { hash(); }
    ftTHashKey(T* k) : m_key(k), m_hash(ftNPOS) { hash(); }
    ftTHashKey(const ftTHashKey& k) : m_key(k.m_key), m_hash(k.m_hash) { if (m_hash == ftNPOS) hash(); }

    ftINLINE T*          key(void)       {return m_key;}
    ftINLINE const T*    key(void) const {return m_key;}


    ftINLINE FBThash hash(void) const
    {
        if (m_hash != ftNPOS)
            return m_hash;

        // Yields the least collisions.
        m_hash = static_cast<FBThash>(reinterpret_cast<FBTuintPtr>(m_key));
        _ftTWHASH(m_hash);
        return m_hash;
    }


    ftINLINE bool operator== (const ftTHashKey& v) const    { return hash() == v.hash();}
    ftINLINE bool operator!= (const ftTHashKey& v) const    { return hash() != v.hash();}
    ftINLINE bool operator== (const FBThash& v) const       { return hash() == v;}
    ftINLINE bool operator!= (const FBThash& v) const       { return hash() != v;}
};
typedef ftTHashKey<void> ftPointerHashKey;


template<typename Key, typename Value>
struct ftHashEntry
{
    Key    first;
    Value  second;
    ftHashEntry() {}
    ftHashEntry(const Key& k, const Value& v) : first(k), second(v) {}

    ftINLINE bool operator==(const ftHashEntry& rhs) const
    {
        return first == rhs.first && second == rhs.second;
    }
};

#define _ftUTHASHTABLE_HASH(key)      ((key.hash() & (m_capacity - 1)))
#define _ftUTHASHTABLE_HKHASH(key)    ((hk & (m_capacity - 1)))
#define _ftUTHASHTABLE_FORCE_POW2     1
#define _ftUTHASHTABLE_INIT           32
#define _ftUTHASHTABLE_EXPANSE  (m_size * 2)


#define _ftUTHASHTABLE_STAT       ftHASHTABLE_STAT
#define _ftUTHASHTABLE_STAT_ALLOC 0


#if _ftUTHASHTABLE_FORCE_POW2 == 1

#define _ftUTHASHTABLE_POW2(x) { \
    --x;            \
    x |= x >> 16;   \
    x |= x >> 8;    \
    x |= x >> 4;    \
    x |= x >> 2;    \
    x |= x >> 1;    \
    ++x;}



#define _ftUTHASHTABLE_IS_POW2(x) (!(x & (x-1)))
#endif

#if _ftUTHASHTABLE_STAT == 1
    #include <typeinfo>
#endif



template < typename Key, typename Value >
class ftHashTable
{
public:
    typedef ftHashEntry<Key, Value>        Entry;
    typedef const ftHashEntry<Key, Value>  ConstEntry;

    typedef Entry*  EntryArray;
    typedef FBTsizeType* IndexArray;


    typedef Key            KeyType;
    typedef Value          ValueType;

    typedef const Key      ConstKeyType;
    typedef const Value    ConstValueType;

    typedef Value&          ReferenceValueType;
    typedef const Value&    ConstReferenceValueType;

    typedef Key&            ReferenceKeyType;
    typedef const Key&      ConstReferenceKeyType;

    typedef EntryArray Pointer;
    typedef const Entry* ConstPointer;


    typedef ftHashTableIterator<ftHashTable<Key, Value> > Iterator;
    typedef const ftHashTableIterator<ftHashTable<Key, Value> > ConstIterator;


public:

    ftHashTable()
        :    m_size(0), m_capacity(0), m_lastPos(ftNPOS),
             m_iptr(0), m_nptr(0), m_bptr(0), m_cache(0)
    {
    }

    ftHashTable(FBTsizeType capacity)
        :    m_size(0), m_capacity(0), m_lastPos(ftNPOS),
             m_iptr(0), m_nptr(0), m_bptr(0), m_cache(0)
    {
    }

    ftHashTable(const ftHashTable& rhs)
        :    m_size(0), m_capacity(0), m_lastPos(ftNPOS),
             m_iptr(0), m_nptr(0), m_bptr(0), m_cache(0)
    {
        doCopy(rhs);
    }

    ~ftHashTable() { clear(); }

    ftHashTable<Key, Value>& operator = (const ftHashTable<Key, Value>& rhs)
    {
        if (this != &rhs)
            doCopy(rhs);
        return *this;
    }

    void clear(bool useCache = false)
    {
        if (!useCache)
        {
            m_size = m_capacity = 0;
            m_lastKey = ftNPOS;
            m_lastPos = ftNPOS;
            m_cache = 0;

            delete [] m_bptr;
            delete [] m_iptr;
            delete [] m_nptr;
            m_bptr = 0; m_iptr = 0; m_nptr = 0;
        }
        else
        {
            ++m_cache;
            if (m_cache > _ftCACHE_LIMIT)
                clear(false);
            else
            {
                m_size = 0;
                m_lastKey = ftNPOS;
                m_lastPos = ftNPOS;

                FBTsizeType i;
                for (i = 0; i < m_capacity; ++i)
                {
                    m_iptr[i] = ftNPOS;
                    m_nptr[i] = ftNPOS;
                }
            }
        }

    }
    Value&              at(FBTsizeType i)                    { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].second; }
    Value&              operator [](FBTsizeType i)           { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].second; }
    const Value&        at(FBTsizeType i)const               { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].second; }
    const Value&        operator [](FBTsizeType i) const     { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].second; }
    Key&                keyAt(FBTsizeType i)                 { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].first; }
    const Key&          keyAt(FBTsizeType i)const            { ftASSERT(m_bptr && i >= 0 && i < m_size); return m_bptr[i].first; }

    Value* get(const Key& key) const
    {
        if (!m_bptr || m_size == 0)
            return (Value*)0;

        FBThash hr = key.hash();
        if (m_lastKey != hr)
        {
            FBTsizeType i = find(key);
            if (i == ftNPOS)
                return (Value*)0;

            ftASSERT(i >= 0 && i < m_size);

            m_lastKey = hr;
            m_lastPos = i;
        }
        return &m_bptr[m_lastPos].second;
    }


    Value*         operator [](const Key& key)       { return get(key); }
    const Value*   operator [](const Key& key) const { return get(key); }

    FBTsizeType find(const Key& key) const
    {
        if (m_capacity == 0 || m_capacity == ftNPOS || m_size == 0)
            return ftNPOS;

        FBTsizeType hk = key.hash();
        if (m_lastPos != ftNPOS && m_lastKey == hk)
            return m_lastPos;

        FBThash hr = _ftUTHASHTABLE_HKHASH(hk);

        ftASSERT(m_bptr && m_iptr && m_nptr);

        FBTsizeType fh = m_iptr[hr];
        while (fh != ftNPOS && (key != m_bptr[fh].first))
            fh = m_nptr[fh];


        if (fh != ftNPOS)
        {
            m_lastKey = hk;
            m_lastPos = fh;

            ftASSERT(fh >= 0  && fh < m_size);
        }
        return fh;
    }

    void erase(const Key& key) {remove(key);}

    void remove(const Key& key)
    {
        FBThash hash, lhash;
        FBTsizeType index, pindex, findex;

        findex = find(key);
        if (findex == ftNPOS || m_capacity == 0 || m_size == 0)
            return;

        m_lastKey = ftNPOS;
        m_lastPos = ftNPOS;
        ftASSERT(m_bptr && m_iptr && m_nptr);

        hash = _ftUTHASHTABLE_HASH(key);

        index  = m_iptr[hash];
        pindex = ftNPOS;
        while (index != findex)
        {
            pindex = index;
            index = m_nptr[index];
        }

        if (pindex != ftNPOS)
        {
            ftASSERT(m_nptr[pindex] == findex);
            m_nptr[pindex] = m_nptr[findex];
        }
        else
            m_iptr[hash] = m_nptr[findex];

        FBTsizeType lindex = m_size - 1;
        if (lindex == findex)
        {
            --m_size;
            m_bptr[m_size].~Entry();
            return;
        }

        lhash = _ftUTHASHTABLE_HASH(m_bptr[lindex].first);
        index  = m_iptr[lhash];
        pindex = ftNPOS;
        while (index != lindex)
        {
            pindex = index;
            index = m_nptr[index];
        }

        if (pindex != ftNPOS)
        {
            ftASSERT(m_nptr[pindex] == lindex);
            m_nptr[pindex] = m_nptr[lindex];
        }
        else
            m_iptr[lhash] = m_nptr[lindex];

        m_bptr[findex] = m_bptr[lindex];
        m_nptr[findex] = m_iptr[lhash];
        m_iptr[lhash] = findex;

        --m_size;
        m_bptr[m_size].~Entry();
        return;
    }

    bool insert(const Key& key, const Value& val)
    {
        if (find(key) != ftNPOS)
            return false;

        if (m_size == m_capacity)
            reserve(m_size == 0 ? _ftUTHASHTABLE_INIT : _ftUTHASHTABLE_EXPANSE);

        const FBThash hr = _ftUTHASHTABLE_HASH(key);

        ftASSERT(m_bptr && m_iptr && m_nptr);
        m_bptr[m_size] = Entry(key, val);
        m_nptr[m_size] = m_iptr[hr];
        m_iptr[hr] = m_size;
        ++m_size;
        return true;
    }

    ftINLINE Pointer ptr(void)                { return m_bptr; }
    ftINLINE ConstPointer ptr(void) const     { return m_bptr; }
    ftINLINE bool valid(void) const           { return m_bptr != 0;}


    ftINLINE FBTsizeType size(void) const         { return m_size; }
    ftINLINE FBTsizeType capacity(void) const     { return capacity; }
    ftINLINE bool empty(void) const               { return m_size == 0; }


    Iterator        iterator(void)       { return m_bptr && m_size > 0 ? Iterator(m_bptr, m_size) : Iterator(); }
    ConstIterator   iterator(void) const { return m_bptr && m_size > 0 ? ConstIterator(m_bptr, m_size) : ConstIterator(); }


    void reserve(FBTsizeType nr)
    {
        if (m_capacity < nr && nr != ftNPOS)
            rehash(nr);
    }

#if _ftUTHASHTABLE_STAT == 1

    void report(void) const
    {
        if (m_capacity == 0 || m_capacity == ftNPOS || m_size == 0)
            return;

        ftASSERT(m_bptr && m_iptr && m_nptr);

        FBTsizeType min_col = m_size, max_col = 0;
        FBTsizeType i, tot = 0, avg = 0;
        for (i = 0; i < m_size; ++i)
        {
            Key& key = m_bptr[i].first;

            FBThash hr = _ftUTHASHTABLE_HASH(key);

            FBTsizeType nr = 0;

            FBTsizeType fh = m_iptr[hr];
            while (fh != ftNPOS && (key != m_bptr[fh].first))
            {
                fh = m_nptr[fh];
                nr++;
            }

            if (nr < min_col)
                min_col = nr;
            if (nr > max_col)
                max_col = nr;

            tot += nr;
            avg += nr ? 1 : 0;
        }

#if _ftUTHASHTABLE_FORCE_POW2 == 1
        ftPrintf("Results using forced power of 2 expansion.\n\n");
#else
        ftPrintf("Results using unaltered expansion.\n\n");
#endif
        ftPrintf("\tTotal number of collisions %i for a table of size %i.\n\t\tusing (%s)\n", tot, m_size, typeid(Key).name());
        ftPrintf("\tThe minimum number of collisions per key: %i\n", min_col);
        ftPrintf("\tThe maximum number of collisions per key: %i\n", max_col);

        int favr = (int)(100.f * ((float)avg / (float)m_size));
        ftPrintf("\tThe average number of key collisions: %i\n\n", favr);

        if (tot == 0)
            ftPrintf("\nCongratulations lookup is 100%% linear!\n\n");
        else if (favr >  35)
            ftPrintf("\nImprove your hash function!\n\n");
    }
#endif



private:

    void doCopy(const ftHashTable<Key, Value>& rhs)
    {
        if (rhs.valid() && !rhs.empty())
        {
            reserve(rhs.m_capacity);

            FBTsizeType i, b;
            m_size     = rhs.m_size;
            m_capacity = rhs.m_capacity;

            b = m_size > 0 ? m_size - 1 : 0;
            for (i = b; i < m_capacity; ++i) m_nptr[i] = m_iptr[i] = ftNPOS;

            for (i = 0; i < m_size; ++i)
            {
                m_bptr[i] = rhs.m_bptr[i];
                m_iptr[i] = rhs.m_iptr[i];
                m_nptr[i] = rhs.m_nptr[i];
            }
        }

    }

    template<typename ArrayType>
    void reserveType(ArrayType** old, FBTsizeType nr, bool cpy = false)
    {
        FBTsizeType i;
        ArrayType* nar = new ArrayType[nr];
        if ((*old) != 0)
        {
            if (cpy)
            {
                const ArrayType* oar = (*old);
                for (i = 0; i < m_size; i++) nar[i] = oar[i];
            }
            delete [](*old);
        }
        (*old) = nar;
    }


    void rehash(FBTsizeType nr)
    {
#if _ftUTHASHTABLE_FORCE_POW2

        if (!_ftUTHASHTABLE_IS_POW2(nr))
            _ftUTHASHTABLE_POW2(nr);

#if _ftUTHASHTABLE_STAT_ALLOC == 1
        ftPrintf("Expanding tables: %i\n", nr);
#endif
        ftASSERT(_ftUTHASHTABLE_IS_POW2(nr));


#else

#if _ftUTHASHTABLE_STAT_ALLOC == 1
        ftPrintf("Expanding tables: %i\n", nr);
#endif

#endif

        reserveType<Entry>(&m_bptr, nr, true);
        reserveType<FBTsizeType>(&m_iptr, nr);
        reserveType<FBTsizeType>(&m_nptr, nr);

        m_capacity = nr;
        ftASSERT(m_bptr && m_iptr && m_nptr);


        FBTsizeType i, h;

        for (i = 0; i < m_capacity; ++i)  m_iptr[i] = m_nptr[i] = ftNPOS;
        for (i = 0; i < m_size; i++)     { h = _ftUTHASHTABLE_HASH(m_bptr[i].first); m_nptr[i] = m_iptr[h]; m_iptr[h] = i;}
    }



    FBTsizeType m_size, m_capacity;
    mutable FBTsizeType m_lastPos;
    mutable FBTsizeType m_lastKey;

    IndexArray m_iptr;
    IndexArray m_nptr;
    EntryArray m_bptr;
    FBTsizeType m_cache;
};




#define ftCharNEq(a, b, n)  ((a && b) && !strncmp(a, b, n))
#define ftCharEq(a, b)      ((a && b) && (*a == *b) && !strcmp(a, b))
#define ftCharEqL(a, b, l)  ((a && b) && (*a == *b) && !memcmp(a, b, l))
#define ftStrLen(a)         ::strlen(a)


// For operations on a fixed size character array
template <const FBTuint16 L>
class ftFixedString
{
public:
    typedef char Pointer[L + 1];
public:
    ftFixedString()
        : m_size(0), m_hash(ftNPOS)
    {
        m_buffer[m_size] = 0;
    }

    ftFixedString(const ftFixedString& rhs)
        :   m_size(0), m_hash(ftNPOS)
    {
        if (rhs.size())
        {
            FBTuint16 i, os = rhs.size();
            const char* cp = rhs.c_str();

            for (i = 0; i < L && i < os; ++i, ++m_size) m_buffer[i] = cp[i];
        }
        m_buffer[m_size] = 0;
    }


    ftFixedString(const char* rhs)
        :   m_size(0), m_hash(ftNPOS)
    {
        if (rhs)
        {
            FBTuint16 i;
            for (i = 0; i < L && rhs[i]; ++i, ++m_size) m_buffer[i] = rhs[i];
        }
        m_buffer[m_size] = 0;
    }



    ftINLINE void push_back(char ch)
    {
        if (m_size >= L)
            return;
        m_buffer[m_size++] = ch;
        m_buffer[m_size] = 0;
    }

    void append(const char* str)
    {
        int len = ftStrLen(str);
        int a = 0;
        while (a < len)
            push_back(str[a++]);
    }


    void append(const ftFixedString& str)
    {
        int len = str.m_size;
        int a = 0;
        while (a < len)
            push_back(str.m_buffer[a++]);
    }


    ftFixedString operator +(const ftFixedString& rhs)
    {
        ftFixedString lhs = *this;
        lhs.append(rhs);
        return lhs;
    }


    ftFixedString operator +=(const ftFixedString& rhs)
    {
        append(rhs);
        return *this;
    }

    ftFixedString operator +(const char* str)
    {
        ftFixedString lhs = *this;
        lhs.append(str);
        return lhs;
    }

    ftFixedString operator +=(const char* str)
    {
        append(str);
        return *this;
    }


    void split(ftArray<ftFixedString<L> >& dest, char c, char e = '\0') const
    {
        FBTuint16 i, p = 0, t;
        for (i = 0; i < L && i < m_size; ++i)
        {
            if (m_buffer[i] == c || m_buffer[i] == e)
            {
                ftFixedString<L> cpy;
                for (t = p; t < i; ++t) cpy.push_back(m_buffer[t]);
                dest.push_back(cpy);
                p = i + 1;
            }
        }

        if (p != i)
        {
            ftFixedString<L> cpy;
            for (t = p; t < i; ++t) cpy.push_back(m_buffer[t]);
            dest.push_back(cpy);
        }
    }


    void resize(FBTuint16 ns)
    {
        if (ns <= L)
        {
            if (ns < m_size)
                for (FBTuint16 i = ns; i < m_size; i++) m_buffer[i] = 0;
            else
                for (FBTuint16 i = m_size; i < ns; i++) m_buffer[i] = 0;
            m_size = ns;
            m_buffer[m_size] = 0;
        }
    }



    template<const FBTuint16 OL>
    ftFixedString<L>& operator = (const ftFixedString<OL>& o)
    {
        if (o.m_size > 0)
        {
            if (m_hash == ftNPOS || m_hash != o.m_hash)
            {
                FBTuint16 i;
                m_size = 0;
                m_hash = o.m_hash;
                for (i = 0; (i < L && i < OL) && i < o.m_size; ++i, ++m_size) m_buffer[i] = o.m_buffer[i];
                m_buffer[m_size] = 0;
            }
        }
        return *this;
    }

    ftINLINE const char* c_str(void) const                 { return m_buffer; }
    ftINLINE char* ptr(void)                               { return m_buffer; }
    ftINLINE const char* ptr(void) const                   { return m_buffer; }
    ftINLINE const char operator [](FBTuint16 i) const     { ftASSERT(i < L); return m_buffer[i]; }
    ftINLINE const char at(FBTuint16 i) const              { ftASSERT(i < L); return m_buffer[i]; }
    ftINLINE void clear(void)                              { m_buffer[0] = 0; m_size = 0; }
    ftINLINE int empty(void) const                         { return m_size == 0; }
    ftINLINE int size(void) const                          { return m_size; }
    ftINLINE int capacity(void) const                      { return L; }



    FBTsize hash(void) const
    {
        if (m_hash != ftNPOS)
            return m_hash;

        if (m_size == 0)
            return ftNPOS;
        ftCharHashKey chk(m_buffer);
        m_hash = chk.hash();
        return m_hash;
    }

    ftINLINE bool operator == (const ftFixedString& str) const { return this->hash() == str.hash(); }
    ftINLINE bool operator != (const ftFixedString& str) const { return !(this->operator ==(str));}

protected:

    Pointer             m_buffer;
    FBTuint16           m_size;
    mutable FBThash     m_hash;
};


enum ftPRIM_TYPE
{
    ftPRIM_CHAR,		// 0
    ftPRIM_UCHAR,		// 1
    ftPRIM_SHORT,		// 2
    ftPRIM_USHORT,	    // 3
    ftPRIM_INT,		    // 4
    ftPRIM_LONG,		// 5
    ftPRIM_ULONG,		// 6
    ftPRIM_FLOAT,		// 7
    ftPRIM_DOUBLE,	    // 8
    ftPRIM_VOID,		// 9
    ftPRIM_UNKNOWN	    // 10
};

ftPRIM_TYPE ftGetPrimType(FBTuint32 typeKey);
ftINLINE ftPRIM_TYPE ftGetPrimType(const char* typeName)
{
    return ftGetPrimType(ftCharHashKey(typeName).hash());
}
ftINLINE bool ftIsIntType(FBTuint32 typeKey)
{
    ftPRIM_TYPE tp = ftGetPrimType(typeKey);
    return tp < ftPRIM_FLOAT;
}
ftINLINE bool ftIsFloatType(FBTuint32 typeKey)
{
    ftPRIM_TYPE tp = ftGetPrimType(typeKey);
    return tp == ftPRIM_FLOAT || tp == ftPRIM_DOUBLE;
}
ftINLINE bool ftIsNumberType(FBTuint32 typeKey)
{
    ftPRIM_TYPE tp = ftGetPrimType(typeKey);
    return tp != ftPRIM_VOID && tp != ftPRIM_UNKNOWN;
}

#endif//_ftTypes_h_

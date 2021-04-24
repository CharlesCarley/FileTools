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
#ifndef _ftHashTypes_h_
#define _ftHashTypes_h_

#include "Utils/skHash.h"
#include "ftConfig.h"


/// <summary>
/// Utility class to store a cached char hashed key. 
/// </summary>
class ftCharHashKey
{
protected:
    char*          m_key;
    mutable SKhash m_hash;

public:
    /// <summary>
    /// Default constructor.
    /// </summary>
    ftCharHashKey() :
        m_key(nullptr),
        m_hash(SK_NPOS)
    {
    }

    /// <summary>
    /// Construct from a character pointer
    /// </summary>
    /// <param name="pointer">The character pointer to hash.</param>
    explicit ftCharHashKey(char* pointer) :
        m_key(pointer),
        m_hash(SK_NPOS)
    {
        (void)hash();
    }

    /// <summary>
    /// Construct from a const character pointer
    /// </summary>
    /// <param name="pointer">The character pointer to hash.</param>
    explicit ftCharHashKey(const char* pointer) :
        m_key(const_cast<char*>(pointer)),
        m_hash(SK_NPOS)
    {
        (void)hash();
    }

    /// <summary>
    /// Default copy constructor.
    /// Copies the other class's key and rehashes the internal key. 
    /// </summary>
    /// <param name="other">The character ftCharHashKey to copy.</param>
    ftCharHashKey(const ftCharHashKey& other) :
        m_key(other.m_key),
        m_hash(SK_NPOS)
    {
        (void)hash();
    }

    /// <summary>
    /// Computes the hash of the internal key and remembers it then returns the result.
    /// </summary>
    SKhash hash() const
    {
        if (m_key == nullptr || !*m_key)
            return SK_NPOS;

        // it has already been calculated
        if (m_hash != SK_NPOS)
            return m_hash;

        m_hash = skHash(m_key, FileTools_MaxCharArray);
        return m_hash;
    }

    /// <summary>
    /// Provides access to the internal character pointer that was supplied during class construction.
    /// </summary>
    /// <returns>The internal key.</returns>
    const char* key() const
    {
        return m_key;
    }

    /// <summary>
    /// Tests for equality using the computed hashes. 
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    bool operator==(const ftCharHashKey& rhs) const
    {
        return hash() == rhs.hash();
    }


    /// <summary>
    /// Tests for inequality using the computed hashes.
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    bool operator!=(const ftCharHashKey& rhs) const
    {
        return hash() != rhs.hash();
    }


    /// <summary>
    /// Tests for equality using the computed hash and a key computed elsewhere.
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    bool operator==(const SKhash& rhs) const
    {
        return hash() == rhs;
    }

    /// <summary>
    /// Tests for inequality using the computed hash and a key computed elsewhere.
    /// </summary>
    /// <param name="rhs"></param>
    /// <returns></returns>
    bool operator!=(const SKhash& rhs) const
    {
        return hash() != rhs;
    }
};

class ftPointerHashKey
{
protected:
    void*          m_key;
    mutable SKhash m_hash;

public:
    ftPointerHashKey() :
        m_key(nullptr),
        m_hash(SK_NPOS)
    {
    }

    explicit ftPointerHashKey(void* k) :
        m_key(k),
        m_hash(SK_NPOS)
    {
        (void)hash();
    }

    explicit ftPointerHashKey(SKsize k) :
        m_key((void*)k),
        m_hash(SK_NPOS)
    {
        (void)hash();
    }

    ftPointerHashKey(const ftPointerHashKey& k) :
        m_key(k.m_key),
        m_hash(k.m_hash)
    {
        (void)hash();
    }

    SKhash hash(void) const
    {
        if (m_key == nullptr)
            return SK_NPOS;

        // it has already been calculated
        if (m_hash != SK_NPOS)
            return m_hash;

        m_hash = skHash(m_key);
        return m_hash;
    }

    const void* key() const
    {
        return m_key;
    }

    bool operator==(const ftCharHashKey& v) const
    {
        return hash() == v.hash();
    }

    bool operator!=(const ftCharHashKey& v) const
    {
        return hash() != v.hash();
    }

    bool operator==(const SKhash& v) const
    {
        return hash() == v;
    }

    bool operator!=(const SKhash& v) const
    {
        return hash() != v;
    }
};

extern SKhash skHash(const ftCharHashKey& hk);
extern SKhash skHash(const ftPointerHashKey& hk);

#endif  //_ftHashTypes_h_

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

#include <memory.h>
#include <string.h>
#include "Utils/skHash.h"
#include "ftTypes.h"


class ftCharHashKey
{
protected:
    char*          m_key;
    mutable FBThash m_hash;

public:

    ftCharHashKey() :
        m_key(0),
        m_hash(SK_NPOS)
    {
    }


    ftCharHashKey(char* k) :
        m_key(k),
        m_hash(SK_NPOS)
    {
        hash();
    }


    ftCharHashKey(const char* k) :
        m_key(const_cast<char*>(k)),
        m_hash(SK_NPOS)
    {
        hash();
    }


    ftCharHashKey(const ftCharHashKey& k) :
        m_key(k.m_key),
        m_hash(SK_NPOS)
    {
        hash();
    }


    SKhash hash(void) const
    {
        if (m_key == nullptr || !(*m_key))
            return SK_NPOS;
     
        // it has already been calculated
        if (m_hash != SK_NPOS)
            return m_hash;

        m_hash = skHash(m_key, ::strlen(m_key));
        return m_hash;
    }


    inline bool operator==(const ftCharHashKey& v) const
    {
        return hash() == v.hash();
    }

    inline bool operator!=(const ftCharHashKey& v) const
    {
        return hash() != v.hash();
    }

    inline bool operator==(const SKhash& v) const
    {
        return hash() == v;
    }

    inline bool operator!=(const SKhash& v) const
    {
        return hash() != v;
    }
};


extern FBTuint32 skHash(const ftCharHashKey& hk);


#endif  //_ftHashTypes_h_
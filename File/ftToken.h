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
#ifndef _ftToken_h_
#define _ftToken_h_

#include "ftTypes.h"

struct ftKeywordTable
{
    const char* m_name;
    int         m_len;
    int         m_token;
};

class ftToken
{
public:
    typedef ftFixedString<FT_MAX_ID> String;

private:
    int    m_id;
    String m_value;
    int    m_arrayConstant;

public:
    ftToken();
    ftToken(int id, const String& val);
    ftToken(const ftToken& tok);

    inline int getToken() const
    {
        return m_id;
    }

    inline void setToken(int tok)
    {
        m_id = tok;
    }

    inline const String& getValue() const
    {
        return m_value;
    }

    inline const String& getConstRef() const
    {
        return m_value;
    }

    inline String& getRef()
    {
        return m_value;
    }

    inline int getArrayLen() const
    {
        return m_arrayConstant;
    }

    inline void setArrayLen(int alen)
    {
        m_arrayConstant = alen;
    }
};

#endif  //_ftToken_h_

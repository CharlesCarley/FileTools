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
#ifndef _ftScanner_h_
#define _ftScanner_h_

#include "ftTypes.h"



enum ftTokenID
{

    NULL_TOKEN = -1,

    COMMA    = ',',
    POINTER  = '*',
    LBRACE   = '[',
    RBRACE   = ']',
    LPARN    = '(',
    RPARN    = ')',
    LBRACKET = '{',
    RBRACKET = '}',
    TERM     = ';',

    IDENTIFIER = 256,
    CHAR,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    INT64,
    SCALAR,
    VOID,
    FUNCTION_POINTER_BEG,
    FUNCTION_POINTER_END,
    CONSTANT,

    NAMESPACE,
    CLASS,
    STRUCT,
    UNION,
};


struct ftKeywordTable
{
    char* name;
    int   len;
    int   token;
};


class ftToken
{
public:
    typedef ftFixedString<72> String;

private:
    int    m_id;
    String m_value;
    int    m_arrayConstant;

public:
    ftToken() :
        m_id(NULL_TOKEN),
        m_value(),
        m_arrayConstant(0)
    {
    }

    ftToken(ftTokenID id, const String& val) :
        m_id(id),
        m_value(val),
        m_arrayConstant(0)
    {
    }


    ftToken(const ftToken& tok) :
        m_id(tok.m_id),
        m_value(tok.m_value),
        m_arrayConstant(tok.m_arrayConstant)
    {
    }


    inline int getToken()
    {
        return m_id;
    }

    inline void setToken(int tok)
    {
        m_id = tok;
    }

    inline const String& getValue()
    {
        return m_value;
    }

    inline String& getRef()
    {
        return m_value;
    }



    inline void setArrayLen(int alen)
    {
        m_arrayConstant = alen;
    }

    inline int getArrayLen()
    {
        return m_arrayConstant;
    }
};



class ftScanner
{
private:
    const char* m_buffer;
    int         m_pos;
    int         m_len;
    int         m_state;

    const static ftKeywordTable KeywordTable[];

public:
    ftScanner(const char* ptr, int length) :
        m_buffer(ptr),
        m_pos(0),
        m_len(length),
        m_state(0)
    {
    }

    ftToken lex();


private:
    int  isKeyword(const char* kw, int len, int stateIf);
    void makeIdentifier(ftToken& tok);
    void makeLeftBracket(ftToken& tok);
    void makeRightBracket(ftToken& tok);
    void makeSemicolon(ftToken& tok);

    void makePointer(ftToken& tok);
    void makeLeftBrace(ftToken& tok);
    void makeRightBrace(ftToken& tok);
    void makeLeftParen(ftToken& tok);
    void makeRightParen(ftToken& tok);

    void makeComma(ftToken& tok);
    void makeDigit(ftToken& tok);
};


#endif  // !_ftScanner_h_
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
#include "ftScanner.h"
#include <stdlib.h>

//% s OSTRC ISTRC IGENUM ISENUM ICMT GCMT PSCMT PRIVSEC INSP SSTRC

enum ftLexState
{
    FT_START = 0,

    FT_NAMESPCE,
    FT_CLASS,
    FT_STRUCT,

    FT_INSIDE,
    FT_ISENUM,
    FT_IN_PRIVSEC,
    FT_IN_COMMENT,
};


inline bool isNewLine(const char& ch)
{
    return ch == '\n' || ch == '\r';
}

inline bool isNCS(const char& ch)
{
    return ch == 'n' || ch == 'c' || ch == 's';
}

inline bool isAlpha(const char& ch)
{
    return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z';
}

inline bool isPotentialKeyword(const char& ch)
{
    return ch == 'c' ||
           ch == 'u' ||
           ch == 's' ||
           ch == 'i' ||
           ch == 'l' ||
           ch == 'f' ||
           ch == 'd' ||
           ch == 'v';
}


inline bool isDigit(const char& ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool isAlphaNumeric(const char& ch)
{
    return isAlpha(ch) || isDigit(ch);
}

inline bool isIdentifier(const char& ch)
{
    return isAlphaNumeric(ch) || ch == '_';
}



inline bool isWS(const char& ch)
{
    return ch == ' ' || ch == '\t' || isNewLine(ch);
}

int ftScanner::isKeyword(const char* kw, int len, int stateIf)
{
    const char* tp = &m_buffer[m_pos];
    if (strncmp(tp, kw, len) == 0 && !isIdentifier(m_buffer[m_pos + len]))
    {
        m_state = stateIf;
        return len;
    }
    return 1;
}

const ftKeywordTable ftScanner::KeywordTable[]{
    {"char", 4, CHAR},
    {"uchar", 5, CHAR},
    {"short", 5, SHORT},
    {"ushort", 6, SHORT},
    {"int", 3, INT},
    {"uint", 4, INT},
    {"long", 4, LONG},
    {"ulong", 5, LONG},
    {"float", 5, FLOAT},
    {"double", 6, DOUBLE},
    {"void", 4, VOID},
    {"class", 5, CLASS},
    {"struct", 6, STRUCT},
};



static const ftToken NONE = ftToken(NULL_TOKEN, "@");

ftToken ftScanner::lex()
{
    if (!m_buffer)
        return NONE;

    char    cp = 1;
    ftToken ct;


    while (cp != 0 && m_pos < m_len)
    {
        cp = m_buffer[m_pos];
        if (m_state == FT_START)
        {
            while (!isNCS(m_buffer[m_pos]))
                ++m_pos;

            if (m_buffer[m_pos] == 'n')
            {
                m_pos += isKeyword("namespace", 9, FT_NAMESPCE);
                if (m_state == FT_NAMESPCE)
                {
                    ct.getRef().clear();
                    ct.setToken(NAMESPACE);
                    return ct;
                }
            }
            else if (m_buffer[m_pos] == 'c')
            {
                m_pos += isKeyword("class", 5, FT_CLASS);
                if (m_state == FT_CLASS)
                {
                    ct.getRef().clear();
                    ct.setToken(CLASS);
                    return ct;
                }
            }
            else if (m_buffer[m_pos] == 's')
            {
                m_pos += isKeyword("struct", 6, FT_STRUCT);

                if (m_state == FT_STRUCT)
                {
                    ct.getRef().clear();
                    ct.setToken(STRUCT);
                    return ct;
                }
            }
            else
                m_pos++;
        }
        else if (m_state == FT_NAMESPCE)
        {
            if (!isWS(m_buffer[m_pos]))
            {
                m_pos++;
                m_state = FT_START;
            }
            else
            {
                while (isWS(m_buffer[m_pos]))
                    m_pos++;

                cp = m_buffer[m_pos++];
                if (isAlpha(cp) || cp == '_')
                {
                    makeIdentifier(ct);
                    m_state = FT_START;
                    return ct;
                }
                else
                {
                    ++m_pos;
                    m_state = FT_START;
                }
            }
        }
        else if (m_state == FT_INSIDE)
        {
            while (isWS(m_buffer[m_pos]))
                m_pos++;

            cp = m_buffer[m_pos];
            if (isAlpha(cp) || cp == '_')
            {
                if (isPotentialKeyword(cp))
                {
                    size_t len = sizeof(KeywordTable) / sizeof(KeywordTable[0]), i;

                    for (i = 0; i < len; ++i)
                    {
                        if (isKeyword(KeywordTable[i].name, KeywordTable[i].len, m_state) > 1)
                        {
                            m_pos += KeywordTable[i].len;
                            ct.getRef().clear();
                            ct.getRef() = KeywordTable[i].name;
                            ct.setToken(KeywordTable[i].token);
                            return ct;
                        }
                    }
                }

                makeIdentifier(ct);
                return ct;
            }
            else if (isDigit(cp))
            {
                makeDigit(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == '*')
            {
                makePointer(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == ',')
            {
                makeComma(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == '[')
            {
                makeLeftBrace(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == ']')
            {
                makeRightBrace(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == '(')
            {
                makeLeftParen(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == ')')
            {
                makeRightParen(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == '}')
            {
                makeRightBracket(ct);
                m_state = FT_START;
                return ct;
            }
            else if (m_buffer[m_pos] == ';')
            {
                makeSemicolon(ct);
                return ct;
            }
            else
                ++m_pos;
        }
        else if (m_state == FT_CLASS || m_state == FT_STRUCT)
        {
            while (isWS(m_buffer[m_pos]))
                m_pos++;

            cp = m_buffer[m_pos];
            if (isAlpha(cp) || cp == '_')
            {
                makeIdentifier(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == '{')
            {
                makeLeftBracket(ct);
                m_state = FT_INSIDE;
                return ct;
            }
            else if (m_buffer[m_pos] == '}')
            {
                makeRightBracket(ct);
                return ct;
            }
            else if (m_buffer[m_pos] == ';')
            {
                makeSemicolon(ct);
                m_state = FT_START;
                return ct;
            }
            else
                ++m_pos;
        }

        else
        {
            m_pos++;
        }
    }
    return NONE;
}


void ftScanner::makeIdentifier(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();

    while (isIdentifier(m_buffer[m_pos]))
    {
        ref.push_back(m_buffer[m_pos]);
        m_pos++;
    }

    ct.setToken(IDENTIFIER);
}


void ftScanner::makeLeftBracket(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(LBRACKET);
    m_pos++;
}

void ftScanner::makeRightBracket(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(RBRACKET);
    m_pos++;
}

void ftScanner::makeSemicolon(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(TERM);
    m_pos++;
}

void ftScanner::makePointer(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(POINTER);
    m_pos++;
}

void ftScanner::makeLeftBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(LBRACE);
    m_pos++;
}

void ftScanner::makeRightBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(RBRACE);
    m_pos++;
}

void ftScanner::makeComma(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(COMMA);
    m_pos++;
}

void ftScanner::makeLeftParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(LPARN);
    m_pos++;
}

void ftScanner::makeRightParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ct.setToken(RPARN);
    m_pos++;
}

void ftScanner::makeDigit(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();

    while (isDigit(m_buffer[m_pos]))
    {
        ref.push_back(m_buffer[m_pos]);
        m_pos++;
    }

    ct.setArrayLen(::atoi(ref.c_str()));
    ct.setToken(CONSTANT);
}

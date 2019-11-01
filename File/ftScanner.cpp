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

int ftScanner::lex(ftToken& ct)
{
    if (!m_buffer)
        return NULL_TOKEN;

    char cp = 1;
    while (cp != 0 && m_pos < m_len)
    {
        cp = m_buffer[m_pos];
        if (m_state == FT_START)
        {
            ignoreUntilNCS();

            if (m_buffer[m_pos] == 'n')
            {
                m_pos += isKeyword("namespace", 9, FT_NAMESPCE);
                if (m_state == FT_NAMESPCE)
                {
                    makeKeyword(ct, "namespace", NAMESPACE);
                    return ct.getToken();
                }
            }
            else if (m_buffer[m_pos] == 'c')
            {
                m_pos += isKeyword("class", 5, FT_CLASS);
                if (m_state == FT_CLASS)
                {
                    makeKeyword(ct, "class", CLASS);
                    return ct.getToken();
                }
            }
            else if (m_buffer[m_pos] == 's')
            {
                m_pos += isKeyword("struct", 6, FT_STRUCT);
                if (m_state == FT_STRUCT)
                {
                    makeKeyword(ct, "struct", STRUCT);
                    return ct.getToken();
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
                ignoreWhiteSpace();

                cp = m_buffer[m_pos];
                if (isAlpha(cp) || cp == '_')
                {
                    makeIdentifier(ct);
                    m_state = FT_START;
                    return ct.getToken();
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
            ignoreWhiteSpace();

            cp = m_buffer[m_pos];
            if (isAlpha(cp) || cp == '_')
            {
                if (isPotentialKeyword(cp))
                {
                    size_t i;
                    for (i = 0; i < KeywordTableSize; ++i)
                    {
                        if (KeywordTable[i].m_name[0] == cp &&
                            isKeyword(KeywordTable[i].m_name, KeywordTable[i].m_len, m_state) > 1)
                        {
                            m_pos += KeywordTable[i].m_len;
                            makeKeyword(ct, KeywordTable[i].m_name, KeywordTable[i].m_token);
                            return ct.getToken();
                        }
                    }
                }
                makeIdentifier(ct);
                return ct.getToken();
            }
            else if (isDigit(cp))
            {
                makeDigit(ct);
                return ct.getToken();
            }
            else
            {
                switch (m_buffer[m_pos])
                {
                case '*':
                    makePointer(ct);
                    return ct.getToken();
                case ',':
                    makeComma(ct);
                    return ct.getToken();
                case '[':
                    makeLeftBrace(ct);
                    return ct.getToken();
                case ']':
                    makeRightBrace(ct);
                    return ct.getToken();
                case '(':
                    makeLeftParen(ct);
                    return ct.getToken();
                case ')':
                    makeRightParen(ct);
                    return ct.getToken();
                case '}':
                    makeRightBracket(ct);
                    m_state = FT_START;
                    return ct.getToken();
                case ';':
                    makeSemicolon(ct);
                    return ct.getToken();
                default:
                    ++m_pos;
                    break;
                }
            }
        }
        else if (m_state == FT_CLASS || m_state == FT_STRUCT)
        {
            ignoreWhiteSpace();

            cp = m_buffer[m_pos];
            if (isAlpha(cp) || cp == '_')
            {
                makeIdentifier(ct);
                return ct.getToken();
            }
            else
            {
                switch (m_buffer[m_pos])
                {
                case '{':
                    makeLeftBracket(ct);
                    m_state = FT_INSIDE;
                    return ct.getToken();
                case '}':
                    makeRightBracket(ct);
                    return ct.getToken();
                case ';':
                    makeSemicolon(ct);
                    m_state = FT_START;
                    return ct.getToken();
                default:
                    ++m_pos;
                    break;
                }
            }
        }
        else
            m_pos++;
    }
    return FT_EOF;
}


int ftScanner::newlineTest()
{
    int skp = 0;

    while (m_pos < m_len && isNewLine(m_buffer[m_pos]))
    {
        m_lineNo++;
        if (m_pos + 1 < m_len && isNewLine(m_buffer[m_pos + 1]))
            skp++;
        skp++;
        m_pos += skp;
    }
    return skp;
}


void ftScanner::ignoreUntilNCS()
{
    while (m_pos < m_len && !isNCS(m_buffer[m_pos]))
    {
        if (newlineTest()==0)
            ++m_pos;
    }
}

void ftScanner::ignoreWhiteSpace()
{
    while (m_pos < m_len && isWS(m_buffer[m_pos]))
    {
        if (newlineTest() == 0)
            ++m_pos;
    }
}

void ftScanner::makeIdentifier(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();

    while (m_pos < m_len &&
           isIdentifier(m_buffer[m_pos]))
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
    ref.push_back('{');
    ct.setToken(LBRACKET);
    m_pos++;
}

void ftScanner::makeRightBracket(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('}');
    ct.setToken(RBRACKET);
    m_pos++;
}

void ftScanner::makeSemicolon(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(';');
    ct.setToken(TERM);
    m_pos++;
}

void ftScanner::makePointer(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('*');
    ct.setToken(POINTER);
    m_pos++;
}

void ftScanner::makeLeftBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('[');
    ct.setToken(LBRACE);
    m_pos++;
}

void ftScanner::makeRightBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(']');
    ct.setToken(RBRACE);
    m_pos++;
}

void ftScanner::makeComma(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(',');
    ct.setToken(COMMA);
    m_pos++;
}

void ftScanner::makeLeftParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('(');
    ct.setToken(LPARN);
    m_pos++;
}

void ftScanner::makeRightParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(')');
    ct.setToken(RPARN);
    m_pos++;
}

void ftScanner::makeDigit(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();

    while (m_pos < m_len &&
           isDigit(m_buffer[m_pos]))
    {
        ref.push_back(m_buffer[m_pos]);
        m_pos++;
    }

    ct.setArrayLen(::atoi(ref.c_str()));
    ct.setToken(CONSTANT);
}

void ftScanner::makeKeyword(ftToken& ct, const char* kw, int id)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref = kw;
    ct.setToken(id);
}


const ftKeywordTable ftScanner::KeywordTable[] = {
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

const size_t ftScanner::KeywordTableSize = sizeof(KeywordTable) / sizeof(ftKeywordTable);

bool ftScanner::isNewLine(const char& ch)
{
    return ch == '\n' || ch == '\r';
}

bool ftScanner::isNCS(const char& ch)
{
    return ch == 'n' || ch == 'c' || ch == 's';
}

bool ftScanner::isAlpha(const char& ch)
{
    return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z';
}

bool ftScanner::isPotentialKeyword(const char& ch)
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

bool ftScanner::isDigit(const char& ch)
{
    return ch >= '0' && ch <= '9';
}

bool ftScanner::isAlphaNumeric(const char& ch)
{
    return isAlpha(ch) || isDigit(ch);
}

bool ftScanner::isIdentifier(const char& ch)
{
    return isAlphaNumeric(ch) || ch == '_';
}

bool ftScanner::isWS(const char& ch)
{
    return ch == ' ' || ch == '\t' || isNewLine(ch);
}

int ftScanner::isKeyword(const char* kw, int len, int stateIf)
{
    const char* tp = &m_buffer[m_pos];
    if (m_pos + len < m_len &&
        !isIdentifier(m_buffer[m_pos + len]) &&
        strncmp(tp, kw, len) == 0)
    {
        m_state = stateIf;
        return len;
    }
    return 1;
}

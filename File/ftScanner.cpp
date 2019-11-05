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
    FT_IN_START = 0,
    FT_IN_NAMESPACE,
    FT_IN_CLASS,
    FT_IN_STRUCT,
    FT_INSIDE,
    FT_IN_SKIP,
};


int ftScanner::lex(ftToken& ct)
{
    if (!m_buffer)
        return FT_NULL_TOKEN;

    int decision;
    while (!isEOF())
    {
        switch (m_state)
        {
        case FT_IN_START:
            decision = handleStartState(ct);
            break;
        case FT_IN_NAMESPACE:
            decision = handleNamespaceState(ct);
            break;
        case FT_INSIDE:
            decision = handleInsideState(ct);
            break;
        case FT_IN_CLASS:
        case FT_IN_STRUCT:
            decision = handleClassState(ct);
            break;
        case FT_IN_SKIP:
            decision = handleToggleState(ct);
            break;
        default:
            decision = FT_KEEP_GOING;
            m_pos++;
            break;
        }

        if (decision == FT_EOF)
            return FT_EOF;
        if (decision != FT_KEEP_GOING)
            return ct.getToken();
    }
    return FT_EOF;
} 



int ftScanner::handleStartState(ftToken& ct)
{
    ignoreUntilNCS();
    if (isEOF())
        return FT_EOF;

    if (m_buffer[m_pos] == 'n')
    {
        m_pos += isKeyword("namespace", 9, FT_IN_NAMESPACE);
        if (m_state == FT_IN_NAMESPACE)
        {
            makeKeyword(ct, "namespace", FT_NAMESPACE);
            return ct.getToken();
        }
    }
    else if (m_buffer[m_pos] == 'c')
    {
        m_pos += isKeyword("class", 5, FT_IN_CLASS);
        if (m_state == FT_IN_CLASS)
        {
            makeKeyword(ct, "class", FT_CLASS);
            return ct.getToken();
        }
    }
    else if (m_buffer[m_pos] == 's')
    {
        m_pos += isKeyword("struct", 6, FT_IN_STRUCT);
        if (m_state == FT_IN_STRUCT)
        {
            makeKeyword(ct, "struct", FT_STRUCT);
            return ct.getToken();
        }
    }
    else
        m_pos++;

    return FT_KEEP_GOING;
}


int ftScanner::handleNamespaceState(ftToken& ct)
{
    ignoreWhiteSpace();
    if (isEOF())
        return FT_EOF;

    if (isAlpha(m_buffer[m_pos]) || m_buffer[m_pos] == '_')
    {
        makeIdentifier(ct);
        m_state = FT_IN_START;
        return ct.getToken();
    }
    else
    {
        ++m_pos;
        m_state = FT_IN_START;
    }
    return FT_KEEP_GOING;
}

int ftScanner::handleClassState(ftToken& ct)
{
    ignoreWhiteSpace();
    if (isEOF())
        return FT_EOF;

    if (isAlpha(m_buffer[m_pos]) || m_buffer[m_pos] == '_')
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
            m_state = FT_IN_START;
            return ct.getToken();
        default:
            ++m_pos;
            break;
        }
    }
    return FT_KEEP_GOING;
}

int ftScanner::handleInsideState(ftToken& ct)
{
    ignoreWhiteSpace();
    if (isEOF())
        return FT_EOF;

    char cp = m_buffer[m_pos];
    if (isAlpha(cp) || cp == '_')
    {
        bool keepgoing = false;

        if (isPotentialKeyword(cp))
        {
            size_t i;
            for (i = 0; i < KeywordTableSize && !keepgoing; ++i)
            {
                if (KeywordTable[i].m_name[0] == cp &&
                    isKeyword(KeywordTable[i].m_name, KeywordTable[i].m_len, m_state) > 1)
                {
                    int tz = KeywordTable[i].m_token;
                    if (tz != FT_PUBLIC && tz != FT_PRIVATE && tz != FT_PROTECTED)
                    {
                        m_pos += KeywordTable[i].m_len;
                        makeKeyword(ct, KeywordTable[i].m_name, KeywordTable[i].m_token);
                        return ct.getToken();
                    }
                    else
                    {
                        m_pos += KeywordTable[i].m_len;
                        keepgoing = true;
                    }
                }
            }
        }
        if (!keepgoing)
        {
            makeIdentifier(ct);
            return ct.getToken();
        }
    }
    else if (isDigit(cp))
    {
        makeDigit(ct);
        return ct.getToken();
    }
    else if (cp == '/')
    {
        m_pos++;
        if (isEOF())
            return FT_EOF;

        cp = m_buffer[m_pos];
        if (cp == '/')
        {
            while (m_pos < m_len &&
                   m_buffer[m_pos] != '@' &&
                   !isNewLine(m_buffer[m_pos]))
                m_pos++;

            if (isEOF())
                return FT_EOF;
            else if (!isNewLine(m_buffer[m_pos]))
            {
                m_pos++;
                if (isKeyword("makeft_ignore", 13, FT_IN_SKIP))
                    m_pos += 13;
                else
                {
                    while (m_pos < m_len && !isNewLine(m_buffer[m_pos]))
                        m_pos++;

                    if (isEOF())
                        return FT_EOF;

                    if (isNewLine(m_buffer[m_pos + 1]))
                        m_pos++;
                }
            }
        }
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
        case ':':
            m_pos++;
            break;
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
            m_state = FT_IN_START;
            return ct.getToken();
        case ';':
            makeSemicolon(ct);
            return ct.getToken();
        default:
            ++m_pos;
            break;
        }
    }
    return FT_KEEP_GOING;
}


int ftScanner::handleToggleState(ftToken& ct)
{
    while (m_pos < m_len && m_buffer[m_pos] != '@')
        m_pos++;

    if (isEOF())
        return FT_EOF;

    m_pos++;
    if (isKeyword("makeft_ignore", 13, FT_INSIDE))
        m_pos += 13;

    return FT_KEEP_GOING;
}


bool ftScanner::newlineTest()
{
    bool skp = m_pos < m_len && isNewLine(m_buffer[m_pos]);
    while (m_pos < m_len && isNewLine(m_buffer[m_pos]))
    {
        m_lineNo++;
        if (m_pos + 1 < m_len && isNewLine(m_buffer[m_pos + 1]))
            m_pos++;
        m_pos++;
    }
    return skp;
}


void ftScanner::ignoreUntilNCS()
{
    while (m_pos < m_len && !isNCS(m_buffer[m_pos]))
    {
        if (!newlineTest())
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
    ct.setToken(FT_ID);
}

void ftScanner::makeLeftBracket(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('{');
    ct.setToken(FT_LBRACKET);
    m_pos++;
}

void ftScanner::makeRightBracket(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('}');
    ct.setToken(FT_RBRACKET);
    m_pos++;
}

void ftScanner::makeSemicolon(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(';');
    ct.setToken(FT_TERM);
    m_pos++;
}

void ftScanner::makeColon(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(':');
    ct.setToken(FT_COLON);
    m_pos++;
}

void ftScanner::makePointer(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('*');
    ct.setToken(FT_POINTER);
    m_pos++;
}

void ftScanner::makeLeftBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('[');
    ct.setToken(FT_LBRACE);
    m_pos++;
}

void ftScanner::makeRightBrace(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(']');
    ct.setToken(FT_RBRACE);
    m_pos++;
}

void ftScanner::makeComma(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(',');
    ct.setToken(FT_COMMA);
    m_pos++;
}

void ftScanner::makeLeftParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back('(');
    ct.setToken(FT_LPARAN);
    m_pos++;
}

void ftScanner::makeRightParen(ftToken& ct)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref.push_back(')');
    ct.setToken(FT_RPARN);
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
    ct.setToken(FT_CONSTANT);
}

void ftScanner::makeKeyword(ftToken& ct, const char* kw, int id)
{
    ftToken::String& ref = ct.getRef();
    ref.clear();
    ref = kw;
    ct.setToken(id);
}


const ftKeywordTable ftScanner::KeywordTable[] = {
    {"public", 6, FT_PUBLIC},
    {"private", 7, FT_PRIVATE},
    {"protected", 9, FT_PROTECTED},
    {"struct", 6, FT_STRUCT},
    {"class", 5, FT_CLASS},
    {"char", 4, FT_CHAR},
    {"uchar", 5, FT_CHAR},
    {"short", 5, FT_SHORT},
    {"ushort", 6, FT_SHORT},
    {"int", 3, FT_INTEGER},
    {"uint", 4, FT_INTEGER},
    {"long", 4, FT_LONG},
    {"ulong", 5, FT_LONG},
    {"float", 5, FT_FLOAT},
    {"double", 6, FT_DOUBLE},
    {"void", 4, FT_VOID},
};

const size_t ftScanner::KeywordTableSize = sizeof(KeywordTable) / sizeof(ftKeywordTable);

bool ftScanner::isEOF()
{
    return m_pos >= m_len || m_buffer[m_pos] == '\0';
}


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
           ch == 'p' ||
           ch == 'f' ||
           ch == 'd' ||
           ch == 'v';
}

bool ftScanner::isDigit(const char& ch)
{
    return ch != '\0' && (ch >= '0' && ch <= '9');
}

bool ftScanner::isAlphaNumeric(const char& ch)
{
    return ch != '\0' && (isAlpha(ch) || isDigit(ch));
}

bool ftScanner::isIdentifier(const char& ch)
{
    return isAlphaNumeric(ch) || ch == '_' && ch != '\0';
}

bool ftScanner::isWS(const char& ch)
{
    return ch == ' ' || ch == '\t' || isNewLine(ch);
}

int ftScanner::isKeyword(const char* kw, int len, int stateIf)
{
    const char* tp = &m_buffer[m_pos];
    if (m_pos + (len - 1) < m_len &&
        !isIdentifier(m_buffer[m_pos + len]) &&
        strncmp(tp, kw, len) == 0)
    {
        m_state = stateIf;
        return len;
    }
    return 1;
}

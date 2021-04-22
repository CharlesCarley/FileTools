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
#include "ftToken.h"



class ftScanner
{
private:
    const char* m_buffer;
    int         m_pos;
    SKsize      m_len;
    int         m_state;
    int         m_lineNo;

    static const ftKeywordTable KeywordTable[];
    static const SKsize         KeywordTableSize;

public:
    ftScanner(const char* ptr, SKsize length);

    int lex(ftToken& tok);


    int getLine() const
    {
        return m_lineNo;
    }

private:
    int         isKeyword(const char* kw, int len, int stateIf);
    bool        isEOF() const;
    static bool isNewLine(const char& ch);
    static bool isNCS(const char& ch);
    static bool isAlpha(const char& ch);
    static bool isPotentialKeyword(const char& ch);
    static bool isDigit(const char& ch);
    static bool isAlphaNumeric(const char& ch);
    static bool isIdentifier(const char& ch);
    static bool isWS(const char& ch);

    void ignoreWhiteSpace();
    void ignoreUntilNCS();
    bool newlineTest();

    void makeKeyword(ftToken& tok, const char* kw, int id);
    void makeIdentifier(ftToken& tok);
    void makeLeftBracket(ftToken& tok);
    void makeRightBracket(ftToken& tok);
    void makeColon(ftToken& tok);
    void makeSemicolon(ftToken& tok);
    void makePointer(ftToken& tok);
    void makeLeftBrace(ftToken& tok);
    void makeRightBrace(ftToken& tok);
    void makeLeftParen(ftToken& tok);
    void makeRightParen(ftToken& tok);
    void makeComma(ftToken& tok);
    void makeDigit(ftToken& tok);


    int handleStartState(ftToken& ct);
    int handleNamespaceState(ftToken& ct);
    int handleClassState(ftToken& ct);
    int handleInsideState(ftToken& ct);
    int handleToggleState(ftToken& ct);
    int handleLineComment(ftToken& ct);
};


#endif  // !_ftScanner_h_

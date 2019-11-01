/*
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
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

// Scalar types are defined as [a-z]"Scalar"
#ifdef FBT_SCALAR_DOUBLE
#define scalar_t double
#else
#define scalar_t float
#endif

typedef enum ftTokenID {

    COMMA       = ',',
    POINTER     = '*',
    LBRACE      = '[',
    RBRACE      = ']',
    LPARN       = '(',
    RPARN       = ')',
    LBRACKET    = '{',
    RBRACKET    = '}',
    TERM        = ';',


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

    NULL_TOKEN,
}ftTokenID;

#define ftTOK_IS_TYPE(x) (x >= IDENTIFIER && x <= VOID)
#define ECHO 

int                 gs_line = 1;
ftFixedString<270>  gs_src;

%}
%option noyywrap
%option never-interactive
%s OSTRC ISTRC IGENUM ISENUM ICMT GCMT PSCMT PRIVSEC INSP SSTRC

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
FS          (f|F|l|L)
IS          (u|U|l|L)*
l           [a-z]
WS          [ \t]+
ID          {L}({L}|{D})*
FWDCLS      "class"{WS}*{ID}*{WS}*";"
FWDSTC      "struct"{WS}*{ID}*{WS}*";"
TYPETERM    "}"{WS}*{ID}*{WS}*";"


PRE         "#"{L}[^\n\r]+



%{
FBTsize         gs_lastConstant;


struct ftToken
{
    ftTokenID  m_cur;
    char*       m_buf;
    FBTsize     m_constantSize;
    FBTsize     m_line;
    char*       m_src;
};


%}


%%

<OSTRC>"\n"                 { gs_line ++; }
<OSTRC>"\r\n"               { gs_line ++; }
<OSTRC>"\r"                 { gs_line ++; }
<OSTRC>{FWDCLS}*            { }
<OSTRC>{FWDSTC}*            { }
<OSTRC>"namespace"          { BEGIN INSP;  return (NAMESPACE); }
<OSTRC>"struct"             { BEGIN ISTRC; return STRUCT; }
<OSTRC>"class"              { BEGIN ISTRC; return STRUCT; }
<OSTRC>"enum"               { BEGIN IGENUM; }
<OSTRC>{PRE}*               { }
<OSTRC>"#"                  { BEGIN SSTRC; }
<OSTRC>"/*"                 { BEGIN GCMT; }
<OSTRC>"//"[^\n\r]+         { }
<OSTRC>[.]+                 { }

<INSP>{L}({L}|{D})*         { return IDENTIFIER; }
<INSP>"{"                   { BEGIN OSTRC; }

<ISTRC>"/*"                 { BEGIN ICMT; }
<ISTRC>"//"[^\n\r]+         { }
<ISTRC>"privsec:"           { BEGIN PRIVSEC;}
<ISTRC>"public:"            { }
<ISTRC>"private:"           { }
<ISTRC>"protected:"         { }
<ISTRC>"enum"               { BEGIN ISENUM; }
<ISTRC>"("[ \t\*]+          { return FUNCTION_POINTER_BEG; }
<ISTRC>")"[^;]+             { return FUNCTION_POINTER_END; }
<ISTRC>"char" |
<ISTRC>"uchar"              { return CHAR; }
<ISTRC>"short" |           
<ISTRC>"ushort"             { return SHORT; }
<ISTRC>"int" |
<ISTRC>"uint"               { return INT; }
<ISTRC>"long" |        
<ISTRC>"ulong"              { return LONG;      }
<ISTRC>"int64_t" |
<ISTRC>"uint64_t"           { return INT64;     }
<ISTRC>"float"              { return FLOAT;     }
<ISTRC>"double"             { return DOUBLE;    }
<ISTRC>[a-z]+"Scalar"       { return SCALAR;    }
<ISTRC>"void"               { return VOID;      }
<ISTRC>"struct"             { }
<ISTRC>"class"              { }
<ISTRC>"signed"             { }
<ISTRC>"unsigned"           { }
<ISTRC>"static"             { }
<ISTRC>"typedef"            { }
<ISTRC>"*"                  { return POINTER;       }
<ISTRC>"["                  { return LBRACE;        }
<ISTRC>"]"                  { return RBRACE;        }
<ISTRC>"{"                  { return LBRACKET;      }
<ISTRC>"("                  { return LPARN;         }
<ISTRC>")"                  { return RPARN;         }
<ISTRC>","                  { return COMMA;         }
<ISTRC>";"                  { return TERM;          }
<ISTRC>{L}({L}|{D})*        { return IDENTIFIER;    }
<ISTRC>{TYPETERM}*          { BEGIN OSTRC; return RBRACKET; }
<ISTRC>{D}+{IS}?            { gs_lastConstant = atoi(yytext); return(CONSTANT); }
<ISTRC>"\n"                 { gs_line ++; }
<ISTRC>"\r\n"               { gs_line ++; }
<ISTRC>"\r"                 { gs_line ++; }
<ISTRC>[ \t\v\f]            { }


<ICMT>"\n"                  { gs_line ++; }
<ICMT>"\r\n"                { gs_line ++; }
<ICMT>"\r"                  { gs_line ++; }
<ICMT>"*/"                  { BEGIN ISTRC; }
<ICMT>[.]+                  { }

<GCMT>"\n"                  { gs_line ++; }
<GCMT>"\r\n"                { gs_line ++; }
<GCMT>"\r"                  { gs_line ++; }
<GCMT>"*/"                  { BEGIN OSTRC; }
<GCMT>[.]+                  { }

<PSCMT>"\n"                 { gs_line ++; }
<PSCMT>"\r\n"               { gs_line ++; }
<PSCMT>"\r"                 { gs_line ++; }
<PSCMT>"*/"                 { BEGIN PRIVSEC; }
<PSCMT>[.]+                 { }


<PRIVSEC>"/*"               { BEGIN PSCMT; }
<PRIVSEC>"//"[^\n\r]+       { }
<PRIVSEC>"\n"               { gs_line ++; }
<PRIVSEC>"\r\n"             { gs_line ++; }
<PRIVSEC>"\r"               { gs_line ++; }
<PRIVSEC>"public:"          { BEGIN ISTRC; }
<PRIVSEC>"private:"         { BEGIN ISTRC; }
<PRIVSEC>"protected:"       { BEGIN ISTRC; }
<PRIVSEC>{TYPETERM}*        { BEGIN OSTRC; return RBRACKET; }
<PRIVSEC>[.]+               { }


<SSTRC>"\n"                 { gs_line ++;      }
<SSTRC>"\r\n"               { gs_line ++;      }
<SSTRC>"\r"                 { gs_line ++;      }
<SSTRC>"}"                  { BEGIN OSTRC;  }
<SSTRC>[.]+                 { }


<IGENUM>"\n"                { gs_line ++;      }
<IGENUM>"\r\n"              { gs_line ++;      }
<IGENUM>"\r"                { gs_line ++;      }
<IGENUM>{TYPETERM}*         { BEGIN OSTRC;  }
<IGENUM>[.]+                { }

<ISENUM>"\n"                { gs_line ++;      }
<ISENUM>"\r\n"              { gs_line ++;      }
<ISENUM>"\r"                { gs_line ++;      }
<ISENUM>{TYPETERM}*         { BEGIN ISTRC;  }
<ISENUM>[.]+                { }


%%

ftTokenID ftLex()
{
    return (ftTokenID)ftlex();
}

ftTokenID ftLex(ftToken &tok)
{
    tok.m_cur           = ftLex();
    tok.m_buf           = yytext;
    tok.m_constantSize  = gs_lastConstant;
    tok.m_line          = gs_line;
    tok.m_src           = (char*)gs_src.c_str();
    gs_lastConstant      = -1;
    return tok.m_cur;
}

void *gsCurFile = 0;
yy_buffer_state *gsCurBuf = 0;


void* ftInitParse(const char *ifile)
{
    gs_line  = 1;
    BEGIN   OSTRC;
    gs_src = ifile;


    gsCurFile = (void*)fopen(ifile, "rb");
    if (!gsCurFile)
    {
        printf("File '%s' loading failed!\n", ifile);
        return 0;
    }

    yyin = (FILE*)gsCurFile;
    return gsCurFile;
}


void* ftInitParse(const char *name, const char *buf)
{
    gs_line    = 1;
    BEGIN   OSTRC;
    gs_src = name;

    gsCurBuf = ft_scan_string(buf);
    return gsCurBuf;
}


void ftFreeParse(void *p)
{   
    if (p == gsCurBuf)
    {
        ft_delete_buffer(gsCurBuf);
        return;
    }
    if (p == gsCurFile)
        fclose((FILE*)gsCurFile);
}

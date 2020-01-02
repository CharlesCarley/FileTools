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
#include "ftCompiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Utils/skArray.h"
#include "Utils/skHash.h"
#include "ftAtomic.h"
#include "ftConfig.h"
#include "ftLogger.h"
#include "ftScanner.h"
#include "ftStreams.h"


#define ftValidToken(x) (x > 0)

using namespace ftFlags;


ftCompiler::ftCompiler() :
    m_build(new ftBuildInfo()),
    m_start(0),
    m_curBuf(0),
    m_writeMode(WRITE_ARRAY),
    m_buffer(0),
    m_pos(0),
    m_scanner(0)
{
}

ftCompiler::~ftCompiler()
{
    delete m_build;
}


void ftCompiler::makeName(ftVariable& v, bool forceArray)
{
    ftId newName;
    int  i = 0, j = 0;
    if (v.m_isFunctionPointer)
        newName.push_back('(');

    if (v.m_ptrCount > 0)
    {
        for (i = 0; i < v.m_ptrCount; ++i)
            newName.push_back('*');
    }
    for (i = 0; i < v.m_name.size(); ++i)
        newName.push_back(v.m_name[i]);
    if (v.m_isFunctionPointer)
    {
        newName.push_back(')');
        newName.push_back('(');
        newName.push_back(')');
    }
    if (v.m_arraySize > 1 || forceArray)
    {
        for (i = 0; i < v.m_numDimensions; ++i)
        {
            ftId dest;
            newName.push_back('[');
            sprintf(dest.ptr(), "%i", v.m_arrays[i]);

            char* cp = dest.ptr();
            for (j = 0; cp[j]; ++j)
                newName.push_back(cp[j]);

            newName.push_back(']');
        }
    }
    v.m_name = newName;
}

int ftCompiler::parse(const ftPath& name, const char* data, size_t len)
{
    ftScanner scanner(data, (SKsize)len);

    // Only enable the scanner
    // in the scope of parsing.
    m_scanner = &scanner;
    m_includes.push_back(name.c_str());


    int ret = parse();

    m_scanner = 0;
    return ret;
}


int ftCompiler::parse(const ftPath& id)
{
    skFileStream fp;
    fp.open(id.c_str(), skStream::READ);
    if (!fp.isOpen())
    {
        printf("Error: File loading failed: %s\n", id.c_str());
        return -1;
    }

    SKsize len = fp.size(), br;
    int    rc  = -1;

    if (len == SK_NPOS)
        printf("Error: Failed to determine the length of the file: %s\n", id.c_str());
    else
    {
        char* buffer = new char[len + 1];

        br = fp.read(buffer, len);
        if (br != SK_NPOS)
        {
            buffer[br] = 0;

            rc = parse(id.c_str(), buffer, len + 1);
        }
        else
            printf("Error: Failed to read from the file: %s\n", id.c_str());

        delete[] buffer;
    }
    return rc;
}


int ftCompiler::parse(void)
{
    int     TOK;
    ftToken tp, np;

    do
    {
        TOK = m_scanner->lex(tp);
        if (TOK == FT_NAMESPACE)
        {
            TOK = m_scanner->lex(tp);
            if (TOK == FT_ID)
            {
                if (m_namespaces.find(tp.getValue().c_str()) == m_namespaces.npos)
                    m_namespaces.push_back(tp.getValue().c_str());
            }
        }
        else if (TOK == FT_STRUCT || TOK == FT_CLASS)
            parseClass(TOK, tp);
    } while (ftValidToken(TOK));

    return TOK;
}


void ftCompiler::parseClass(int& TOK, ftToken& tp)
{
    do
    {
        TOK = m_scanner->lex(tp);
        if (TOK == FT_ID)
        {
            ftCompileStruct bs;
            bs.m_name = tp.getValue();

            TOK = m_scanner->lex(tp);
            if (TOK == FT_LBRACKET)
            {
                do
                {
                    TOK = m_scanner->lex(tp);
                    if (TOK == FT_RBRACKET)
                        break;

                    if (TOK == FT_CLASS || TOK == FT_STRUCT)
                        TOK = m_scanner->lex(tp);

                    if (TOK >= FT_ID && TOK <= FT_VOID)
                    {
                        const ftToken::String& typeId = tp.getValue();
                        ftVariable             cur;

                        cur.m_type      = typeId;
                        cur.m_undefined = 0;

                        bool forceArray = false;
                        bool isId       = TOK == FT_ID;
                        do
                        {
                            TOK = m_scanner->lex(tp);
                            switch (TOK)
                            {
                            case FT_RBRACE:
                            case FT_LBRACE:
                                forceArray = true;
                                break;
                            case FT_CONSTANT:
                                if (cur.m_numDimensions + 1 > FT_ARR_DIM_MAX)
                                {
                                    printf("Maximum number of array slots exceeded!\n");
                                    printf("define FT_ARR_DIM_MAX to expand.\nCurrent = [] * %i\n", FT_ARR_DIM_MAX);
                                    TOK = FT_NULL_TOKEN;
                                }
                                cur.m_arrays[cur.m_numDimensions] = tp.getArrayLen();
                                cur.m_numDimensions++;
                                cur.m_arraySize *= tp.getArrayLen();
                                break;
                            case FT_POINTER:
                                cur.m_ptrCount++;
                                break;
                            case FT_ID:
                                cur.m_name = tp.getValue();
                                break;
                            case FT_LPARN:
                                cur.m_isFunctionPointer = 1;
                                cur.m_ptrCount          = 0;
                                cur.m_name              = tp.getValue();
                                break;
                            case FT_RPARN:
                            case FT_PRIVATE:
                            case FT_PUBLIC:
                            case FT_PROTECTED:
                            case FT_COLON:
                                break;
                            case FT_TERM:
                            case FT_COMMA:
                            {
                                makeName(cur, forceArray);
                                if (isId && cur.m_ptrCount == 0)
                                {
                                    if (bs.m_nrDependentTypes > 0)
                                        bs.m_nrDependentTypes = bs.m_nrDependentTypes * 2;
                                    else
                                        bs.m_nrDependentTypes++;
                                    cur.m_isDependentType = true;
                                }
                                bs.m_data.push_back(cur);
                                cur.m_ptrCount  = 0;
                                cur.m_arraySize = 1;
                                if (TOK == FT_COMMA)
                                    cur.m_numDimensions = 0;
                                break;
                            }
                            default:
                                printf("%s(%i): error : Unknown character parsed! %s\n",
                                       m_includes.back().c_str(),
                                       m_scanner->getLine(),
                                       tp.getValue().c_str());
                                TOK = FT_NULL_TOKEN;
                                break;
                            }
                        } while ((TOK != FT_TERM) && ftValidToken(TOK));
                    }
                    else
                    {
                        printf("%s(%i): error : Unknown character parsed! %s\n",
                               m_includes.back().c_str(),
                               m_scanner->getLine(),
                               tp.getValue().c_str());
                        TOK = FT_NULL_TOKEN;
                    }

                } while ((TOK != FT_RBRACKET) && ftValidToken(TOK));

                m_builders.push_back(bs);
            }
        }
    } while ((TOK != FT_RBRACKET && TOK != FT_TERM) && ftValidToken(TOK));
}


FBTuint32 ftCompiler::getNumberOfBuiltinTypes(void)
{
    return m_build->m_numberOfBuiltIn;
}



int ftCompiler::buildTypes(void)
{
    return m_build->getLengths(m_builders);
}

void ftCompiler::writeFile(const ftId& id, skStream* fp)
{
    if (fp)
    {
        fp->writef("const unsigned char %sTable[]={\n", id.c_str());
        m_writeMode = WRITE_ARRAY;
        writeStream(fp);
        fp->writef("\n};\n");
        fp->writef("const int %sLen=sizeof(%sTable);\n", id.c_str(), id.c_str());
    }
    else
    {
        ftLogger::logF("Invalid write stream");
    }
}



void ftCompiler::writeFile(const ftId& id, const ftPath& path)
{
    skFileStream fp;
    fp.open(path.c_str(), skStream::WRITE);
    if (!fp.isOpen())
    {
        printf("Failed to open data file: %s\n", path.c_str());
        return;
    }

    fp.writef("const unsigned char %sTable[]={\n", id.c_str());

    m_writeMode = WRITE_ARRAY;
    writeStream(&fp);

    fp.writef("\n};\n");
    fp.writef("const int %sLen=sizeof(%sTable);\n", id.c_str(), id.c_str());


#if FT_TYLE_LEN_VALIDATE == 1
    writeValidationProgram(path.c_str());
#endif
}

void ftCompiler::writeStream(skStream* fp)
{
    int i;
    m_curBuf = -1;


    writeBinPtr(fp, (void*)&ftIdNames::FT_SDNA[0], 4);
    writeBinPtr(fp, (void*)&ftIdNames::FT_NAME[0], 4);
    i = m_build->m_name.size();


#if ftFAKE_ENDIAN == 1
    i = ftSwap32(i);
#endif

    writeBinPtr(fp, &i, 4);
    writeCharPtr(fp, m_build->m_name);



    writeBinPtr(fp, (void*)&ftIdNames::FT_TYPE[0], 4);
    i = m_build->m_typeLookup.size();
#if ftFAKE_ENDIAN == 1
    i = ftSwap32(i);
#endif
    writeBinPtr(fp, &i, 4);
    writeCharPtr(fp, m_build->m_typeLookup);
    writeBinPtr(fp, (void*)&ftIdNames::FT_TLEN[0], 4);

#if ftFAKE_ENDIAN == 1
    for (i = 0; i < (int)m_build->m_tlen.size(); i++)
        m_build->m_tlen.at(i) = ftSwap16(m_build->m_tlen.at(i));
#endif
    writeBinPtr(fp, m_build->m_tlen.ptr(), m_build->m_alloc.m_tlen);
    if (m_build->m_tlen.size() & 1)
    {
        char pad[2] = {'@', '@'};
        writeBinPtr(fp, (void*)&pad[0], 2);
    }

    writeBinPtr(fp, (void*)&ftIdNames::FT_STRC[0], 4);
    i = m_builders.size();

#if ftFAKE_ENDIAN == 1
    i = ftSwap32(i);
#endif
    writeBinPtr(fp, &i, 4);

#if ftFAKE_ENDIAN == 1
    for (i = 0; i < (int)m_build->m_strc.size(); i++)
        m_build->m_strc.at(i) = ftSwap16(m_build->m_strc.at(i));
#endif
    writeBinPtr(fp, m_build->m_strc.ptr(), m_build->m_alloc.m_strc);
}



void ftCompiler::writeCharPtr(skStream* fp, const ftStringPtrArray& ptrs)
{
    char    pad[4] = {'b', 'y', 't', 'e'};
    SKuint32 i = 0, s = ptrs.size();
    SKuint32 t = 0;

    while (i < s)
    {
        ftId id = ptrs[i++];
        id.push_back('\0');
        writeBinPtr(fp, (void*)id.c_str(), id.size());
        t += id.size();
    }

    int len = t;

    len = (len + 3) & ~3;
    if (len - t)
    {
        ftId id;
        SKuint32 p;
        for (p = 0; p < (len - t); p++)
            id.push_back(pad[p % 4]);

        writeBinPtr(fp, (void*)id.c_str(), id.size());
    }
}

void ftCompiler::writeBinPtr(skStream* fp, void* ptr, int len)
{
    if (m_writeMode == WRITE_ARRAY)
    {
        int            i;
        unsigned char* cb = (unsigned char*)ptr;

        for (i = 0; i < len; ++i, ++m_curBuf)
        {
            if ((m_curBuf % 16) == 15)
                fp->write("\n", 1);
            fp->writef("0x%02X,", (unsigned char)cb[i]);
        }
    }
    else
        fp->write(ptr, len);
}



void ftCompiler::writeValidationProgram(const ftPath& path)
{
#if FT_TYLE_LEN_VALIDATE == 1

    ftPath      string;
    ftPathArray split;
    path.split(split, '/', '\\');

    int i;
    int last = 0;
    for (i = path.size() - 1; i > 0; --i)
    {
        if (path[i] == '.')
        {
            last = i;
            break;
        }
    }

    for (i = 0; i < path.size(); ++i)
    {
        if (i >= last)
            break;

        string.push_back(path[i]);
    }
    string += "Validator.cpp";

    FILE* fp = fopen(string.c_str(), "wb");
    if (!fp)
    {
        printf("Failed to open validation file %s\n", string.c_str());
        return;
    }

    for (i = 0; i < (int)m_includes.size(); ++i)
    {
        split.clear();
        m_includes[i].split(split, '/', '\\');

        fprintf(fp, "#include \"%s\"\n", m_includes[i].c_str());
    }

    fprintf(fp, "#include <stdlib.h>\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    if (!m_namespaces.empty())
    {
        for (i = 0; i < (int)m_namespaces.size(); ++i)
            fprintf(fp, "using namespace %s;\n\n\n", m_namespaces[i].c_str());
    }

    fprintf(fp, "int main()\n{\n\tint errors=0;\n");

    ftCompileStruct::Array::Iterator it = m_builders.iterator();
    while (it.hasMoreElements())
    {
        ftCompileStruct& bs = it.getNext();

        ftId&   cur = m_build->m_typeLookup.at((SKuint32)bs.m_structId);
        FBTtype len = m_build->m_tlen.at((SKuint32)bs.m_structId);

        if (m_skip.find(cur) != m_skip.npos)
            continue;
#if ftFAKE_ENDIAN == 1
        len = ftSwap16(len);
#endif
        fprintf(fp, "\t");
        fprintf(fp, "if (sizeof(%s) != %i)\n\t{\n\t\terrors ++;\n", cur.c_str(), len);
        fprintf(fp, "#ifdef _MSC_VER\n");
        fprintf(fp,
                "\t\tfprintf(stderr, \"%%s(%%i): error : Validation failed with "
                "( %%i = sizeof(%s) ) != %%i\\n\", __FILE__, __LINE__, (int)sizeof(%s), %i);\n",
                cur.c_str(),
                cur.c_str(),
                len);
        fprintf(fp, "#else\n");
        fprintf(fp,
                "\t\tfprintf(stderr, \"%%s:%%i: error : Validation failed with "
                "( %%i = sizeof(%s) ) != %%i\\n\", __FILE__, __LINE__, (int)sizeof(%s), %i);\n",
                cur.c_str(),
                cur.c_str(),
                len);
        fprintf(fp, "#endif\n");
        fprintf(fp, "\t}\n");
        fprintf(fp, "\n");
    }

    fprintf(fp, "\t");
    fprintf(fp,
            "if (errors > 0)fprintf(stderr, \"%%s(%%i): error : "
            "there are %%i misaligned types.\\n\", __FILE__, __LINE__, errors);\n");

    fprintf(fp, "\treturn errors == 0 ? 0 : 1;\n}\n");

#endif
    fclose(fp);
}

/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

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
#include <cstdio>
#include "Utils/skArray.h"
#include "ftConfig.h"
#include "ftLogger.h"
#include "ftScanner.h"
#include "ftStreams.h"

#define FT_IS_VALID_TOKEN(x) ((x) > 0)

using namespace ftFlags;

ftCompiler::ftCompiler() :
    m_build(new ftTableBuilder()),
    m_curBuf(0),
    m_writeMode(WRITE_ARRAY),
    m_scanner(nullptr)
{
}

ftCompiler::~ftCompiler()
{
    delete m_build;
}

void ftCompiler::makeName(ftBuildMember& v, bool forceArray)
{
    ftId     newName;
    SKuint16 i;
    if (v.isFunctionPointer)
        newName.push_back('(');

    if (v.ptrCount > 0)
    {
        for (i = 0; i < v.ptrCount; ++i)
            newName.push_back('*');
    }

    for (i = 0; i < v.name.size(); ++i)
        newName.push_back(v.name[i]);

    if (v.isFunctionPointer)
    {
        newName.push_back(')');
        newName.push_back('(');
        newName.push_back(')');
    }
    if (v.arraySize > 1 || forceArray)
    {
        if (v.numDimensions > FileTools_MaxArrayDim)
            printf("The number of array dimensions exceeded. Max FT_ARR_DIM_MAX(%i) needed (%i)\n", FileTools_MaxArrayDim, v.numDimensions);
        else
        {
            for (i = 0; i < v.numDimensions; ++i)
            {
                ftId dest;
                newName.push_back('[');
                skSprintf(dest.ptr(), ftId::capacity() - 1, "%i", v.arrays[i]);

                char* cp = dest.ptr();
                for (int j = 0; cp[j]; ++j)
                    newName.push_back(cp[j]);

                newName.push_back(']');
            }
        }
    }
    v.name = newName;
}

int ftCompiler::parse(const char* path, const char* data, SKsize len)
{
    if (!path)
        return -1;

    ftScanner scanner(data, (SKsize)len);

    // Only enable the scanner
    // in the scope of parsing.
    m_scanner = &scanner;
    m_includes.push_back(path);

    const int ret = parse();

    m_scanner = nullptr;
    return ret;
}

int ftCompiler::parse(const char* path)
{
    skFileStream fp;
    fp.open(path, skStream::READ);
    if (!fp.isOpen())
    {
        printf("Error: File loading failed: %s\n", path);
        return -1;
    }

    const SKsize len = fp.size();
    int          rc  = -1;

    if (len == SK_NPOS)
        printf("Error: Failed to determine the length of the file: %s\n", path);
    else
    {
        char* buffer = new char[len + 1];

        const SKsize br = fp.read(buffer, len);
        if (br != SK_NPOS)
        {
            buffer[br] = 0;

            rc = parse(path, buffer, len + 1);
        }
        else
            printf("Error: Failed to read from the file: %s\n", path);

        delete[] buffer;
    }
    return rc;
}

int ftCompiler::parse()
{
    int     token;
    ftToken tokenPtr;

    do
    {
        token = m_scanner->lex(tokenPtr);
        if (token == FT_NAMESPACE)
        {
            token = m_scanner->lex(tokenPtr);
            if (token == FT_ID)
            {
                if (m_namespaces.find(tokenPtr.getValue().c_str()) == m_namespaces.npos)
                    m_namespaces.push_back(tokenPtr.getValue().c_str());
            }
        }
        else if (token == FT_STRUCT || token == FT_CLASS)
            parseClass(token, tokenPtr);

    } while (FT_IS_VALID_TOKEN(token));

    return token;
}

void ftCompiler::parseClass(int& token, ftToken& tokenPtr)
{
    do
    {
        token = m_scanner->lex(tokenPtr);
        if (token == FT_ID)
        {
            ftBuildStruct bs;
            bs.name = tokenPtr.getValue();

            token = m_scanner->lex(tokenPtr);
            if (token == FT_L_BRACKET)
            {
                do
                {
                    token = m_scanner->lex(tokenPtr);
                    if (token != FT_R_BRACKET)
                    {
                        if (token == FT_CLASS || token == FT_STRUCT)
                            token = m_scanner->lex(tokenPtr);

                        if (token >= FT_ID && token <= FT_VOID)
                            parseIdentifier(token, tokenPtr, bs);
                        else
                            errorUnknown(token, tokenPtr);
                    }
                } while (token != FT_R_BRACKET &&
                         FT_IS_VALID_TOKEN(token));

                m_builders.push_back(bs);
            }
        }

    } while ((token != FT_R_BRACKET && token != FT_TERM) &&
             FT_IS_VALID_TOKEN(token));
}

void ftCompiler::parseIdentifier(int& token, ftToken& tokenPtr, ftBuildStruct& buildStruct)
{
    bool forceArray = false;

    const ftToken::String& typeId = tokenPtr.getValue();

    ftBuildMember cur;
    cur.type      = typeId;
    cur.undefined = 0;

    const bool isId = token == FT_ID;

    do
    {
        token = m_scanner->lex(tokenPtr);
        switch (token)
        {
        case FT_RBRACE:
        case FT_L_BRACE:
            forceArray = true;
            break;
        case FT_CONSTANT:
            handleConstant(token, tokenPtr, cur);
            break;
        case FT_POINTER:
            cur.ptrCount++;
            break;
        case FT_ID:
            cur.name = tokenPtr.getValue();
            break;
        case FT_L_PARENTHESIS:
            cur.name              = tokenPtr.getValue();
            cur.isFunctionPointer = 1;
            cur.ptrCount          = 0;
            break;
        case FT_R_PARENTHESIS:
        case FT_PRIVATE:
        case FT_PUBLIC:
        case FT_PROTECTED:
        case FT_COLON:
            break;
        case FT_TERM:
        case FT_COMMA:
            handleStatementClosure(token,
                                   buildStruct,
                                   cur,
                                   forceArray,
                                   isId);
            break;
        default:
            errorUnknown(token, tokenPtr);
            break;
        }

    } while (token != FT_TERM && FT_IS_VALID_TOKEN(token));
}

void ftCompiler::handleConstant(int& token, ftToken& tokenPtr, ftBuildMember& member)
{
    if (member.numDimensions + 1 > FileTools_MaxArrayDim)
    {
        printf("Maximum number of array slots exceeded!\n");
        printf("define FT_ARR_DIM_MAX to expand.\nCurrent = [] * %i\n", FileTools_MaxArrayDim);
        token = FT_NULL_TOKEN;
    }
    else
    {
        member.arrays[member.numDimensions] = tokenPtr.getArrayLen();
        member.numDimensions++;
        member.arraySize *= tokenPtr.getArrayLen();
    }
}

void ftCompiler::handleStatementClosure(int&           token,
                                        ftBuildStruct& buildStruct,
                                        ftBuildMember& member,
                                        bool           forceArray,
                                        bool           isIdentifier)
{
    makeName(member, forceArray);

    if (isIdentifier && member.ptrCount == 0)
    {
        if (buildStruct.nrDependentTypes > 0)
            buildStruct.nrDependentTypes = buildStruct.nrDependentTypes * 2;
        else
            buildStruct.nrDependentTypes++;

        // Flag it as dependent
        member.isDependentType = true;
    }

    buildStruct.data.push_back(member);

    // reset it for the next iteration
    member.ptrCount  = 0;
    member.arraySize = 1;
    if (token == FT_COMMA)
        member.numDimensions = 0;
}

void ftCompiler::errorUnknown(int& token, ftToken& tokenPtr)
{
    printf("%s(%i): error : Unknown character parsed! '%s'\n",
           m_includes.back().c_str(),
           m_scanner->getLine(),
           tokenPtr.getValue().c_str());
    token = FT_NULL_TOKEN;
}

int ftCompiler::compile()
{
    return m_build->getLengths(m_builders);
}

void ftCompiler::writeFile(const ftId& tableName, skStream* fp)
{
    if (fp)
    {
        fp->writef("const unsigned char %sTable[]={\n", tableName.c_str());
        m_writeMode = WRITE_ARRAY;
        writeStream(fp);
        fp->writef("\n};\n");
        fp->writef("const int %sLen=sizeof(%sTable);\n", tableName.c_str(), tableName.c_str());
    }
    else
    {
        ftLogger::logF("Invalid write stream");
    }
}

void ftCompiler::writeFile(const ftId& tableName, const ftPath& path)
{
    skFileStream fp;
    fp.open(path.c_str(), skStream::WRITE);
    if (!fp.isOpen())
    {
        printf("Failed to open data file: %s\n", path.c_str());
        return;
    }

    fp.writef("const unsigned char %sTable[]={\n", tableName.c_str());

    m_writeMode = WRITE_ARRAY;
    writeStream(&fp);

    fp.writef("\n};\n");
    fp.writef("const int %sLen=sizeof(%sTable);\n", tableName.c_str(), tableName.c_str());

#if FileTools_TypeLengthValidate == 1
    writeValidationProgram(path.c_str());
#endif
}

void ftCompiler::writeStream(skStream* fp)
{
    int i;
    m_curBuf = -1;

    writeBinPtr(fp, (void*)&ftIdNames::FT_SDNA[0], 4);
    writeBinPtr(fp, (void*)&ftIdNames::FT_NAME[0], 4);
    i = m_build->name.size();

#ifdef FileTools_SwapEndian
    i = ftSwap32(i);
#endif

    writeBinPtr(fp, &i, 4);
    writeCharPtr(fp, m_build->name);

    writeBinPtr(fp, (void*)&ftIdNames::FT_TYPE[0], 4);
    i = m_build->typeLookup.size();

#ifdef FileTools_SwapEndian
    i = ftSwap32(i);
#endif

    writeBinPtr(fp, &i, 4);
    writeCharPtr(fp, m_build->typeLookup);
    writeBinPtr(fp, (void*)&ftIdNames::FT_TLEN[0], 4);

#ifdef FileTools_SwapEndian
    for (i = 0; i < (int)m_build->m_tlen.size(); i++)
        m_build->m_tlen.at(i) = ftSwap16(m_build->m_tlen.at(i));
#endif

    writeBinPtr(fp, m_build->typeLengths.ptr(), m_build->allocationSizes.lengths);
    if (m_build->typeLengths.size() & 1)
    {
        char pad[2] = {'@', '@'};
        writeBinPtr(fp, (void*)&pad[0], 2);
    }

    writeBinPtr(fp, (void*)&ftIdNames::FT_STRC[0], 4);
    i = m_builders.size();

#ifdef FileTools_SwapEndian
    i = ftSwap32(i);
#endif
    writeBinPtr(fp, &i, 4);

#ifdef FileTools_SwapEndian
    for (i = 0; i < (int)m_build->m_strc.size(); i++)
        m_build->m_strc.at(i) = ftSwap16(m_build->m_strc.at(i));
#endif

    writeBinPtr(fp, m_build->structures.ptr(), m_build->allocationSizes.structures);
}

void ftCompiler::writeCharPtr(skStream* fp, const ftStringPtrArray& pointers)
{
    char           pad[4] = {'b', 'y', 't', 'e'};
    SKuint32       i      = 0;
    const SKuint32 s      = pointers.size();
    SKuint32       t      = 0;

    while (i < s)
    {
        ftId id = pointers[i++];
        id.push_back('\0');
        writeBinPtr(fp, (void*)id.c_str(), id.size());
        t += id.size();
    }

    int len = t;

    len = len + 3 & ~3;
    if (len - t)
    {
        ftId id;
        for (SKuint32 p = 0; p < (len - t); p++)
            id.push_back(pad[p % 4]);

        writeBinPtr(fp, (void*)id.c_str(), id.size());
    }
}

void ftCompiler::writeBinPtr(skStream* fp, void* ptr, int len)
{
    if (m_writeMode == WRITE_ARRAY)
    {
        unsigned char* cb = (unsigned char*)ptr;

        for (int i = 0; i < len; ++i, ++m_curBuf)
        {
            if (m_curBuf % 16 == 15)
                fp->write("\n", 1);
            fp->writef("0x%02X,", (unsigned char)cb[i]);
        }
    }
    else
        fp->write(ptr, len);
}

#define ToString(x) #x

void ftCompiler::writeValidationProgram(const ftPath& path)
{
#if FileTools_TypeLengthValidate == 1

    ftPath      string;
    ftPathArray split;
    path.split(split, '/', '\\');

    int last = 0;
    for (SKuint16 i = path.size() - 1; i > 0; --i)
    {
        if (path[i] == '.')
        {
            last = i;
            break;
        }
    }

    for (SKuint16 i = 0; i < path.size(); ++i)
    {
        if (i >= last)
            break;

        string.push_back(path[i]);
    }

    string += "Validator.cpp";

    skFileStream fp;

    fp.open(string.c_str(), skStream::WRITE);
    if (!fp.isOpen())
    {
        printf("Failed to open validation file %s\n", string.c_str());
        return;
    }

    for (const ftPath& include : m_includes)
    {
        split.clear();
        include.split(split, '/', '\\');

        fp.writef("#include \"%s\"\n", include.c_str());
    }

    fp.writef("#include <cstdlib>\n");
    fp.writef("#include <cstdio>\n\n");
    fp.writef("#define ToString(x) #x\n");
    fp.writef("#define AssertFailed(typeName, expected, actual) ");
    const char* extra = ToString(
        fprintf(stderr,
                "%s(%i): error : Validation failed with ( %i = sizeof(%s) ) != %i\n",
                __FILE__,
                __LINE__,
                actual,
                typeName,
                expected););
    fp.writef("%s\n", extra);

    if (!m_namespaces.empty())
    {
        for (SKuint32 i = 0; i < m_namespaces.size(); ++i)
            fp.writef("using namespace %s;\n\n\n", m_namespaces[i].c_str());
    }

    fp.writef("int main()\n{\n\tint errors=0;\n");
    ftBuildStruct::Array::Iterator it = m_builders.iterator();
    while (it.hasMoreElements())
    {
        ftBuildStruct& bs = it.getNext();

        ftId&        cur = m_build->typeLookup.at((SKuint32)bs.structureId);
        const FTtype len = m_build->typeLengths.at((SKuint32)bs.structureId);

        if (m_skip.find(cur) != m_skip.npos)
            continue;

#ifdef FileTools_SwapEndian
        len = ftSwap16(len);
#endif

        fp.writef("\tif (sizeof(%s) != %i)\n\t{\n\t\terrors ++;\n", cur.c_str(), len);
        fp.writef("\t\t");
        fp.writef("AssertFailed(ToString(%s), %i, (int)sizeof(%s));\n", cur.c_str(), len, cur.c_str());
        fp.writef("\t}\n\n");
    }

    fp.writef("\t");
    fp.writef(
        "if (errors > 0) fprintf(stderr, \"%%s(%%i): error : "
        "there are %%i misaligned types.\\n\", __FILE__, __LINE__, errors);\n");

    fp.writef("\treturn errors == 0 ? 0 : 1;\n}\n");

#endif
}

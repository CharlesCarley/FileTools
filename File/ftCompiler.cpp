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
//#include "Generated/ftScanner.inl"
#include "ftConfig.h"
#include "ftScanner.h"
#include "ftStreams.h"

#define ftValidToken(x) (x > 0)


typedef ftArray<FBTtype> IntPtrArray;
typedef ftArray<FBTtype> TypeArray;

struct MaxAllocSize
{
    FBTuint32 m_name;
    FBTuint32 m_type;
    FBTuint32 m_tlen;
    FBTuint32 m_strc;
    FBTuint32 m_offs;
};

class ftBuildInfo
{
public:
    ftBuildInfo();
    ~ftBuildInfo()
    {
    }

    void        reserve(void);
    int         getLengths(ftCompileStruct::Array& struct_builders);
    int         getTLengths(ftCompileStruct::Array& struct_builders);
    void        makeBuiltinTypes(void);
    bool        hasType(const ftId& type);
    FBTsizeType addType(const ftId& type, const FBTuint32& len);
    FBTsizeType addName(const ftId& name);

    MaxAllocSize     m_alloc;
    ftStringPtrArray m_name;
    ftStringPtrArray m_typeLookup;
    IntPtrArray      m_tlen;
    IntPtrArray      m_64ln;
    TypeArray        m_strc;
    ftStringPtrArray m_undef;
};



ftCompiler::ftCompiler() :
    m_build(new ftBuildInfo()),
    m_start(0),
    m_curBuf(0),
    m_writeMode(0),
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
    if (v.m_isFptr)
        newName.push_back('(');

    if (v.m_ptrCount > 0)
    {
        for (i = 0; i < v.m_ptrCount; ++i)
            newName.push_back('*');
    }
    for (i = 0; i < v.m_name.size(); ++i)
        newName.push_back(v.m_name[i]);
    if (v.m_isFptr)
    {
        newName.push_back(')');
        newName.push_back('(');
        newName.push_back(')');
    }
    if (v.m_arraySize > 1 || forceArray)
    {
        for (i = 0; i < v.m_numSlots; ++i)
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

int ftCompiler::parseBuffer(const ftId& name, const char* ms, int len)
{
    ftScanner scanner(ms, len);
    m_scanner = &scanner;
    m_includes.push_back(name.c_str());


    int ret   = doParse();
    m_scanner = 0;
    return ret;
}


int ftCompiler::parseFile(const ftPath& id)
{
    FILE* fp = fopen(id.c_str(), "rb");
    if (!fp)
    {
        printf("Error: File loading failed: %s\n", id.c_str());
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_t len = ftell(fp), br;
    fseek(fp, 0L, SEEK_SET);

    char* buffer = new char[len + 1];
    br           = fread(buffer, 1, len, fp);
    buffer[br]   = 0;
    fclose(fp);

    int ret = parseBuffer(id.c_str(), buffer, len + 1);

    delete[] buffer;
    return ret;
}


int ftCompiler::doParse(void)
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
                if (m_namespaces.find(tp.getValue().c_str()) == ftNPOS)
                    m_namespaces.push_back(tp.getValue().c_str());
            }
        }
        else if (TOK == FT_STRUCT || TOK == FT_CLASS)
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

                                        if (cur.m_numSlots + 1 > FT_ARR_DIM_MAX)
                                        {
                                            printf("Maximum number of array slots exceeded!\n");
                                            printf("define FT_ARR_DIM_MAX to expand.\nCurrent = [] * %i\n", FT_ARR_DIM_MAX);
                                            return -1;
                                        }
                                        cur.m_arrays[cur.m_numSlots] = tp.getArrayLen();
                                        cur.m_numSlots++;
                                        cur.m_arraySize *= tp.getArrayLen();
                                        break;
                                    case FT_POINTER:
                                        cur.m_ptrCount++;
                                        break;
                                    case FT_ID:
                                        cur.m_name = tp.getValue();
                                        break;
                                    case FT_LPARAN:
                                        cur.m_isFptr = 1;
                                        cur.m_ptrCount++;
                                        cur.m_name = tp.getValue();
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
                                            cur.m_numSlots = 0;
                                    }
                                    break;
                                    default:
                                    {
                                        printf("%s(%i): error : Unknown character parsed! %s\n",
                                               m_includes.back().c_str(),
                                               m_scanner->getLine(),
                                               tp.getValue().c_str());
                                        return -1;
                                    }
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
                                return -1;
                            }

                        } while ((TOK != FT_RBRACKET) && ftValidToken(TOK));

                        m_builders.push_back(bs);
                    }
                }
            } while ((TOK != FT_RBRACKET && TOK != FT_TERM) && ftValidToken(TOK));
        }
    } while (ftValidToken(TOK));

    return 0;
}

int ftCompiler::buildTypes(void)
{
    return m_build->getLengths(m_builders);
}

void ftCompiler::writeFile(const ftId& id, ftStream* fp)
{
    if (!fp)
        return;

    fp->writef("const unsigned char %sTable[]={\n", id.c_str());
    m_writeMode = 0;
    writeStream(fp);
    fp->writef("\n};\n");
    fp->writef("const int %sLen=sizeof(%sTable);\n", id.c_str(), id.c_str());
}



void ftCompiler::writeFile(const ftId& id, const ftPath& path)
{
    ftFileStream fp;
    fp.open(path.c_str(), ftStream::SM_WRITE);
    if (!fp.isOpen())
    {
        printf("Failed to open data file: %s\n", path.c_str());
        return;
    }

    fp.writef("const unsigned char %sTable[]={\n", id.c_str());

    m_writeMode = 0;
    writeStream(&fp);

    fp.writef("\n};\n");
    fp.writef("const int %sLen=sizeof(%sTable);\n", id.c_str(), id.c_str());


#if FT_TYLE_LEN_VALIDATE == 1
    writeValidationProgram(path.c_str());
#endif
}

void ftCompiler::writeStream(ftStream* fp)
{
    m_curBuf = -1;
    int i;
    writeBinPtr(fp, (void*)&ftIdNames::ftSDNA[0], 4);
    writeBinPtr(fp, (void*)&ftIdNames::ftNAME[0], 4);
    i = m_build->m_name.size();
#if ftFAKE_ENDIAN == 1
    i = ftSwap32(i);
#endif
    writeBinPtr(fp, &i, 4);
    writeCharPtr(fp, m_build->m_name);

    writeBinPtr(fp, (void*)&ftIdNames::ftTYPE[0], 4);
    i = m_build->m_typeLookup.size();
#if ftFAKE_ENDIAN == 1
    i = ftSwap32(i);
#endif
    writeBinPtr(fp, &i, 4);

    writeCharPtr(fp, m_build->m_typeLookup);
    writeBinPtr(fp, (void*)&ftIdNames::ftTLEN[0], 4);
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

    writeBinPtr(fp, (void*)&ftIdNames::ftSTRC[0], 4);
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



void ftCompiler::writeCharPtr(ftStream* fp, const ftStringPtrArray& ptrs)
{
    char    pad[4] = {'b', 'y', 't', 'e'};
    FBTsize i = 0, s = ptrs.size();
    int     t = 0;

    while (i < s)
    {
        ftId id = ptrs[i++];
        id.push_back('\0');
        writeBinPtr(fp, (void*)id.c_str(), id.size());
        t += id.size();
    }

    int len = t;
    len     = (len + 3) & ~3;
    if (len - t)
    {
        ftId id;
        int  p;
        for (p = 0; p < (len - t); p++)
            id.push_back(pad[p % 4]);
        writeBinPtr(fp, (void*)id.c_str(), id.size());
    }
}

void ftCompiler::writeBinPtr(ftStream* fp, void* ptr, int len)
{
    if (m_writeMode == 0)
    {
        unsigned char* cb = (unsigned char*)ptr;
        for (int i = 0; i < len; ++i, ++m_curBuf)
        {
            if ((m_curBuf % 18) == (17))
                fp->writef("\n");

            unsigned char cp = cb[i];
            fp->writef("0x%02X,", cp);
        }
    }
    else
        fp->write(ptr, len);
}

ftBinTables* ftCompiler::write(void)
{
    ftMemoryStream ms;
    ms.open(ftStream::SM_WRITE);
    m_writeMode = -1;

    writeStream(&ms);

    void* buffer = ::malloc(ms.size() + 1);
    if (buffer != 0)
        ::memcpy(buffer, ms.ptr(), ms.size());
    return new ftBinTables(buffer, ms.size());
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
        split.clear(true);
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

        ftId&   cur = m_build->m_typeLookup.at(bs.m_structId);
        FBTtype len = m_build->m_tlen.at(bs.m_structId);

        if (m_skip.find(cur) != ftNPOS)
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


ftBuildInfo::ftBuildInfo()
{
    m_alloc.m_name = 0;
    m_alloc.m_type = 0;
    m_alloc.m_tlen = 0;
    m_alloc.m_strc = 0;
    m_alloc.m_offs = 0;
}


void ftBuildInfo::reserve(void)
{
    m_name.reserve(FT_MAX_TABLE);
    m_typeLookup.reserve(FT_MAX_TABLE);
    m_tlen.reserve(FT_MAX_TABLE);
    m_strc.reserve(FT_MAX_TABLE * FT_MAX_MEMBERS);
}


void ftBuildInfo::makeBuiltinTypes(void)
{
    addType("char", sizeof(char));
    addType("uchar", sizeof(char));
    addType("short", sizeof(short));
    addType("ushort", sizeof(short));
    addType("int", sizeof(int));
    addType("long", sizeof(long));
    addType("ulong", sizeof(long));
    addType("float", sizeof(float));
    addType("double", sizeof(double));
    addType("int64_t", sizeof(FBTint64));
    addType("uint64_t", sizeof(FBTuint64));
#ifdef ftSCALAR_DOUBLE
    addType("scalar_t", sizeof(double));
#else
    addType("scalar_t", sizeof(float));
#endif
    addType("void", 0);
}

FBTsizeType ftBuildInfo::addType(const ftId& type, const FBTuint32& len)
{
    FBTsize loc = m_typeLookup.find(type);
    if (loc == ftNPOS)
    {
        m_alloc.m_type += type.size() + 1;
        m_alloc.m_tlen += sizeof(FBTtype);
        loc = m_typeLookup.size();

        m_typeLookup.push_back(type);
        m_tlen.push_back(len);
        m_64ln.push_back(len);
    }
    return loc;
}

bool ftBuildInfo::hasType(const ftId& type)
{
    return m_typeLookup.find(type) != ftNPOS;
}

FBTsizeType ftBuildInfo::addName(const ftId& name)
{
    FBTsize loc;
    if ((loc = m_name.find(name)) == ftNPOS)
    {
        m_alloc.m_name += name.size() + 1;
        loc = m_name.size();
        m_name.push_back(name);
    }
    return loc;
}

int ftBuildInfo::getLengths(ftCompileStruct::Array& struct_builders)
{
    makeBuiltinTypes();

    ftCompileStruct::Array::Iterator bit = struct_builders.iterator();
    while (bit.hasMoreElements())
    {
        ftCompileStruct& bs = bit.getNext();
        bs.m_structId       = addType(bs.m_name, 0);

        m_strc.push_back(bs.m_structId);
        m_strc.push_back((FBTint16)bs.m_data.size());

        m_alloc.m_strc += (sizeof(FBTtype) * 2);

        ftVariables::Iterator it = bs.m_data.iterator();
        while (it.hasMoreElements())
        {
            ftVariable& cvar = it.getNext();

            cvar.m_typeId = addType(cvar.m_type, 0);
            cvar.m_nameId = addName(cvar.m_name);

            m_strc.push_back(cvar.m_typeId);
            m_strc.push_back(cvar.m_nameId);

            m_alloc.m_strc += (sizeof(FBTtype) * 2);
        }
    }

    return getTLengths(struct_builders);
}


int ftBuildInfo::getTLengths(ftCompileStruct::Array& struct_builders)
{
    ftCompileStruct* strcs = struct_builders.ptr();
    FBTsize          tot   = struct_builders.size(), i, e;

    int next = tot, prev = 0;

    FBTtype*         tln64  = m_64ln.ptr();
    FBTtype*         tlens  = m_tlen.ptr();
    FBTsize          nrel   = 0, ct, len, fake64;
    int              status = LNK_OK;
    ftStringPtrArray m_missingReport, m_zpdef;

    FBTtype ft_start = 0;
    if (strcs)
        ft_start = (FBTtype)strcs[0].m_structId;

    ftVariable* vptr = 0;
    while (next != prev && strcs)
    {
        prev = next;
        next = 0;

        for (i = 0; i < tot; ++i)
        {
            ftCompileStruct& cur = strcs[i];
            if (tlens[cur.m_structId] != 0)
            {
                FBTsizeType pos;
                if ((pos = m_missingReport.find(cur.m_name)) != ftNPOS)
                    m_missingReport.erase(pos);
            }
            else
            {
                vptr = cur.m_data.ptr();
                nrel = cur.m_data.size();

                len         = 0;
                fake64      = 0;
                bool hasPtr = false;
                for (e = 0; e < nrel; ++e)
                {
                    ftVariable& v = vptr[e];
                    ct            = v.m_typeId;

                    if (v.m_ptrCount > 0)
                    {
                        hasPtr = true;
                        if (len % ftVOID)
                        {
                            printf(v.m_path.c_str(),
                                   v.m_line,
                                   "align %i: %s %s add %i bytes\n",
                                   ftVOID,
                                   v.m_type.c_str(),
                                   v.m_name.c_str(),
                                   ftVOID - (len % ftVOID));

                            status |= LNK_ALIGNEMENTP;
                        }
                        if (fake64 % 8)
                        {
                            printf(v.m_path.c_str(),
                                   v.m_line,
                                   "align %i: %s %s add %i bytes\n",
                                   8,
                                   v.m_type.c_str(),
                                   v.m_name.c_str(),
                                   8 - (fake64 % 8));

                            status |= LNK_ALIGNEMENTP;
                        }
                        len += ftVOID * v.m_arraySize;
                        fake64 += 8 * v.m_arraySize;
                    }
                    else if (tlens[ct])
                    {
                        if (ct >= ft_start)
                        {
                            if (ftVOID8 && (len % 8))
                            {
                                printf(v.m_path.c_str(),
                                       v.m_line,
                                       "align: %i alignment error add %i bytes\n",
                                       8,
                                       8 - (len % 8));
                                status |= LNK_ALIGNEMENTS;
                            }
                        }

                        if (tlens[ct] > 3 && (len % 4))
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   4,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   4 - (len % 4));
                            status |= LNK_ALIGNEMENT4;
                        }
                        else if (tlens[ct] == 2 && (len % 2))
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   2,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   2 - (len % 2));
                            status |= LNK_ALIGNEMENT2;
                        }

                        len += tlens[ct] * v.m_arraySize;
                        fake64 += tln64[ct] * v.m_arraySize;
                    }
                    else
                    {
                        next++;
                        len = 0;
                        if (m_missingReport.find(cur.m_name) == ftNPOS)
                            m_missingReport.push_back(cur.m_name);

                        tln64[cur.m_structId] = 0;
                        tlens[cur.m_structId] = 0;
                        break;
                    }
                }

                tln64[cur.m_structId] = fake64;
                tlens[cur.m_structId] = len;

                if (len != 0)
                {
                    if (hasPtr || fake64 != len)
                    {
                        if (fake64 % 8)
                        {
                            printf(cur.m_path.c_str(),
                                   cur.m_line,
                                   "64Bit alignment, in %s add %i bytes\n",
                                   cur.m_name.c_str(),
                                   8 - (fake64 % 8));
                            status |= LNK_ALIGNEMENT8;
                        }
                    }

                    if (len % 4)
                    {
                        printf(cur.m_path.c_str(),
                               cur.m_line,
                               "align 4: in %s add %i bytes\n",
                               cur.m_name.c_str(),
                               4 - (len % 4));
                        status |= LNK_ALIGNEMENT4;
                    }
                }
            }
        }
    }


    if (!m_missingReport.empty())
    {
        status |= LNK_UNDEFINED_TYPES;

        ftStringPtrArray::Iterator it = m_missingReport.iterator();
        while (it.hasMoreElements())
        {
            ftId& id = it.getNext();
            printf("Link error: undefined reference to type '%s'\n", id.c_str());
        }
    }

    if (FT_DEBUG >= 3)
    {
        ftCompileStruct::Array::Iterator bit = struct_builders.iterator();

        while (bit.hasMoreElements())
        {
            ftCompileStruct& bs = bit.getNext();

            printf(bs.m_path.c_str(), bs.m_line, "typeid (%s):%i\n", bs.m_name.c_str(), bs.m_structId);
            if (FT_DEBUG > 0 && !bs.m_data.empty())
            {
                ftVariables::Iterator it = bs.m_data.iterator();
                while (it.hasMoreElements())
                {
                    ftVariable& cvar = it.getNext();
                    printf(cvar.m_path.c_str(), cvar.m_line, "typeid:%-8inameid:%-8isizeof:%-8i%s %s\n", 
                        cvar.m_typeId, cvar.m_nameId, 
                        (cvar.m_ptrCount > 0 ? ftVOID : tlens[cvar.m_typeId]) * cvar.m_arraySize, 
                        cvar.m_type.c_str(), cvar.m_name.c_str());
                }
            }
        }
    }
    return status;
}

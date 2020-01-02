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
#define FT_IN_SOURCE_FILE
#include <fstream>
#include <iomanip>
#include <iostream>
//#include "Blender.h"
//#include "SID.h"
#include "Utils/skList.h"
#include "Utils/skStack.h"
#include "Utils/skString.h"
#include "Utils/skTimer.h"
#include "ftCompiler.h"
#include "ftMember.h"
#include "ftPlatformHeaders.h"
#include "ftScanDNA.h"


typedef skArray<ftStruct*> StructArray;
typedef skList<ftStruct*>  StructDeque;

using namespace std;


struct ProgramInfo
{
    const char* m_progName;
    skString    m_ifile;
    skString    m_ofile;
    skStream*   m_stream;
    void*       m_dnaBlock;
    ftTables*   m_tables;
    char        m_header[13];
    bool        m_useNamespace;
    bool        m_fixupBlend;
    bool        m_initTypes;
    bool        m_writeHashCode;

    // extra vars only valid in extract to file
    string m_outName;
};

void writeFileHeader(ProgramInfo& ctx, ostream& out)
{
    char     buf[32];
    SKuint32 br = skGetTimeString(buf, 31, "%a %b %d %r");

    out << "/*" << endl
        << endl;
    out << "    This file was automatically generated." << endl;
    out << "    https://github.com/CharlesCarley/FileTools" << endl;
    out << endl;
    out << "    By    : " << ctx.m_progName << endl;
    out << "    From  : " << ctx.m_ifile.c_str() << "(" << ctx.m_header << ")" << endl;
    if (br != SK_NPOS32)
        out << "    On    : " << buf << endl
            << endl;
    out << "*/" << endl;

    ctx.m_outName = ctx.m_ofile.substr(0, ctx.m_ofile.find('.')).c_str();
    out << "#ifndef _" << ctx.m_outName << "_h_" << endl;
    out << "#define _" << ctx.m_outName << "_h_" << endl;
    out << endl;

    if (ctx.m_initTypes)
    {
        out << "#include <stdint.h>" << endl;
        out << endl;
    }

    if (ctx.m_fixupBlend)
    {
        out << "#ifdef near" << endl;
        out << "#undef near" << endl;
        out << "#endif//near" << endl;

        out << "#ifdef far" << endl;
        out << "#undef far" << endl;
        out << "#endif//far" << endl;
        out << endl;
    }
}


void writeFileFooter(ProgramInfo& ctx, ostream& out)
{
    out << "#endif//_" << ctx.m_outName << "_h_" << endl;
}

void writeIndent(ostream& out, int nr, int spacePerIndent = 4)
{
    spacePerIndent = skMax(spacePerIndent, 0);
    int i;
    for (i = 0; i < nr; ++i)
        out << setw(spacePerIndent) << ' ';
}

void writeForward(ProgramInfo& ctx, ostream& out, ftStruct* ftStrc)
{
    out << "struct " << ftStrc->getName() << ';' << endl;
}

void writeStructure(ProgramInfo& ctx, ostream& out, ftStruct* fstrc)
{

    writeIndent(out, 1);
    out << "struct " << fstrc->getName() << endl;
    writeIndent(out, 1);
    out << "{" << endl;
    FBTtype* strc = ctx.m_tables->getStructAt(fstrc->getStructIndex());

    int maxLeft = -1;
    if (strc)
    {
        FBTtype cnt = strc[1], i;
        strc += 2;
        for (i = 0; i < cnt; ++i, strc += 2)
        {
            const ftType& type = ctx.m_tables->getTypeAt(strc[0]);

            maxLeft = skMax(maxLeft, (int)strlen(type.m_name));
        }
    }

    strc = ctx.m_tables->getStructAt(fstrc->getStructIndex());
    if (strc)
    {
        FBTtype cnt = strc[1], i;
        strc += 2;
        for (i = 0; i < cnt; ++i, strc += 2)
        {
            const ftType& type = ctx.m_tables->getTypeAt(strc[0]);
            const ftName& name = ctx.m_tables->getNameAt(strc[1]);
            if (ctx.m_fixupBlend)
            {
                if (string(type.m_name) == "anim")
                    out << left << setw(maxLeft) << "Anim" << ' ' << name.m_name << ';' << endl;
                else
                    out << left << setw(maxLeft) << type.m_name << ' ' << name.m_name << ';' << endl;
            }
            else
                out << left << setw(maxLeft) << type.m_name << ' ' << name.m_name << ';' << endl;
        }
    }
    out << "};" << endl;
    out << endl;
}



void writeHashCodes(ProgramInfo& ctx, ostream& out, const StructArray& strcs)
{
    out << "namespace StructCodes" << endl;
    out << "{" << endl;
    out << "        typedef unsigned long long HashCode;" << endl;
    out << endl;

    StructArray::ConstIterator cit = strcs.iterator();
    while (cit.hasMoreElements())
    {
        const ftStruct* strc = cit.getNext();
        out << setw(ctx.m_useNamespace ? 8 : 4) << ' ';
        out << "const HashCode SDNA_" << uppercase << strc->getName() << "= 0x" << hex << strc->getHashedType() << ';' << endl;
    }
    out << "}" << endl;
    out << endl;
    out << endl;
}


void writeUnresolved(ProgramInfo& ctx, ostream& out, FBTtype* typeNotFound)
{
    char*         typeName = ctx.m_tables->getTypeNameAt(typeNotFound[0]);
    const ftName& name     = ctx.m_tables->getNameAt(typeNotFound[1]);

    if (name.m_ptrCount > 0)
    {
        if (ctx.m_fixupBlend && string(typeName) == "anim")
        {
            out << "struct Anim" << endl;
            out << '{' << endl;
            out << "    int missing;" << endl;
            out << "};" << endl;
            out << endl;
        }
        else
        {
            out << "struct " << typeName << endl;
            out << '{' << endl;
            out << "    int missing;" << endl;
            out << "};" << endl;
            out << endl;
        }
    }
}

bool hasUnResolved(skArray<FBTtype*>& unresolved, FBTtype* inp)
{
    bool found = false;

    skArray<FBTtype*>::Iterator it = unresolved.iterator();
    while (!found && it.hasMoreElements())
    {
        FBTtype* type = it.getNext();

        found = type[0] == inp[0];
    }
    return found;
}

bool structContains(ftStruct* a, ftStruct* b, StructArray& searchList, StructArray& indp)
{
    ftTables* tables = a->getParent();
    bool      result = false;
    if (tables)
    {
        FBTtype* strc = tables->getStructAt(a->getStructIndex());
        if (strc)
        {
            FBTtype cnt = strc[1], i;
            strc += 2;
            for (i = 0; i < cnt && !result; ++i, strc += 2)
            {
                const ftType& type = tables->getTypeAt(strc[0]);
                const ftName& name = tables->getNameAt(strc[1]);

                if (type.m_strcId != SK_NPOS32)
                {
                    if (name.m_ptrCount <= 0)
                    {
                        ftStruct* fstrc = tables->findStructByName(type.m_name);

                        if (fstrc && fstrc == b)
                        {
                            if (indp.find(b) == indp.npos)
                            {
                                if (searchList.find(fstrc) != searchList.npos)
                                {
                                    if (fstrc->getHashedType() == b->getHashedType())
                                        result = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

void findMemberStructs(ftStruct* fstrc, StructArray& searchList, StructArray& indp, StructArray& out)
{
    StructArray::Iterator it = searchList.iterator();
    while (it.hasMoreElements())
    {
        ftStruct* tstrc = it.getNext();
        if (tstrc != fstrc)
        {
            if (structContains(fstrc, tstrc, searchList, indp))
                out.push_back(tstrc);
        }
    }
}

void organizeDependentStructs(StructArray& dependent, StructArray& independent)
{
    StructDeque structs, toplevel;

    if (!dependent.empty())
    {
        bool needs_resort = true;

        StructArray::Iterator it = dependent.iterator();

        while (it.hasMoreElements())
        {
            ftStruct*   fstrc = it.getNext();
            StructArray deps;

            findMemberStructs(fstrc, dependent, independent, deps);

            if (deps.empty())
            {
                fstrc->lock();
                toplevel.push_front(fstrc);
            }
            else
                structs.push_back(fstrc);
        }

        it = dependent.iterator();
        while (it.hasMoreElements())
        {
            ftStruct* fstrc = it.getNext();

            StructArray deps;
            findMemberStructs(fstrc, dependent, independent, deps);

            if (!deps.empty())
            {
                StructDeque::Link* link = structs.find(fstrc);
                if (link)
                {
                    StructArray           erase;
                    StructArray::Iterator sit = deps.iterator();
                    while (sit.hasMoreElements())
                    {
                        ftStruct* strc = sit.getNext();
                        if (!strc->isLocked())
                        {
                            erase.push_back(strc);
                            structs.erase(strc);
                        }
                    }

                    sit = erase.iterator();
                    while (sit.hasMoreElements())
                    {
                        ftStruct* strc = sit.getNext();
                        if (!strc->isLocked())
                        {
                            strc->lock();
                            structs.insert_front(link, strc);
                        }
                    }
                }
            }
            else if (!fstrc->isLocked())
            {
                StructDeque::Link* link = structs.find(fstrc);
                if (link)
                {
                    fstrc->lock();
                    structs.erase(link);
                    toplevel.push_front(fstrc);
                }
            }
        }
    }

    dependent.clear();
    StructDeque::Iterator it = toplevel.iterator();
    while (it.hasMoreElements())
        dependent.push_back(it.getNext());

    it = structs.iterator();
    while (it.hasMoreElements())
        dependent.push_back(it.getNext());
}


void sortStructs(ProgramInfo&        ctx,
                 skArray<FBTtype*>&  unresolved,
                 skArray<ftStruct*>& independent,
                 skArray<ftStruct*>& dependent)
{
    skArray<ftStruct*> main = ctx.m_tables->getStructureArray();

    skArray<ftStruct*>::Iterator it = main.iterator();
    while (it.hasMoreElements())
    {
        ftStruct* cs = it.getNext();

        FBTtype* strc = ctx.m_tables->getStructAt(cs->getStructIndex());
        if (strc)
        {
            FBTtype cnt = strc[1], i;
            strc += 2;

            bool hasStructs           = false;
            bool hasNonPointerStructs = false;

            for (i = 0; i < cnt; ++i, strc += 2)
            {
                const ftType& type = ctx.m_tables->getTypeAt(strc[0]);
                const ftName& name = ctx.m_tables->getNameAt(strc[1]);

                if (type.m_strcId != SK_NPOS32 && name.m_ptrCount <= 0)
                    hasNonPointerStructs = true;

                if (type.m_strcId != SK_NPOS32)  // it's a valid struct
                    hasStructs = true;
                else if (name.m_ptrCount > 0)
                {
                    // if it's not a struct or an atomic type then it's unresolved.
                    if (string(type.m_name) != "bool")
                    {
                        ftAtomic atomic = ftAtomicUtils::getPrimitiveType(ftCharHashKey(type.m_name).hash());
                        if (atomic == ftAtomic::FT_ATOMIC_UNKNOWN)
                        {
                            if (!hasUnResolved(unresolved, strc))
                                unresolved.push_back(strc);
                        }
                    }
                }
            }
            if (!hasStructs || !hasNonPointerStructs)
                independent.push_back(cs);
            else
                dependent.push_back(cs);
        }
    }

    organizeDependentStructs(dependent, independent);
}


int extractToFile(ProgramInfo& ctx)
{
    int       status = 0;
    ftTables* tables = ctx.m_tables;

    skArray<FBTtype*>  unresolved;
    skArray<ftStruct*> independent, dependent;
    sortStructs(ctx, unresolved, independent, dependent);


    ofstream outf;
    ostream& sout = outf;

    outf.open(ctx.m_ofile.c_str());
    if (outf.is_open())
    {
        writeFileHeader(ctx, sout);

        if (ctx.m_useNamespace)
        {
            sout << "namespace " << ctx.m_outName << endl;
            sout << "{" << endl;
            sout << endl;
        }

        if (ctx.m_writeHashCode)
            writeHashCodes(ctx, sout, tables->getStructureArray());


        sout << "#pragma region Forward" << endl;
        skArray<ftStruct*>::Iterator it = tables->getStructIterator();
        while (it.hasMoreElements())
        {
            ftStruct* strc = it.getNext();
            writeForward(ctx, sout, strc);
        }
        sout << "#pragma endregion" << endl;
        sout << endl;


        sout << "// Pointers that have references to no known" << endl;
        sout << "// struct need to be declared as some type of handle." << endl;
        sout << "// This should be a struct handle class so that it can be" << endl;
        sout << "// recompiled. struct XXX {int unused; }" << endl;
        sout << "#pragma region MissingStructures" << endl;
        sout << endl;

        skArray<FBTtype*>::Iterator uit = unresolved.iterator();
        while (uit.hasMoreElements())
        {
            FBTtype* cur = uit.getNext();
            writeUnresolved(ctx, sout, cur);
        }
        sout << "#pragma endregion" << endl;
        sout << endl;


        sout << "// Independent structures:" << endl;
        sout << "// The member declarations only contain references to other " << endl;
        sout << "// structures via a pointer (or only atomic types); Therefore," << endl;
        sout << "// declaration order does not matter as long as any pointer " << endl;
        sout << "// reference is forwardly declared." << endl;
        sout << "#pragma region Independent" << endl;

        it = independent.iterator();
        while (it.hasMoreElements())
            writeStructure(ctx, sout, it.getNext());
        sout << "#pragma endregion" << endl;
        sout << endl;


        sout << "// Dependent structures:" << endl;
        sout << "// The member declarations have references to other " << endl;
        sout << "// structures without a pointer; Therefore, declaration order DOES matter." << endl;
        sout << "// If a structure has a non pointer member structure, then that structure " << endl;
        sout << "// must be visible before defining the structure that uses it." << endl;

        sout << "#pragma region Dependent" << endl;
        it = dependent.iterator();
        while (it.hasMoreElements())
            writeStructure(ctx, sout, it.getNext());
        sout << "#pragma endregion" << endl;
        sout << endl;
        if (ctx.m_useNamespace)
            sout << "}" << endl;
        writeFileFooter(ctx, sout);
    }
    else
    {
        status = -1;
        cout << "Failed to open the output file " << ctx.m_ofile.c_str() << endl;
    }
    return status;
}

void usage(const char* prog)
{
    if (prog)
    {
        printf("%s\n", prog);
        printf("       <options> -i <infile> -o <outfile>\n\n");
        printf("       <options>\n");
        printf("                 -n  Use namespace. Puts all declarations inside a namespace.\n");
        printf("                 -b  Fix .blend  nonstandard naming that would require -fpermissive.\n");
        printf("                 -s  include <stdint.h>.\n");
        printf("                 -c  Generate structure hash codes.\n");
        printf("                 -h  Display this message.\n");
        printf("\n");
    }
    else
        printf("Invalid program name supplied to usage.\n");
}

int parseCommandLine(ProgramInfo& ctx, int argc, char** argv)
{
    int i, status = 0;
    for (i = 0; i < argc && status == 0; ++i)
    {
        char* carg = argv[i];
        if (*carg == '-')
        {
            ++carg;
            if (*carg == 'i')
            {
                if (i + 1 < argc)
                    ctx.m_ifile = argv[++i];
                else
                {
                    printf(
                        "Parsed the end of the command line while "
                        "searching for the next input.\n");
                    status = -1;
                }
            }
            else if (*carg == 'h')
                usage(ctx.m_progName);
            else if (*carg == 'n')
                ctx.m_useNamespace = true;
            else if (*carg == 'b')
                ctx.m_fixupBlend = true;
            else if (*carg == 's')
                ctx.m_initTypes = true;
            else if (*carg == 'c')
                ctx.m_writeHashCode = true;
            else if (*carg == 'o')
            {
                if (i + 1 < argc)
                    ctx.m_ofile = argv[++i];
                else
                {
                    printf(
                        "Parsed the end of the command line while "
                        "searching for the next input.\n");
                    status = -1;
                }
            }
        }
    }

    if (ctx.m_ifile.empty())
    {
        status = -2;
        printf("The input file option (-i <infile>) is required.\n");
    }
    else if (ctx.m_ofile.empty())
    {
        status = -2;
        printf("The output file option (-o <outfile>) is required.\n");
    }

    return status;
}


int getBaseName(const char* input)
{
    int offs = 0;
    if (input)
    {
        int len = ((int)strlen(input)), i;
        for (i = len - 1; i >= 0 && offs == 0; --i)
            if (input[i] == '/' || input[i] == '\\')
                offs = i + 1;
    }
    return offs;
}



int main(int argc, char** argv)
{
    // The main idea of this program is
    // to load any type of file and scan for a DNA1 chunk
    // then extract the tables.

    char* base   = argv[0] + getBaseName(argv[0]);
    int   status = 0;

    ProgramInfo ctx    = {base, "", "", nullptr, nullptr, nullptr};
    ctx.m_useNamespace = false;


    if (parseCommandLine(ctx, argc, argv) < 0)
    {
        usage(base);
        return 1;
    }
    else
    {
        ftGzStream fp;
        fp.open(ctx.m_ifile.c_str(), skStream::READ);
        if (fp.isOpen())
        {
            ftScanDNA scanner;
            status = scanner.findHeaderFlags(&fp);
            if (status != ftFlags::FS_OK)
            {
                printf(
                    "Failed to extract the appropriate header "
                    "flags from the supplied file %s.\n",
                    ctx.m_ifile.c_str());
            }
            else
            {
                fp.seek(0, SEEK_SET);
                fp.read(ctx.m_header, 12);
                ctx.m_header[12] = 0;

                status = scanner.scan(&fp);
                if (status == ftFlags::FS_OK)
                {
                    ctx.m_dnaBlock = scanner.getDNA();
                    ctx.m_tables   = new ftTables(scanner.is64Bit() ? 8 : 4);

                    status = ctx.m_tables->read(ctx.m_dnaBlock,
                                                scanner.getLength(),
                                                scanner.getFlags(),
                                                ftFlags::LF_ONLY_ERR);
                    if (status == ftFlags::FS_OK)
                    {
                        status = extractToFile(ctx);
                    }
                    else
                    {
                        printf(
                            "Failed to rebuild the DNA table"
                            "from the supplied file %s.\n",
                            ctx.m_ifile.c_str());
                    }
                }
                else
                {
                    printf(
                        "Failed to properly scan to the DNA1 block"
                        "in the supplied file %s.\n",
                        ctx.m_ifile.c_str());
                }
            }
        }
        else
        {
            status = -1;
            printf("Failed to open the input file for reading.\n");
        }
    }

    if (ctx.m_tables)
        delete ctx.m_tables;
    if (ctx.m_dnaBlock)
        free(ctx.m_dnaBlock);
    return status;
}

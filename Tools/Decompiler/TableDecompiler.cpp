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

    // extra vars only valid in extract to file
    string m_outName;
};

using namespace ftFlags;


void writeFileHeader(ProgramInfo& ctx, ostream& out)
{
    char     buf[32];
    SKuint32 br = skGetTimeString(buf, 31, "%a %b %d %r");

    out << "/*" << endl
        << endl;
    out << "    This file was automatically generated." << endl;
    out << endl;
    out << "    By    : " << ctx.m_progName << endl;
    out << "    From  : " << ctx.m_ifile.c_str() << endl;
    out << "            " << ctx.m_header << endl;
    if (br != SK_NPOS32)
        out << "    On    : " << buf << endl
            << endl;
    out << "*/" << endl;

    ctx.m_outName = ctx.m_ofile.substr(0, ctx.m_ofile.find('.')).c_str();
    out << "#ifndef _" << ctx.m_outName << "_" << endl;
    out << "#define _" << ctx.m_outName << "_" << endl;
    out << endl;
}


void writeFileFooter(ProgramInfo& ctx, ostream& out)
{
    out << "#endif//_" << ctx.m_outName << "_" << endl;
}

void writeIndent(ostream& out, int nr, int spacePerIndent = 4)
{
    spacePerIndent = skMax(spacePerIndent - 1, 0);
    int i;
    for (i = 0; i < nr; ++i)
        out << setw(spacePerIndent) << ' ';
}

void writeForward(ProgramInfo& ctx, ostream& out, ftStruct* ftStrc)
{
    out << "struct " << ftStrc->getName() << ';' << endl;
}

void writeStructure(ProgramInfo& ctx, ostream& out, ftStruct* ftStrc)
{
    out << "struct " << ftStrc->getName() << endl;
    out << "{" << endl;
    FBTtype* strc = ctx.m_tables->getStructAt(ftStrc->getStructIndex());

    if (strc)
    {
        FBTtype cnt = strc[1], i;
        strc += 2;

        for (i = 0; i < cnt; ++i, strc += 2)
        {
            char* type = ctx.m_tables->getTypeNameAt(strc[0]);
            char* name = ctx.m_tables->getNameAt(strc[1]).m_name;

            if (type && name)
            {
                writeIndent(out, 1);
                out << type << ' ' << name << ';' << endl;
            }
        }
    }

    out << "};" << endl
        << endl;
}


void writeUnresolved(ProgramInfo& ctx, ostream& out, FBTtype* typeNotFound)
{
    char*         typeName = ctx.m_tables->getTypeNameAt(typeNotFound[0]);
    const ftName& name     = ctx.m_tables->getNameAt(typeNotFound[1]);

    if (name.m_ptrCount > 0)
    {
        out << "struct " << typeName << endl;
        out << "{" << endl;
        out << "    size_t missing;" << endl;
        out << "};" << endl;
    }
}


int hasDependantStructures(ftStruct* a)
{
    ftTables* tables = a->getParent();


    int deps = 0;
    if (tables)
    {
        FBTtype* strc = tables->getStructAt(a->getStructIndex());
        if (strc)
        {
            FBTtype cnt = strc[1], i;
            strc += 2;
            for (i = 0; i < cnt; ++i, strc += 2)
            {
                ftType type = tables->getTypeAt(strc[0]);
                ftName name = tables->getNameAt(strc[1]);

                if (type.m_strcId != SK_NPOS32)
                {
                    if (name.m_ptrCount <= 0)
                        return true;
                }
            }
        }
    }
    return false;
}

int hasDependantStructure(ftStruct* a, ftStruct* b)
{
    ftTables* tables = a->getParent();

    int deps = 0;
    if (tables)
    {
        FBTtype* strc = tables->getStructAt(a->getStructIndex());
        if (strc)
        {
            FBTtype cnt = strc[1], i;
            strc += 2;
            for (i = 0; i < cnt; ++i, strc += 2)
            {
                ftType type = tables->getTypeAt(strc[0]);
                ftName name = tables->getNameAt(strc[1]);

                if (string(b->getName()) == type.m_name)
                    return true;
            }
        }
    }
    return false;
}



void getUnresolved(ftStruct* a, skArray<FBTtype*>& unresolved)
{
    ftTables* tables = a->getParent();

    int deps = 0;
    if (tables)
    {
        FBTtype* strc = tables->getStructAt(a->getStructIndex());
        if (strc)
        {
            FBTtype cnt = strc[1], i;
            strc += 2;
            for (i = 0; i < cnt; ++i, strc += 2)
            {
                ftStruct* fstrc = tables->findStructByType(strc[0]);
                if (fstrc)
                {
                }
            }
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



bool hasType(const skArray<ftStruct*>& lst, const ftType& inp)
{
    bool found = false;

    skArray<ftStruct*>::ConstIterator it = lst.iterator();
    while (!found && it.hasMoreElements())
    {
        const ftStruct* type = it.getNext();

        found = string(type->getName()) == string(inp.m_name);
    }
    return found;
}



void select_sort(StructArray& srt)
{
    StructArray a2;
    StructDeque tmp, tmp2;
    if (!srt.empty())
    {
        StructArray::Iterator it = srt.iterator();
        while (it.hasMoreElements())
        {
            ftStruct* strc = it.getNext();

            if (hasDependantStructures(strc) > 0)
                a2.push_back(strc);
            else
                tmp.push_back(strc);
        }
    }

    srt.clear();

    {
        StructArray::Iterator it = a2.iterator();
        while (it.hasMoreElements())
        {
            ftStruct *a = it.getNext(), *b = 0, *c = 0;


            StructArray::Iterator it2 = a2.iterator();
            while (it2.hasMoreElements() && !b)
            {
                c = it2.getNext();
                if (c != a)
                {
                    if (hasDependantStructure(a, c))
                        b = c;
                
                }
            }


            if (b)
                tmp2.push_back(a);
            else
                tmp2.push_front(a);
        }

        {
            StructDeque::Iterator it = tmp2.iterator();
            while (it.hasMoreElements())
                srt.push_back(it.getNext());
        }
    }


    {
        StructDeque::Iterator it = tmp.iterator();
        while (it.hasMoreElements())
            srt.push_back(it.getNext());
    }
}


void sortStructs(ProgramInfo& ctx, skArray<FBTtype*>& unresolved, skArray<ftStruct*>& nondependent, skArray<ftStruct*>& dependent)
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

            bool hasStructs = false;

            for (i = 0; i < cnt; ++i, strc += 2)
            {
                ftType type = ctx.m_tables->getTypeAt(strc[0]);
                ftName name = ctx.m_tables->getNameAt(strc[1]);

                if (type.m_strcId != SK_NPOS32)
                {
                    hasStructs = true;
                    cout << type.m_name << ' ' << name.m_name << endl;
                }
                else if (name.m_ptrCount > 0)
                {
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

            if (!hasStructs)
                nondependent.push_back(cs);
            else
                dependent.push_back(cs);
        }
    }

    select_sort(dependent);
}


int extractToFile(ProgramInfo& ctx)
{
    int       status = 0;
    ftTables* tables = ctx.m_tables;

    skArray<FBTtype*>  unresolved;
    skArray<ftStruct*> nondependent, arr, dependent;
    sortStructs(ctx, unresolved, nondependent, dependent);


    ofstream outf;
    ostream& sout = outf;

    outf.open(ctx.m_ofile.c_str());
    if (outf.is_open())
    {
        writeFileHeader(ctx, sout);


        sout << "#pragma region Forwards" << endl;
        skArray<ftStruct*>::Iterator it = dependent.iterator();
        while (it.hasMoreElements())
        {
            ftStruct* strc = it.getNext();
            writeForward(ctx, sout, strc);
        }
        sout << "#pragma endregion" << endl;
        sout << endl;

        sout << "#pragma region MissingStructures" << endl
             << endl;

        sout << "// Pointers that are references to no known" << endl;
        sout << "// struct need to be declared as a handle type" << endl;
        sout << "// with the same length as a pointer." << endl;
        //sout << "//@makeft_ignore" << endl;

        skArray<FBTtype*>::Iterator uit = unresolved.iterator();
        while (uit.hasMoreElements())
        {
            FBTtype* cur = uit.getNext();
            writeUnresolved(ctx, sout, cur);
        }
        //sout << "//@makeft_ignore" << endl;
        sout << "#pragma endregion" << endl;
        sout << endl;

        sout << "#pragma region NonDependent" << endl;
        it = nondependent.iterator();
        while (it.hasMoreElements())
        {
            ftStruct* strc = it.getNext();
            writeStructure(ctx, sout, strc);
        }
        sout << "#pragma endregion" << endl;
        sout << endl;



        sout << "#pragma region Dependent" << endl;
        it = dependent.iterator();
        while (it.hasMoreElements())
        {
            ftStruct* strc = it.getNext();
            writeStructure(ctx, sout, strc);
        }
        sout << "#pragma endregion" << endl;
        sout << endl;



        writeFileFooter(ctx, sout);
    }
    else
    {
        status = -1;
        cout << "Failed to open the output file " << ctx.m_ofile.c_str() << endl;
    }
    return status;
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


void usage(const char* prog)
{
    if (prog)
    {
        printf("%s\n", prog);
        printf("       <options>  -i <infile> -o <outfile>\n");
        printf("\n");
        printf("\n");
        printf("\n");
    }
    else
        printf("Invalid program name supplied to usage.\n");
}

int main(int argc, char** argv)
{
    // the idea of this program is
    // to load any type of file,
    // and scan for a DNA1 chunk
    // and extract the tables.

    char* base   = argv[0] + getBaseName(argv[0]);
    int   status = 0;

    ProgramInfo ctx = {base, "", "", nullptr, nullptr, nullptr};
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
            if (status != FS_OK)
            {
                printf(
                    "Failed to extract the appropriate header "
                    "flags from the supplied file %s.\n",
                    ctx.m_ifile.c_str());
            }
            else
            {
                // it passed the header test, so save the header magic
                // for later printout
                fp.seek(0, SEEK_SET);
                fp.read(ctx.m_header, 12);
                ctx.m_header[12] = 0;

                status = scanner.scan(&fp);
                if (status == FS_OK)
                {
                    ctx.m_dnaBlock = scanner.getDNA();
                    ctx.m_tables   = new ftTables(scanner.is64Bit() ? 8 : 4);

                    status = ctx.m_tables->read(ctx.m_dnaBlock,
                                                scanner.getLength(),
                                                scanner.getFlags(),
                                                ftFlags::LF_ONLY_ERR);
                    if (status == FS_OK)
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

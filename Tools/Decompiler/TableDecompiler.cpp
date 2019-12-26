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
#include "ftPlatformHeaders.h"

#include "Utils/skString.h"
#include "ftCompiler.h"
#include "ftScanDNA.h"


struct ProgramInfo
{
    const char* m_progName;
    skString    m_ifile;
    skString    m_ofile;
    skStream*   m_stream;
    void*       m_dnaBlock;
    ftTables*   m_tables;
};

using namespace ftFlags;



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

int extractToFile(ProgramInfo& ctx)
{
    return -1;
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

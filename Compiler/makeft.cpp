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
#include <stdio.h>
#include "ftCompiler.h"


int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("makeft <tablename> <outfile> <infile>[0] ... <infile>[n]\n");
        printf("\n");
        printf("       tablename - A prefix on the generated table.  \n");
        printf("                   Eg: 'File' would be generated as:\n");
        printf("\n");
        printf("                      const unsigned char FileTable[]={...};\n");
        printf("                      const unsigned int  FileTableLen;\n");
        printf("\n");
        printf("       outfile   - The name of the output file that will be used to store the tables.\n");
        printf("       infile    - a space separated list of file names to compile.\n");
        return 1;
    }


    printf("makeft -> Building table %s\n", argv[1]);

    ftCompiler tables;
    for (int i = 3; i < argc; ++i)
    {
        if (tables.parseFile(argv[i]) < 0)
        {
            printf("makeft -> Parse Error: When compiling file %s\n", argv[i]);
            return 1;
        }
    }

    int code;
    if ((code = tables.buildTypes()) != LNK_OK)
    {
        printf("makeft -> Link Error: When compiling table %s\n", argv[1]);
        return code;
    }

    tables.writeFile(argv[1], argv[2]);
    return 0;
}

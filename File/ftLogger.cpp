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
#include "ftLogger.h"
#include <iomanip>
#include <iostream>
#include "Utils/skHexPrint.h"
#include "Utils/skPlatformHeaders.h"
#include "ftTables.h"

using namespace std;

#define FT_INVALID_CHUNK_READ printf("Failed to read chunk.\n");
#define FT_TABLE_FAILED printf("Failed to initialize tables.\n");
#define FT_DUPLICATE_BLOCK printf("Duplicate memory block.\n");

void ftLogger::log(int status)
{
    switch (status)
    {
    case ftFile::FS_INV_HEADER_STR:
        printf("Invalid header string.\n");
        break;
    case ftFile::FS_INV_LENGTH:
        printf("Invalid block length.\n");
        break;
    case ftFile::FS_INV_READ:
        printf("Invalid read.\n");
        break;
    case ftFile::FS_LINK_FAILED:
        printf("Linking failed.\n");
        break;
    case ftFile::FS_BAD_ALLOC:
        printf("Failed to allocate memory.\n");
        break;
    case ftFile::FS_INV_INSERT:
        printf("Map insertion failed.\n");
        break;
    case ftFile::FS_DUPLICATE_BLOCK:
        printf("Duplicate block read.\n");
        break;
    default:
        break;
    }
}

void ftLogger::log(int status, const char *msg, ...)
{
    log(status);
    if (msg)
    {
        char    buf[513];
        va_list lst;
        va_start(lst, msg);
        int size = skp_printf(buf, 512, msg, lst);
        va_end(lst);
        if (size < 0)
            size = 0;

        buf[size] = 0;
        printf("%s\n", buf);
    }
}



void ftLogger::logF(const char *msg, ...)
{
    if (msg)
    {
        char buf[513];

        va_list lst;
        va_start(lst, msg);
        int size = skp_printf(buf, 512, msg, lst);
        va_end(lst);
        if (size < 0)
            size = 0;

        buf[size] = 0;
        printf("%s\n", buf);
    }
}


void ftLogger_writeSeperator()
{
    skHexPrint::writeColor(CS_GREY);
    cout << setfill('-') << setw(FT_VOID8 ? 87 : 79) << '-';
    cout << setfill(' ') << endl;
    skHexPrint::writeColor(CS_WHITE);
}


void ftLogger::log(const ftFile::Chunk &chunk)
{
    char *cp = (char *)&chunk.m_code;
    char  buf[5];
    memcpy(buf, cp, 4);
    buf[4] = '\0';
    ftLogger_writeSeperator();


    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "Chunk" << endl;
    cout << "Code   : " << buf << endl;
    cout << "Len    : " << chunk.m_len << endl;
    cout << "Old    : " << chunk.m_old << endl;
    cout << "TypeId : " << chunk.m_typeid << endl;
    cout << "Count  : " << chunk.m_nr << endl;
    skHexPrint::writeColor(CS_WHITE);
}


void ftLogger::log(void *ptr, FBTsize len)
{
    ftLogger_writeSeperator();
    skHexPrint::dumpHex((char *)ptr, 0, len, skHexPrint::PF_DEFAULT, -1);
}


void ftLogger::log(ftBinTables *table, ftStruct *strc)
{
    ftLogger_writeSeperator();
    skHexPrint::writeColor(CS_GREEN);
    char *name = "", *tmpname;

    tmpname = (char *)table->getTypeNameAt(strc->getNameIndex());
    if (tmpname != nullptr)
        name = tmpname;


    cout << "Struct : " << name << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "-----------------------" << endl;
    skHexPrint::writeColor(CS_WHITE);

    cout << "Key           : " << '{' << strc->getTypeIndex() << ',' << strc->getNameIndex() << '}' << endl;
    cout << "Hash          : " << '{' << strc->getHashedType() << ',' << strc->getHashedName() << '}' << endl;
    cout << "Offset        : " << strc->getBufferOffset() << endl;
    cout << "Size In Bytes : " << strc->getSizeInBytes() << endl;
    cout << "Aligned 4     : " << (((strc->getSizeInBytes() % 4) == 0) ? 1 : 0) << endl;
}

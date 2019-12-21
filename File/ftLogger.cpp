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
#include "ftMember.h"
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
        char buf[513] = {};

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
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << setfill('-') << setw(FT_VOID8 ? 87 : 79) << '-';
    cout << setfill(' ') << endl;
    skHexPrint::writeColor(CS_WHITE);
}

void ftLogger_writeDivider()
{
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << setfill('-') << setw(FT_VOID8 ? 47 : 39) << '-';
    cout << setfill(' ') << endl;
    skHexPrint::writeColor(CS_WHITE);
}



void ftLogger::seperator()
{
    ftLogger_writeSeperator();
}

void ftLogger::divider()
{
    ftLogger_writeDivider();
}

void ftLogger::log(const ftChunk &chunk)
{
    char *cp = (char *)&chunk.m_code;
    char  buf[5];
    memcpy(buf, cp, 4);
    buf[4] = '\0';

    ftLogger_writeSeperator();

    skHexPrint::writeColor(CS_DARKYELLOW);
    cout << "Chunk" << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "Code   : " << buf << endl;
    cout << "Len    : " << dec << chunk.m_len << endl;
    cout << "Old    : 0x" << hex << chunk.m_old << endl;
    cout << "TypeId : " << dec << chunk.m_typeid << endl;
    cout << "Count  : " << dec << chunk.m_nr << endl;
    skHexPrint::writeColor(CS_WHITE);
}



void ftLogger::log(void *ptr, FBTsize len)
{
    skHexPrint::dumpHex((char *)ptr, 0, len, skHexPrint::PF_DEFAULT, -1);
}


void ftLogger::width(FBTsize w)
{
    cout << setw(w);
}

void ftLogger::newline()
{
    cout << endl;
}


void ftLogger::log(ftStruct *strc)
{
    ftLogger_writeSeperator();
    skHexPrint::writeColor(CS_GREEN);
    cout << "Struct : " << strc->getName() << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "-----------------------" << endl;

    skHexPrint::writeColor(CS_WHITE);
    cout << "Type          : " << strc->getTypeIndex() << endl;
    cout << "Hash          : " << strc->getHashedType() << endl;
    cout << "Size In Bytes : " << strc->getSizeInBytes() << endl;
    cout << "Aligned 4     : " << (((strc->getSizeInBytes() % 4) == 0) ? 1 : 0) << endl;
}



void ftLogger::log(ftMember *member)
{
    ftStruct *parent = member->getParent();
    if (parent != nullptr)
    {
        ftLogger_writeSeperator();
        skHexPrint::writeColor(CS_GREEN);
        cout << "Struct        : " << parent->getName() << endl;
        ftLogger_writeDivider();

        cout << "Type          : " << member->getType() << endl;
        cout << "Name          : " << member->getName() << endl;
        cout << "Pointer Count : " << member->getPointerCount() << endl;
        cout << "Array Size    : " << member->getArraySize() << endl;
    }
}



void ftLogger_logStructTable(ftStruct *strc)
{
    cout << "Struct        : " << strc->getName() << endl;
    ftLogger_writeDivider();

    int                         nr = 0;
    ftStruct::Members::Iterator it = strc->getMemberIterator();
    while (it.hasMoreElements())
    {
        ftMember *mbr = it.getNext();
        cout << setw(10) << nr;
        cout << setw(20) << mbr->getType();
        cout << setw(20) << mbr->getName();
        cout << endl;
        ++nr;
    }
}



void ftLogger::log(ftStruct *fstrc, ftStruct *mstrc)
{
    int A = fstrc->getMemberCount();
    int B = mstrc->getMemberCount();
    int C = skMax(A, B);
    int D = 0;

    cout << setw(30) << ' ';
    cout << fstrc->getName() << endl;
    ftLogger_writeSeperator();
    cout << left;
    for (D = 0; D < C; ++D)
    {
        ftMember *fmbr = 0, *mmbr = 0;

        if (D < A)
            fmbr = fstrc->getMember(D);
        if (D < B)
            mmbr = mstrc->getMember(D);

        if (fmbr != 0)
        {
            cout << setw(15) << fmbr->getType();
            cout << setw(25) << fmbr->getName();
        }
        else
            cout << setw(40) << "Missing";

        if (mmbr != 0)
        {
            cout << setw(15) << mmbr->getType();
            cout << setw(25) << mmbr->getName();
        }
        else
            cout << setw(40) << "Missing";

        cout << endl;
    }
    ftLogger_writeSeperator();
    cout << endl;
}

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
#include <string>
#include "Utils/skHexPrint.h"
#include "Utils/skPlatformHeaders.h"
#include "ftMember.h"
#include "ftTables.h"

using namespace std;
using namespace ftFlags;


void ftLogger::log(int status)
{
    cout << "Exit status: ";
    switch (status)
    {
    case FS_INV_HEADER_STR:
        cout << "FS_INV_HEADER_STR";
        break;
    case FS_INV_LENGTH:
        cout << "FS_INV_LENGTH";
        break;
    case FS_INV_READ:
        cout << "FS_INV_READ";
        break;
    case FS_LINK_FAILED:
        cout << "FS_LINK_FAILED";
        break;
    case FS_BAD_ALLOC:
        cout << "FS_BAD_ALLOC";
        break;
    case FS_INV_INSERT:
        cout << "FS_INV_INSERT";
        break;
    case FS_DUPLICATE_BLOCK:
        cout << "FS_DUPLICATE_BLOCK";
        break;
    case FS_TABLE_INIT_FAILED:
        cout << "FS_TABLE_INIT_FAILED";
        break;
    case FS_OVERFLOW:
        cout << "FS_OVERFLOW";
        break;
    case FS_FAILED:
        cout << "FS_FAILED";
        break;
    case RS_INVALID_PTR:
        cout << "RS_INVALID_PTR";
        break;
    case RS_INVALID_CODE:
        cout << "RS_INVALID_CODE";
        break;
    case RS_LIMIT_REACHED:
        cout << "RS_LIMIT_REACHED";
        break;
    case RS_BAD_ALLOC:
        cout << "RS_BAD_ALLOC";
        break;
    case RS_MIS_ALIGNED:
        cout << "RS_MIS_ALIGNED";
        break;
    default:
        break;
    }
    cout << endl;
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
        cout << buf << endl;
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
        cout << buf << endl;
    }
}


void ftLogger::seperator()
{
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << setfill('-') << setw(FT_VOID8 ? 87 : 79) << '-';
    cout << setfill(' ') << endl;
    skHexPrint::writeColor(CS_WHITE);
}

void ftLogger::divider()
{
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << setfill('-') << setw(FT_VOID8 ? 47 : 39) << '-';
    cout << setfill(' ') << endl;
    skHexPrint::writeColor(CS_WHITE);
}

void ftLogger::log(const ftChunk &chunk)
{
    char *cp = (char *)&chunk.m_code;
    char  buf[5];
    memcpy(buf, cp, 4);
    buf[4] = '\0';

    seperator();

    skHexPrint::writeColor(CS_DARKYELLOW);
    cout << "Chunk" << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "Code   : " << buf << endl;
    cout << "Len    : " << dec << chunk.m_len << endl;
    cout << "Old    : 0x" << hex << chunk.m_addr << endl;
    cout << "TypeId : " << dec << chunk.m_typeid << endl;
    cout << "Count  : " << dec << chunk.m_nr << endl;
    skHexPrint::writeColor(CS_WHITE);
}



void ftLogger::log(void *ptr, FBTsize len)
{
    skHexPrint::dumpHex((char *)ptr, 0, len, skHexPrint::PF_DEFAULT, -1);
}


void ftLogger::newline(int nr)
{
    int i;
    for (i = 0; i < nr; ++i)
        cout << endl;
}


void ftLogger::log(ftStruct *strc)
{
    skHexPrint::writeColor(CS_GREEN);
    cout << "Struct : " << strc->getName() << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
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
        seperator();
        skHexPrint::writeColor(CS_GREEN);
        cout << "Struct        : " << parent->getName() << endl;
        divider();
        cout << "Type          : " << member->getType() << endl;
        cout << "Name          : " << member->getName() << endl;
        cout << "Pointer Count : " << member->getPointerCount() << endl;
        cout << "Array Size    : " << member->getArraySize() << endl;
    }
}



void ftLogger_logStructTable(ftStruct *strc)
{
    cout << "Struct        : " << strc->getName() << endl;
    ftLogger::divider();

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



string makeName(const char *name, size_t max)
{
    string rv = name;
    if (rv.size() > max - 2)
    {
        string nrv;
        for (size_t i = 0; i < max - 5; ++i)
            nrv.push_back(rv[i]);
        nrv += "...";
        rv = nrv;
    }
    return rv;
}



void ftLogger::log(ftStruct *fstrc, ftStruct *mstrc)
{
    int A = fstrc->getMemberCount();
    int B = mstrc->getMemberCount();
    int C = skMax(A, B);
    int D = 0;
    seperator();
    cout << left;

    cout << ' ';
    cout << setw(15) << "Type";
    cout << setw(15) << "Name";
    cout << setw(10) << "Offset";
    cout << setw(15) << "Type";
    cout << setw(15) << "Name";
    cout << setw(10) << "Offset";
    cout << endl;
    seperator();

    for (D = 0; D < C; ++D)
    {
        ftMember *fmbr = 0, *mmbr = 0;
        if (D < A)
            fmbr = fstrc->getMember(D);
        if (D < B)
            mmbr = mstrc->getMember(D);

        cout << ' ';
        if (fmbr != 0)
        {
            cout << setw(15) << makeName(fmbr->getType(), 15);
            cout << setw(15) << makeName(fmbr->getName(), 15);
            cout << setw(10) << fmbr->getOffset();
        }
        else
            cout << setw(40) << ' ';
        if (mmbr != 0)
        {
            cout << setw(15) << makeName(mmbr->getType(), 15);
            cout << setw(15) << makeName(mmbr->getName(), 15);
            cout << setw(10) << mmbr->getOffset();
        }
        else
            cout << setw(40) << ' ';

        cout << endl;
    }
    seperator();

    cout << "Size in bytes:";
    cout << right;
    cout << setw(25) << fstrc->getSizeInBytes();
    cout << setw(40) << mstrc->getSizeInBytes();
    cout << endl;
    cout << left;
}


void ftLogger::log(const ftName &name)
{
    seperator();
    skHexPrint::writeColor(CS_GREEN);
    cout << "Name                 : " << name.m_name << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "Hash                 : " << name.m_hash << endl;
    cout << "Pointer Count        : " << name.m_ptrCount << endl;
    cout << "Function pointer     : " << name.m_isFunctionPointer << endl;
    cout << "Number Of Dimensions : " << name.m_numDimensions << endl;
    cout << "Dimension Size       : ";

    int i;
    for (i = 0; i < skMax(name.m_numDimensions, 1); ++i)
    {
        if (i > 0)
            cout << ',' << ' ';
        cout << name.m_dimensions[i];
    }
    cout << endl;
    skHexPrint::writeColor(CS_WHITE);
}


void ftLogger::log(const ftType &type)
{
    seperator();
    skHexPrint::writeColor(CS_GREEN);
    cout << "Name                 : " << type.m_name << endl;
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << "Hash                 : " << type.m_hash << endl;
    cout << "Structure ID         : " << type.m_strcId << endl;
}


void ftLogger::log(const ftType &type, FBTtype size)
{
    cout << right << setw(10) << size << ' ';
    skHexPrint::writeColor(CS_LIGHT_GREY);
    cout << left << type.m_name;

    cout << endl;
    skHexPrint::writeColor(CS_WHITE);
}

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

using namespace std;


void ftLogger::log(const char *msg)
{
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
}


void ftLogger::log(void *ptr, FBTsize len)
{
    ftLogger_writeSeperator();
    skHexPrint::dumpHex((char *)ptr, 0, len, skHexPrint::PF_DEFAULT, -1);
}

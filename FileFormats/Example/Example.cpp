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
#include "Example.h"
#include "ftTables.h"
#include "ftStreams.h"

#define ftIN_SOURCE
#include "ftPlatformHeaders.h"


namespace ExampleCodes
{
    const FBTuint32 INFO = ftID('I', 'N', 'F', 'O');
}


Example::Example() :
    ftFile("Example")
{

    m_info.major    = FileVersionMajor;
    m_info.minor    = FileVersionMinor;
    m_info.build    = FileVersionBuild;

    m_info.revision = FileVersionRevision;

    ftp_sprintf(m_info.versionString, 32, "%i%i%i", 
        m_info.major, m_info.minor, m_info.build);

    m_version = atoi(m_info.versionString);
}



Example::~Example()
{
}

extern unsigned char ExampleTablesTable[];
extern int ExampleTablesLen;


void* Example::getTables(void)
{
    return ExampleTablesTable;
}

FBTsize Example::getTableSize(void)
{
    return ExampleTablesLen;
}

int Example::dataRead(void* p, const Chunk& id)
{
    ftASSERT(p);
    if (id.m_code == ExampleCodes::INFO)
        m_info = *((FileInfo*)p);

    return 0;
}

int Example::writeData(ftStream* stream)
{
    writeStruct(stream, "FileInfo", ExampleCodes::INFO, sizeof(FileInfo), &m_info);

    return FS_OK;
}

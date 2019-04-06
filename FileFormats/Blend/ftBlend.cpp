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
#include "ftBlend.h"
#include "ftTables.h"
#include "ftStreams.h"



extern unsigned char    bfBlenderTable[];
extern int              bfBlenderLen;

const FBTuint32 GLOB = ftID('G', 'L', 'O', 'B');


struct ftIdDB
{
    const FBTuint16 m_code;
    ftList ftBlend::* m_ptr;
};


ftIdDB ftData[] =
{
    { ftID2('S', 'C'), &ftBlend::m_scene },
    { ftID2('L', 'I'), &ftBlend::m_library },
    { ftID2('O', 'B'), &ftBlend::m_object },
    { ftID2('M', 'E'), &ftBlend::m_mesh },
    { ftID2('C', 'U'), &ftBlend::m_curve },
    { ftID2('M', 'B'), &ftBlend::m_mball },
    { ftID2('M', 'A'), &ftBlend::m_mat },
    { ftID2('T', 'E'), &ftBlend::m_tex },
    { ftID2('I', 'M'), &ftBlend::m_image },
    { ftID2('L', 'T'), &ftBlend::m_latt },
    { ftID2('L', 'A'), &ftBlend::m_lamp },
    { ftID2('C', 'A'), &ftBlend::m_camera },
    { ftID2('I', 'P'), &ftBlend::m_ipo },
    { ftID2('K', 'E'), &ftBlend::m_key },
    { ftID2('W', 'O'), &ftBlend::m_world },
    { ftID2('S', 'N'), &ftBlend::m_screen},
    { ftID2('P', 'Y'), &ftBlend::m_script },
    { ftID2('V', 'F'), &ftBlend::m_vfont },
    { ftID2('T', 'X'), &ftBlend::m_text },
    { ftID2('S', 'O'), &ftBlend::m_sound },
    { ftID2('G', 'R'), &ftBlend::m_group },
    { ftID2('A', 'R'), &ftBlend::m_armature },
    { ftID2('A', 'C'), &ftBlend::m_action },
    { ftID2('N', 'T'), &ftBlend::m_nodetree },
    { ftID2('B', 'R'), &ftBlend::m_brush },
    { ftID2('P', 'A'), &ftBlend::m_particle },
    { ftID2('G', 'D'), &ftBlend::m_gpencil },
    { ftID2('W', 'M'), &ftBlend::m_wm },
    { 0, 0 }
};

ftBlend::ftBlend() :
    ftFile("BLENDER"),
    m_stripList(0),
    m_stripListLen(0)
{
}

ftBlend::~ftBlend()
{
}

int ftBlend::initializeTables(ftBinTables* tables)
{
    return tables->read(bfBlenderTable, bfBlenderLen, false) ? FS_OK : FS_FAILED;
}

int ftBlend::notifyData(void* p, const Chunk& id)
{
    if (id.m_code == GLOB)
    {
        m_fg = (Blender::FileGlobal*)p;
        return FS_OK;
    }

    if ((id.m_code <= 0xFFFF))
    {
        int i = 0;
        while (ftData[i].m_code != 0)
        {
            if (ftData[i].m_code == id.m_code)
            {
                (this->*ftData[i].m_ptr).push_back(p);
                break;
            }
            ++i;
        }
    }
    return FS_OK;
}

int ftBlend::writeData(ftStream* stream)
{
    ftBinTables::OffsM::Pointer md = m_memory->m_offs.ptr();
    for (MemoryChunk* node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_newTypeId > m_memory->m_strcNr)
            continue;
        if (!node->m_newBlock)
            continue;

        void* wd = node->m_newBlock;

        Chunk ch;
        ch.m_code   = node->m_chunk.m_code;
        ch.m_nr     = node->m_chunk.m_nr;
        ch.m_len    = node->m_chunk.m_len;
        ch.m_typeid = node->m_newTypeId;
        ch.m_old    = (FBTsize)wd;

        stream->write(&ch, sizeof(Chunk));
        stream->write(wd, ch.m_len);
    }
    return FS_OK;
}



bool ftBlend::skip(const FBTuint32& id)
{
    if (!m_stripList)
        return false;
    int f = 0, l = m_stripListLen - 1, m, i=0;

    while (f <= l)
    {
        ++i;

        m = (f + l) / 2;
        if (m_stripList[m] == id)
            return true;
        else if (m_stripList[m] > id)
            l = m - 1;
        else 
            f = m + 1;
    }
    return false;
}



void ftBlend::setIgnoreList(FBTuint32* stripList) 
{ 
    m_stripList = stripList; 
    if (!m_stripList)
        return;
    int i = 0, j, k;
    while (m_stripList[i++] != 0);

    m_stripListLen = i;
    for (i = 0; i < m_stripListLen - 1; i++)
    {
        k = i;
        for (j = i + 1; j < m_stripListLen; ++j)
        {
            if (stripList[j] < stripList[k])
                k = j;
        }
        if (k != i)
            ftSwap(m_stripList[i], m_stripList[k]);
    }
}


void* ftBlend::getTables(void)
{
    return (void*)bfBlenderTable;
}

FBTsize ftBlend::getTableSize(void)
{
    return bfBlenderLen;
}

int ftBlend::save(const char* path, const int mode)
{
    m_version = m_fileVersion;
    return ftFile::save(path, mode);
}

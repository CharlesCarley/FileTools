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
#include "ftStreams.h"
#include "ftTables.h"

const FBTuint32 GLOB = FT_TYPEID('G', 'L', 'O', 'B');


struct ftIdDB
{
    const FBTuint16 m_code;
    ftList ftBlend::*m_ptr;
};


const ftIdDB ftData[] =
    {
        {FT_TYPEID2('S', 'C'), &ftBlend::m_scene},
        {FT_TYPEID2('L', 'I'), &ftBlend::m_library},
        {FT_TYPEID2('O', 'B'), &ftBlend::m_object},
        {FT_TYPEID2('M', 'E'), &ftBlend::m_mesh},
        {FT_TYPEID2('C', 'U'), &ftBlend::m_curve},
        {FT_TYPEID2('M', 'B'), &ftBlend::m_mball},
        {FT_TYPEID2('M', 'A'), &ftBlend::m_mat},
        {FT_TYPEID2('T', 'E'), &ftBlend::m_tex},
        {FT_TYPEID2('I', 'M'), &ftBlend::m_image},
        {FT_TYPEID2('L', 'T'), &ftBlend::m_latt},
        {FT_TYPEID2('L', 'A'), &ftBlend::m_lamp},
        {FT_TYPEID2('C', 'A'), &ftBlend::m_camera},
        {FT_TYPEID2('I', 'P'), &ftBlend::m_ipo},
        {FT_TYPEID2('K', 'E'), &ftBlend::m_key},
        {FT_TYPEID2('W', 'O'), &ftBlend::m_world},
        {FT_TYPEID2('S', 'N'), &ftBlend::m_screen},
        {FT_TYPEID2('P', 'Y'), &ftBlend::m_script},
        {FT_TYPEID2('V', 'F'), &ftBlend::m_vfont},
        {FT_TYPEID2('T', 'X'), &ftBlend::m_text},
        {FT_TYPEID2('S', 'O'), &ftBlend::m_sound},
        {FT_TYPEID2('G', 'R'), &ftBlend::m_group},
        {FT_TYPEID2('A', 'R'), &ftBlend::m_armature},
        {FT_TYPEID2('A', 'C'), &ftBlend::m_action},
        {FT_TYPEID2('N', 'T'), &ftBlend::m_nodetree},
        {FT_TYPEID2('B', 'R'), &ftBlend::m_brush},
        {FT_TYPEID2('P', 'A'), &ftBlend::m_particle},
        {FT_TYPEID2('G', 'D'), &ftBlend::m_gpencil},
        {FT_TYPEID2('W', 'M'), &ftBlend::m_wm},
        {0, 0}};



ftBlend::ftBlend() :
    ftFile("BLENDER"),
    m_fg()
{
}

ftBlend::~ftBlend()
{
}

int ftBlend::notifyDataRead(void* p, const ftChunk& id)
{
    if (id.m_code == GLOB)
    {
        m_fg = (Blender::FileGlobal*)p;
        return ftFlags::FS_OK;
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
    return ftFlags::FS_OK;
}

int ftBlend::serializeData(skStream* stream)
{
    for (ftMemoryChunk* node = (ftMemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_newTypeId > m_memory->getNumberOfTypes())
            continue;
        if (!node->m_mblock)
            continue;

        void* wd = node->m_mblock;
        ftChunk ch;
        ch.m_code   = node->m_chunk.m_code;
        ch.m_nr     = node->m_chunk.m_nr;
        ch.m_len    = node->m_chunk.m_len;
        ch.m_typeid = node->m_newTypeId;
        ch.m_addr    = (FBTsize)wd;

        stream->write(&ch, sizeof(ftChunk));
        stream->write(wd, ch.m_len);
    }
    return ftFlags::FS_OK;
}

int ftBlend::save(const char* path, const int mode)
{
    m_memoryVersion = m_fileVersion;
    return ftFile::save(path, mode);
}


#include "bfBlender.inl"

void* ftBlend::getTables(void)
{
    return (void*)bfBlenderTable;
}

FBTsize ftBlend::getTableSize(void)
{
    return bfBlenderLen;
}

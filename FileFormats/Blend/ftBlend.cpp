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



const FBTuint32 GLOB = FileTools_ID('G', 'L', 'O', 'B');

struct ftIdDB
{
	const FBTuint16     m_code;
	ftList             ftBlend::*m_ptr;
};

ftIdDB ftData[] =
{
	{ FileTools_ID2('S', 'C'), &ftBlend::m_scene},
	{ FileTools_ID2('L', 'I'), &ftBlend::m_library },
	{ FileTools_ID2('O', 'B'), &ftBlend::m_object },
	{ FileTools_ID2('M', 'E'), &ftBlend::m_mesh },
	{ FileTools_ID2('C', 'U'), &ftBlend::m_curve },
	{ FileTools_ID2('M', 'B'), &ftBlend::m_mball },
	{ FileTools_ID2('M', 'A'), &ftBlend::m_mat },
	{ FileTools_ID2('T', 'E'), &ftBlend::m_tex },
	{ FileTools_ID2('I', 'M'), &ftBlend::m_image },
	{ FileTools_ID2('L', 'T'), &ftBlend::m_latt },
	{ FileTools_ID2('L', 'A'), &ftBlend::m_lamp },
	{ FileTools_ID2('C', 'A'), &ftBlend::m_camera },
	{ FileTools_ID2('I', 'P'), &ftBlend::m_ipo },
	{ FileTools_ID2('K', 'E'), &ftBlend::m_key },
	{ FileTools_ID2('W', 'O'), &ftBlend::m_world },
	{ FileTools_ID2('S', 'N'), &ftBlend::m_screen},
	{ FileTools_ID2('P', 'Y'), &ftBlend::m_script },
	{ FileTools_ID2('V', 'F'), &ftBlend::m_vfont },
	{ FileTools_ID2('T', 'X'), &ftBlend::m_text },
	{ FileTools_ID2('S', 'O'), &ftBlend::m_sound },
	{ FileTools_ID2('G', 'R'), &ftBlend::m_group },
	{ FileTools_ID2('A', 'R'), &ftBlend::m_armature },
	{ FileTools_ID2('A', 'C'), &ftBlend::m_action },
	{ FileTools_ID2('N', 'T'), &ftBlend::m_nodetree },
	{ FileTools_ID2('B', 'R'), &ftBlend::m_brush },
	{ FileTools_ID2('P', 'A'), &ftBlend::m_particle },
	{ FileTools_ID2('G', 'D'), &ftBlend::m_gpencil },
	{ FileTools_ID2('W', 'M'), &ftBlend::m_wm },
	{ 0, 0 }
};




extern unsigned char bfBlenderFBT[];
extern int bfBlenderLen;


ftBlend::ftBlend()
	:   ftFile("BLENDER"), m_stripList(0)
{
	m_aluhid = "BLENDEs"; //a stripped blend file
}



ftBlend::~ftBlend()
{
}



int ftBlend::initializeTables(ftBinTables* tables)
{
	return tables->read(bfBlenderFBT, bfBlenderLen, false) ? FS_OK : FS_FAILED;
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

	int i = 0;
	while (m_stripList[i] != 0)
	{
		if (m_stripList[i++] == id)
			return true;
	}

	return false;
}


void*   ftBlend::getFBT(void)
{
	return (void*)bfBlenderFBT;
}

FBTsize ftBlend::getFBTlength(void)
{
	return bfBlenderLen;
}

int ftBlend::save(const char *path, const int mode)
{
	m_version = m_fileVersion;
	return ftFile::save(path, mode);
}

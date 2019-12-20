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
#ifndef _ftChunk_h_
#define _ftChunk_h_

#include "ftTypes.h"



struct Chunk32
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTuint32 m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen32, sizeof(ftFile::Chunk32) == 20);


struct Chunk64
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTuint64 m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen64, sizeof(ftFile::Chunk64) == 24);


struct Chunk
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTsize   m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};
#if ftARCH == ftARCH_32
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk32) == sizeof(ftFile::Chunk));
#else
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk64) == sizeof(ftFile::Chunk));
#endif



struct MemoryChunk
{
    enum Flag
    {
        BLK_MODIFIED = (1 << 0),
    };

    MemoryChunk *m_next, *m_prev;

    Chunk    m_chunk;
    void*    m_fblock;
    void*    m_mblock;
    FBTuint8 m_flag;
    FBTtype  m_newTypeId;
};



struct ftChunk
{
    enum Size
    {
        BlockSize = sizeof(Chunk),
        Block32   = sizeof(Chunk32),
        Block64   = sizeof(Chunk64),
    };
    static FBTsize read(Chunk* dest, skStream* stream, int flags);
    static FBTsize write(Chunk* src, skStream* stream);

    static const Chunk BLANK_CHUNK;
};


#endif  //_ftChunk_h_

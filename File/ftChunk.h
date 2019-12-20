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



struct ftChunk32
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTuint32 m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen32, sizeof(ftChunk32) == 20);


struct ftChunk64
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTuint64 m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen64, sizeof(ftChunk64) == 24);


struct ftChunk
{
    FBTuint32 m_code;
    FBTuint32 m_len;
    FBTsize   m_old;
    FBTuint32 m_typeid;
    FBTuint32 m_nr;
};


#if ftARCH == ftARCH_32
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftChunk32) == sizeof(ftChunk));
#else
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftChunk64) == sizeof(ftChunk));
#endif



struct ftMemoryChunk
{
    enum Flag
    {
        BLK_MODIFIED = (1 << 0),
    };

    ftMemoryChunk *m_next, *m_prev;

    ftChunk  m_chunk;
    void*    m_fblock;
    void*    m_mblock;
    FBTuint8 m_flag;
    FBTtype  m_newTypeId;
};



struct ftChunkUtils
{
    enum Size
    {
        BlockSize = sizeof(ftChunk),
        Block32   = sizeof(ftChunk32),
        Block64   = sizeof(ftChunk64),
    };
    static FBTsize read(ftChunk* dest, skStream* stream, int flags);
    static FBTsize write(ftChunk* src, skStream* stream);

    static const ftChunk BLANK_CHUNK;
};


#endif  //_ftChunk_h_

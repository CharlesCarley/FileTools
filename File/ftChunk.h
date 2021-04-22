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


struct ftChunkScan
{
    SKuint32 m_code;
    SKuint32 m_len;
};



struct ftChunk32
{
    SKuint32 m_code;
    SKuint32 m_len;
    SKuint32 m_addr;
    SKuint32 m_structId;
    SKuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen32, sizeof(ftChunk32) == 20);


struct ftChunk64
{
    SKuint32 m_code;
    SKuint32 m_len;
    SKuint64 m_addr;
    SKuint32 m_structId;
    SKuint32 m_nr;
};
SK_ASSERTCOMP(ChunkLen64, sizeof(ftChunk64) == 24);


struct ftChunk
{
    SKuint32 m_code;
    SKuint32 m_len;
    SKsize   m_addr;
    SKuint32 m_structId;
    SKuint32 m_nr;
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
        BLK_MODIFIED = 1 << 0,
        BLK_LINKED   = 1 << 1,
    };

    ftMemoryChunk *m_next, *m_prev;
    ftChunk        m_chunk;
    
    // m_fblock: is the block of memory that was allocated and read 
    // from the file. It contains the data of the structure at 
    // the time of saving.
    void* m_fblock;

    // m_mblock: is the block of memory allocated for conversion. 
    // Its length is the size of the corresponding structure in its 
    // current state. The data from m_fblock gets cast into m_mblock 
    // one member at a time. 
    void* m_mblock;

    // m_pblock: is the storage location for pointers to pointers. 
    // When casting m_fblock into m_mblock, if the current member is 
    // a pointer to a pointer, this block provides the storage location 
    // for each of the pointers. The address of m_pblock is assigned to 
    // m_mblock at the offset for the pointer to pointer member.
    void*     m_pblock;
    SKuint32 m_pblockLen;

    SKuint8  m_flag;
    SKtype   m_newTypeId;

    ftStruct* m_fstrc;
    ftStruct* m_mstrc;
};


struct ftChunkUtils
{
    enum Size
    {
        BlockSize = sizeof(ftChunk),
        Block32   = sizeof(ftChunk32),
        Block64   = sizeof(ftChunk64),
        BlockScan = sizeof(ftChunkScan),
    };

    static SKsize read(ftChunk* dest, skStream* stream, int flags);
    static SKsize write(ftChunk* src, skStream* stream);
    static SKsize scan(ftChunkScan* dest, skStream* stream, int flags);

    static const ftChunk BLANK_CHUNK;
};

#endif  //_ftChunk_h_

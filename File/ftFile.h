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
#ifndef _ftFile_h_
#define _ftFile_h_

#include "Utils/skList.h"
#include "Utils/skMap.h"
#include "ftHashTypes.h"
#include "ftTypes.h"

class skStream;
class ftMemoryStream;
class ftBinTables;
class ftStruct;
struct ftName;
class ftMember;



class ftFile
{
public:
    enum FileMagic
    {
        FM_BIG_ENDIAN    = 'V',
        FM_LITTLE_ENDIAN = 'v',
        FM_32_BIT        = '_',
        FM_64_BIT        = '-',
    };

    enum FileStatus
    {
        FS_LINK_FAILED = -8,
        FS_INV_INSERT,
        FS_BAD_ALLOC,
        FS_DUPLICATE_BLOCK,
        FS_INV_READ,
        FS_INV_LENGTH,
        FS_INV_HEADER_STR,
        FS_FAILED,
        FS_OK,  // should always be zero
    };

    enum ParseMode
    {
        PM_UNCOMPRESSED,
        PM_COMPRESSED,
        PM_READTOMEMORY,

    };
    enum FileHeader
    {
        FH_ENDIAN_SWAP = (1 << 0),
        FH_CHUNK_64    = (1 << 1),
        FH_VAR_BITS    = (1 << 2),
    };

    struct Chunk32
    {
        FBTuint32 m_code;
        FBTuint32 m_len;
        FBTuint32 m_old;
        FBTuint32 m_typeid;
        FBTuint32 m_nr;
    };

    struct Chunk64
    {
        FBTuint32 m_code;
        FBTuint32 m_len;
        FBTuint64 m_old;
        FBTuint32 m_typeid;
        FBTuint32 m_nr;
    };

    struct Chunk
    {
        FBTuint32 m_code;
        FBTuint32 m_len;
        FBTsize   m_old;  // varies
        FBTuint32 m_typeid;
        FBTuint32 m_nr;
    };

    struct MemoryChunk
    {
        enum Flag
        {
            BLK_MODIFIED = (1 << 0),
        };

        MemoryChunk *m_next, *m_prev;
        ftStruct*    m_attached;
        Chunk        m_chunk;
        void*        m_fblock;
        void*        m_mblock;
        FBTuint8     m_flag;
        FBTtype      m_newTypeId;
    };

    typedef skHashTable<SKuintPtr, MemoryChunk*> ChunkMap;
    typedef ftList                               MemoryChunks;


private:
    int               m_headerFlags;
    const char*       m_uhid;
    ftFixedString<12> m_header;
    char*             m_curFile;
    void*             m_fileData;

protected:
    int          m_version, m_fileVersion;
    MemoryChunks m_chunks;
    ChunkMap     m_map;
    ftBinTables* m_memory;
    ftBinTables* m_file;

public:
    ftFile(const char* uid);
    virtual ~ftFile();


    int load(const char* path, int mode = PM_UNCOMPRESSED);
    int load(const void* memory, FBTsize sizeInBytes, int mode = PM_UNCOMPRESSED);
    int save(const char* path, const int mode = PM_UNCOMPRESSED);

    ftBinTables* getMemoryTable(void);

    inline const ftFixedString<12>& getHeader(void) const
    {
        return m_header;
    }

    inline const int& getVersion(void) const
    {
        return m_fileVersion;
    }

    inline const char* getPath(void) const
    {
        return m_curFile;
    }

    inline ftBinTables* getFileTable(void)
    {
        return m_file;
    }

    inline MemoryChunks& getChunks(void)
    {
        return m_chunks;
    }


    // Enable a filter for structures
    // inclusive - true:  Filter everything except what is in the list.
    // inclusive - false: Filter everything not but list.
    virtual void setFilterList(FBTuint32* filter, bool inclusive = false)
    {
        // override to handle
    }

    void serialize(skStream* stream, const char* id, FBTuint32 code, FBTsize len, void* writeData);
    void serialize(skStream* stream, FBTtype index, FBTuint32 code, FBTsize len, void* writeData);
    void serialize(skStream* stream, FBTsize len, void* writeData, int nr = 1);


protected:
    int initializeTables(ftBinTables* tables);
    int initializeMemory(void);

    virtual bool skip(const FBThash& id)
    {
        return false;
    }

    virtual void*   getTables(void)                          = 0;
    virtual FBTsize getTableSize(void)                       = 0;
    virtual int     notifyDataRead(void* p, const Chunk& id) = 0;
    virtual int     serializeData(skStream* stream)          = 0;

private:
    void*        findPtr(const FBTsize& iptr);
    MemoryChunk* findBlock(const FBTsize& iptr);
    skStream*    openStream(const char* path, int mode);


    void handleTable(skStream* stream, void* block, const Chunk& chunk, int& status);
    void handleChunk(skStream* stream, void* block, const Chunk& chunk, int& status);


    void clearStorage(void);
    int  allocNewBlocks(void);
    int  parseHeader(skStream* stream);
    int  parseStreamImpl(skStream* stream);
    int  crossLink(void);


    int  link(void);
    void castMembers(MemoryChunk* chunk, ftStruct* cur);
    void castMember(ftMember* dst, FBTsize*& dstPtr, ftMember* src, FBTsize*& srcPtr);

    void castMemberPointer(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr);

    void castPointer(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr);

    void castPointerToPointer(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr);


    void castMemberVariable(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr);

    ftStruct* findInTable(ftStruct* fileStruct, ftBinTables* sourceTable, ftBinTables* findInTable);

    ftStruct* findInMemoryTable(ftStruct* fileStruct);
    ftStruct* findInFileTable(ftStruct* memoryStruct);
    ftMember* findInFileTable(ftStruct* fileStruct,
                              ftMember* memoryMember,
                              bool      isPointer,
                              bool&     needCast);
};


#endif  //_ftFile_h_

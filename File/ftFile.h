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
#include "ftChunk.h"
#include "ftHashTypes.h"
#include "ftTypes.h"



class ftFile
{
public:
    enum FileMagic
    {
        FM_BIG_ENDIAN    = 0x56,
        FM_LITTLE_ENDIAN = 0x76,
        FM_32_BIT        = 0x5F,
        FM_64_BIT        = 0x2D,
        HEADER_OFFSET    = 0x0C,
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
        FS_OK,              // should always be zero
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

    enum LogFlags
    {
        LF_NONE         = 0,         // No logging
        LF_ONLY_ERR     = (1 << 0),  // Errors only (default)
        LF_READ_CHUNKS  = (1 << 1),
        LF_WRITE_CHUNKS = (1 << 2),
        LF_WRITE_LINK   = (1 << 3),
        LF_DIAGNOSTICS  = (1 << 4)
    };

    typedef skHashTable<ftPointerHashKey, ftMemoryChunk*> ChunkMap;
    typedef ftList                                        MemoryChunks;

private:
    int               m_headerFlags, m_loggerFlags;
    const char*       m_uhid;
    ftFixedString<12> m_header;
    char*             m_curFile;


protected:
    int          m_version, m_fileVersion;
    MemoryChunks m_chunks;
    ChunkMap     m_map;
    ftTables*    m_memory;
    ftTables*    m_file;
    void*        m_fileTableData;


public:
    ftFile(const char* uid);
    virtual ~ftFile();

    int load(const char* path, int mode = PM_UNCOMPRESSED);
    int load(const void* memory, FBTsize sizeInBytes, int mode = PM_UNCOMPRESSED);
    int save(const char* path, const int mode = PM_UNCOMPRESSED);

    ftTables* getMemoryTable(void);

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

    inline ftTables* getFileTable(void)
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

    inline int getLogFlags()
    {
        return m_loggerFlags;
    }

    inline void setLogFlags(int v)
    {
        m_loggerFlags = v;
    }

    inline void addLogFlag(int v)
    {
        m_loggerFlags |= v;
    }


protected:
    bool isValidWriteData(void* writeData, FBTsize len);

    int initializeTables(ftTables* tables);
    int initializeMemory(void);

    virtual bool skip(const FBThash& id)
    {
        return false;
    }

    virtual void*   getTables(void)                            = 0;
    virtual FBTsize getTableSize(void)                         = 0;
    virtual int     notifyDataRead(void* p, const ftChunk& id) = 0;
    virtual int     serializeData(skStream* stream)            = 0;

private:
    void*          findPtr(const FBTsize& iptr);
    ftMemoryChunk* findBlock(const FBTsize& iptr);
    skStream*      openStream(const char* path, int mode);

    void serializeChunk(skStream* stream,
                        FBTuint32 code,
                        FBTuint32 nr,
                        FBTuint32 typeIndex,
                        FBTsize   len,
                        void*     writeData);
    void handleChunk(skStream* stream, void* block, const ftChunk& chunk, int& status);


    void clearStorage(void);
    int  parseHeader(skStream* stream);
    int  parseStreamImpl(skStream* stream);
    int  preScan(skStream* stream);
    int  rebuildBlocks();

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
        ftMember*   dst,
        FBTsize*& dstPtr,
        ftMember*   src,
        FBTsize*& srcPtr);


    void castMemberVariable(
        ftMember*   dst,
        FBTsize*& dstPtr,
        ftMember*   src,
        FBTsize*& srcPtr);

    ftStruct* findInTable(ftStruct* fileStruct, ftTables* sourceTable, ftTables* findInTable);

    ftStruct* findInMemoryTable(ftStruct* fileStruct);
    ftStruct* findInFileTable(ftStruct* memoryStruct);
    ftMember* findInFileTable(ftStruct* fileStruct,
                              ftMember* memoryMember);
};


#endif  //_ftFile_h_

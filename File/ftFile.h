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
        FS_STATUS_MIN = -16,
        FS_LINK_FAILED,
        FS_INV_INSERT, 
        FS_BAD_ALLOC,
        FS_DUPLICATE_BLOCK,
        FS_INV_READ,
        FS_INV_LENGTH,
        FS_INV_HEADER_STR,
        FS_TABLE_INIT_FAILED,
        FS_OVERFLOW,
        FS_FAILED,

        // Table codes
        RS_INVALID_PTR,
        RS_INVALID_CODE,
        RS_LIMIT_REACHED,
        RS_BAD_ALLOC,
        RS_MIS_ALIGNED,

        // This should always be zero
        // until < 0 tests are removed
        // and replaced with != FS_OK
        FS_OK, 
    };

    enum ParseMode
    {
        PM_UNCOMPRESSED,
        PM_COMPRESSED,
        PM_READTOMEMORY,

    };

    enum FileHeader
    {
        FH_ENDIAN_SWAP = 1 << 0,
        FH_CHUNK_64    = 1 << 1,
        FH_VAR_BITS    = 1 << 2,
    };

    enum LogFlags
    {
        LF_NONE            = 0,
        LF_ONLY_ERR          = 1 << 0,
        LF_READ_CHUNKS       = 1 << 1,
        LF_WRITE_CHUNKS      = 1 << 2,
        LF_WRITE_LINK        = 1 << 3,
        LF_DIAGNOSTICS       = 1 << 4,
        LF_DO_CHECKS         = 1 << 5,
        LF_DUMP_NAME_TABLE   = 1 << 6,
        LF_DUMP_TYPE_TABLE   = 1 << 7,
        LF_DUMP_SIZE_TABLE = 1 << 8,
    };

    typedef skHashTable<ftPointerHashKey, ftMemoryChunk*> ChunkMap;
    typedef ftList                                        MemoryChunks;

private:

    int         m_headerFlags;
    int         m_fileFlags;
    const char* m_uhid;
    ftHeader    m_header;
    char*       m_curFile;
    void*       m_fileTableData;

protected:
    int          m_memoryVersion;
    int          m_fileVersion;

    MemoryChunks m_chunks;
    ChunkMap     m_map;
    ftTables*    m_memory;
    ftTables*    m_file;


public:
    ftFile(const char* uid);
    virtual ~ftFile();

    int load(const char* path, int mode = PM_UNCOMPRESSED);
    int load(const void* memory, FBTsize sizeInBytes, int mode = PM_UNCOMPRESSED);
    int save(const char* path, const int mode = PM_UNCOMPRESSED);

    ftTables* getMemoryTable(void);

    inline const ftHeader& getHeader(void) const
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



    inline int getFileFlags()
    {
        return m_fileFlags;
    }

    inline void setFileFlags(int v)
    {
        m_fileFlags = v;
    }

    inline void addFileFlag(int v)
    {
        m_fileFlags |= v;
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
                        void*     writeData,
                        bool      noOldData = false);

    void handleChunk(skStream* stream, void* block, const ftChunk& chunk, int& status);
    void insertChunk(const ftPointerHashKey& phk, ftMemoryChunk*& chunk, int& status);
    void freeChunk(ftMemoryChunk*& chunk);

    int runTableChecks(ftTables* tbltochk);

    void clearStorage(void);
    int  parseHeader(skStream* stream);
    int  parseStreamImpl(skStream* stream);
    int  preScan(skStream* stream);
    int  rebuildStructures();

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

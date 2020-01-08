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
    typedef skHashTable<ftPointerHashKey, ftMemoryChunk*> ChunkMap;
    typedef ftList                                        MemoryChunks;

private:
    int         m_headerFlags;
    int         m_fileFlags;
    const char* m_uhid;
    ftHeader    m_header;
    char*       m_curFile;
    void*       m_fileTableData;
    FBThash*    m_filterList;
    FBTint32    m_filterListLen;
    FBThash*    m_castFilter;
    FBTint32    m_castFilterLen;
    bool        m_inclusive;


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


    int load(const char* path, int mode = 0);
    int load(const void* memory, FBTsize sizeInBytes, int mode = 0);
    int save(const char* path, const int mode = 0);

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


    // This enables a filter for structures. 
    // inclusive - true:  Filter everything except what is in the list.
    // inclusive - false: Filter everything not in list.
    //
    // Internally struct types are stored by a hash of their unique type name.
    //
    // The filter list needs to be a zero terminated FBThash [] array.
    //
    // The hash function resides in the ftCharHashKey class
    // Usage:
    // FBThash filterList[] = {
    //      ftCharHashKey("StructToFilter").hash(),
    //      0
    // };
    void setFilterList(FBThash* filter, FBTuint32 length, bool inclusive = false);
    
    
    // use with dumping the results of casting
    void setCastFilter(FBThash* filter, FBTuint32 length);


    void serialize(skStream* stream, const char* id, FBTuint32 code, FBTsize len, void* writeData, int nr);
    void serialize(skStream* stream, const char* id, FBTuint32 code, FBTsize len, void* writeData);
    void serialize(skStream* stream, FBTtype index, FBTuint32 code, FBTsize len, void* writeData);
    void serialize(skStream* stream, FBTsize len, void* writeData, int nr = 1);

protected:
    bool isValidWriteData(void* writeData, FBTsize len);
    int  initializeTables(ftTables* tables);
    int  initializeMemory(void);



    virtual void*   getTables(void)                            = 0;
    virtual FBTsize getTableSize(void)                         = 0;
    virtual int     notifyDataRead(void* p, const ftChunk& id) = 0;
    virtual int     serializeData(skStream* stream)            = 0;

private:

    bool searchFilter(const FBThash* searchIn, const FBThash& searchFor, const FBTint32& len);
    void setFilter(FBThash*& dest, FBTint32& destLen, FBThash* filter, FBTint32 length);

    template <typename BaseType>
    void castPointer(FBTsize*& dstPtr, FBTsize* srcPtr, FBTsize arrayLen);

    void*          findPointer(const ftPointerHashKey& iptr);
    void*          findPointer(const FBTsize& iptr);

    ftMemoryChunk* findBlock(const FBTsize& iptr);
    skStream*      openStream(const char* path, int mode);
    bool           skip(const FBThash& id);

    void serializeChunk(skStream* stream,
                        FBTuint32 code,
                        FBTuint32 nr,
                        FBTuint32 typeIndex,
                        FBTsize   len,
                        void*     writeData);

    void handleChunk(skStream* stream, void* block, const ftChunk& chunk, int& status);
    void insertChunk(const ftPointerHashKey& phk, ftMemoryChunk*& chunk, bool addToRebuildList, int& status);
    void freeChunk(ftMemoryChunk*& chunk);

    int runTableChecks(ftTables* tbltochk);

    void clearStorage(void);

    int  parseHeader(skStream* stream);
    int  parseStreamImpl(skStream* stream);
    int  preScan(skStream* stream);
    int  rebuildStructures();
    int  allocateMBlock(const ftPointerHashKey& phk, ftMemoryChunk* bin, const FBTsize& len, bool zero);

    void castMember(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr,
        int&      status);

    void castMemberPointer(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr,
        int&      status);

    void castPointer(
        ftMember* dst,
        FBTsize*& dstPtr,
        ftMember* src,
        FBTsize*& srcPtr,
        int&      status);

    void castPointerToPointer(
        ftMember*   dst,
        FBTsize*& dstPtr,
        ftMember*   src,
        FBTsize*& srcPtr,
        int&      status);

    void castMemberVariable(
        ftMember*   dst,
        FBTsize*& dstPtr,
        ftMember*   src,
        FBTsize*& srcPtr,
        int&      status);

    void castAtomicMemberArray(
        ftMember* dst,
        FBTbyte*& dstPtr,
        ftMember* src,
        FBTbyte*& srcPtr,
        int&      status);

    void castAtomicMember(
        ftMember* dst,
        FBTbyte*& dstPtr,
        ftMember* src,
        FBTbyte*& srcPtr,
        int&      status);


    ftStruct* findInTable(ftStruct* fileStruct, ftTables* sourceTable, ftTables* findInTable);

    ftStruct* findInMemoryTable(ftStruct* fileStruct);
    ftStruct* findInFileTable(ftStruct* memoryStruct);
};


#endif  //_ftFile_h_

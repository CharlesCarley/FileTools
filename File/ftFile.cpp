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
#define FT_IN_SOURCE_FILE

#include "ftFile.h"
#include "ftAtomic.h"
#include "ftLogger.h"
#include "ftMember.h"
#include "ftPlatformHeaders.h"
#include "ftStreams.h"
#include "ftTables.h"
#include "ftEndianUtils.h"


using namespace ftEndianUtils;


const FBTuint32 ENDB      = ftID('E', 'N', 'D', 'B');
const FBTuint32 DNA1      = ftID('D', 'N', 'A', '1');
const FBTuint32 DATA      = ftID('D', 'A', 'T', 'A');
const FBTuint32 SDNA      = ftID('S', 'D', 'N', 'A');
const FBThash   DATABLOCK = ftCharHashKey("Link").hash();



ftFile::ftFile(const char* uhid) :
    m_version(-1),
    m_fileVersion(0),
    m_headerFlags(0),
    m_uhid(uhid),
    m_curFile(0),
    m_fileData(0),
    m_memory(0),
    m_file(0)
{
    m_chunks.clear();
}

ftFile::~ftFile()
{
    if (m_curFile)
    {
        ::free(m_curFile);
        m_curFile = 0;
    }

    clearStorage();
}

int ftFile::load(const char* path, int mode)
{
    skStream* stream = 0;
    if (path == 0 || !(*path))
    {
        printf("Path name must not be null\n");
        return FS_FAILED;
    }

    if (mode == PM_COMPRESSED)
    {
        skFileStream fs;
        fs.open(path, skStream::READ);

        if (fs.isOpen())
        {
            // Test the file for magic numbers ID1, ID2
            // https://www.ietf.org/rfc/rfc1952.txt ( 2.3.1.)

            unsigned char magic[3];
            fs.read(magic, 2);
            magic[2] = 0;

            if (magic[0] != 0x1F && magic[1] != 0x8B)
            {
                // Assuming it is uncompressed or any other type of file.
                mode = PM_UNCOMPRESSED;
            }
        }
    }

    stream = openStream(path, mode);
    if (!stream->isOpen())
    {
        printf("File '%s' loading failed\n", path);
        return FS_FAILED;
    }

    if (m_curFile)
        ::free(m_curFile);


    FBTsize pl = strlen(path);
    m_curFile  = (char*)::malloc(pl + 1);
    if (m_curFile)
    {
        ::memcpy(m_curFile, path, pl);
        m_curFile[pl] = 0;
    }

    int result = parseStreamImpl(stream);

    delete stream;

    return result;
}

skStream* ftFile::openStream(const char* path, int mode)
{
    skStream* stream = 0;
    if (mode == PM_UNCOMPRESSED || mode == PM_COMPRESSED)
    {
        if (mode == PM_COMPRESSED)
            stream = new ftGzStream();
        else
            stream = new skFileStream();

        stream->open(path, skStream::READ);
    }
    else
    {
        stream = new skMemoryStream();
        stream->open(path, skStream::READ);
    }

    return stream;
}


int ftFile::load(const void* memory, FBTsize sizeInBytes, int mode)
{
    skMemoryStream ms;
    ms.open(memory, sizeInBytes, skStream::READ);  //, mode == PM_COMPRESSED);

    if (!ms.isOpen())
    {
        ftLogger::logF("Memory %p(%d) loading failed!\n",
                       memory,
                       sizeInBytes);
        return FS_FAILED;
    }

    return parseStreamImpl(&ms);
}

int ftFile::parseHeader(skStream* stream)
{
    FBTbyte *hp, *magic;

    m_header.resize(12);
    m_headerFlags = 0;
    m_fileVersion = 0;

    hp = m_header.ptr();
    stream->read(hp, 12);

    if (!ftCharNEq(hp, m_uhid, 6))
    {
        return FS_INV_HEADER_STR;
    }

    magic = (hp + 7);
    if (*(magic++) == FM_64_BIT)
    {
        m_headerFlags |= FH_CHUNK_64;
        if (FT_VOID4)
            m_headerFlags |= FH_VAR_BITS;
    }
    else if (FT_VOID8)
        m_headerFlags |= FH_VAR_BITS;

    ftEndian current = getEndian();
    char     endian  = *(magic++);

    if (endian == FM_BIG_ENDIAN)
    {
        if (current == FT_ENDIAN_IS_LITTLE)
            m_headerFlags |= FH_ENDIAN_SWAP;
    }
    else if (endian == FM_LITTLE_ENDIAN)
    {
        if (current == FT_ENDIAN_IS_BIG)
            m_headerFlags |= FH_ENDIAN_SWAP;
    }


    m_fileVersion = atoi(magic);

    return FS_OK;
}

int ftFile::parseStreamImpl(skStream* stream)
{
    int     status;
    Chunk   chunk     = ftChunk::BLANK_CHUNK;
    FBTsize bytesRead = 0;

    // Ensure that any memory from a previous
    // call has been freed.
    clearStorage();

    status = parseHeader(stream);
    if (status != FS_OK)
    {
        ftLogger::log(status, "Failed to extract file header.");
        return status;
    }

    status = initializeMemory();
    if (status != FS_OK)
    {
        ftLogger::log(status, "Failed to initialize memory tables.");
        return status;
    }

    // pre allocate the pointer lookup table
    m_map.reserve(FT_DEF_ALLOC);

    while (chunk.m_code != ENDB && status == FS_OK && !stream->eof())
    {
        if ((bytesRead = ftChunk::read(&chunk, stream, m_headerFlags)) <= 0)
            status = FS_INV_READ;
        else if (chunk.m_code != ENDB)
        {
            if (chunk.m_code == SDNA)
            {
                chunk.m_code = DNA1;
                stream->seek(-((int)bytesRead), SEEK_CUR);
                chunk.m_len = (FBTuint32)(stream->size() - stream->position());
            }

            if (chunk.m_len > 0 && chunk.m_len != SK_NPOS32)
            {
                void* curPtr = ::malloc(chunk.m_len);
                if (!curPtr)
                    status = FS_BAD_ALLOC;
                else
                {
                    if (stream->read(curPtr, chunk.m_len) <= 0)
                        status = FS_INV_READ;
                    else
                    {
                        if (chunk.m_code == DNA1)
                            handleTable(stream, curPtr, chunk, status);
                        else
                            handleChunk(stream, curPtr, chunk, status);
                    }
                }
            }
        }
    }

    if (status != FS_OK)
        ftLogger::log(status, "File read failed.");
    else if (chunk.m_code != ENDB)
        ftLogger::logF("Failed to read end byte.");
    return status;
}


void ftFile::handleTable(skStream* stream, void* block, const Chunk& chunk, int& status)
{
    // Save this for later deletion. All other blocks are
    // in the MemoryChunk linked list.
    m_fileData = block;

    m_file = new ftBinTables(block, chunk.m_len, m_headerFlags & FH_CHUNK_64 ? 8 : 4);
    if (!m_file->read((m_headerFlags & FH_ENDIAN_SWAP) != 0))
        status = FS_INV_READ;
    else
    {
        crossLink();

        if (link() != FS_OK)
            status = FS_LINK_FAILED;
    }
}



void ftFile::handleChunk(skStream* stream, void* block, const Chunk& chunk, int& status)
{
    MemoryChunk* bin = (MemoryChunk*)(::malloc(sizeof(MemoryChunk)));
    if (!bin)
        status = FS_BAD_ALLOC;
    else
    {
        ::memset(bin, 0, sizeof(MemoryChunk));
        ::memcpy(&bin->m_chunk, &chunk, sizeof(Chunk));

        ftPointerHashKey phk(bin->m_chunk.m_old);

        if (m_map.insert(phk, bin))
        {
            bin->m_fblock = block;
            m_chunks.push_back(bin);
        }
        else
        {
            ftLogger::logF("print (0x%08X)", bin->m_chunk.m_old);
            free(bin);
        }
    }
}


int ftFile::crossLink(void)
{
    ftBinTables::OffsM::Iterator it = m_memory->getOffsetIterator();
    while (it.hasMoreElements())
    {
        ftStruct* mstrc   = it.getNext();
        FBThash   curHash = mstrc->getHashedType();

        if (!skip(curHash))
        {
            ftStruct* fstrc = findInFileTable(mstrc);
            if (fstrc)
            {
                mstrc->setLink(fstrc);
                fstrc->setLink(mstrc);

                ftStruct::Members::Iterator members = mstrc->getMemberIterator();
                while (members.hasMoreElements())
                {
                    bool      iptr, needsCast = false;
                    ftMember *mmbr, *fmbr;

                    mmbr = members.getNext();
                    iptr = mmbr->isPointer();

                    fmbr = findInFileTable(fstrc, mmbr, iptr, needsCast);

                    if (fmbr)
                    {
                        fmbr->setLink(mmbr);
                        mmbr->setLink(fmbr);
                    }
                }
            }
        }
    }
    return FS_OK;
}

int ftFile::link(void)
{
    int status = allocNewBlocks();
    if (status != FS_OK)
        return status;

    MemoryChunk* node;
    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        const Chunk& chunk = node->m_chunk;

        if (!m_file->isValidType(chunk.m_typeid))
        {
            ftLogger::log(chunk);
            ftLogger::logF("Chunk type id out of bounds(%d)\n",
                           chunk.m_typeid);
        }
        else if (!m_memory->isValidType(node->m_newTypeId))
        {
            ftLogger::log(chunk);
            ftLogger::logF("Memory Type id out of bounds(%d)",
                           node->m_newTypeId);
        }
        else
        {
            ftStruct* mstrc = m_memory->findStructByType(node->m_newTypeId);
            if (mstrc != nullptr)
            {
                FBThash curHash = mstrc->getHashedType();

                // filter out plain pointer types,
                // there is no need to attempt to update them.

                if (curHash != DATABLOCK)
                {
                    bool skipBlock = skip(curHash);
                    if (skipBlock || !node->m_mblock)
                    {
                        if (node->m_mblock)
                        {
                            ::free(node->m_mblock);
                            node->m_mblock = 0;
                        }
                        // else
                        // {
                        //    In which case do not
                        //    call cast members.
                        // }
                    }
                    else
                        castMembers(node, mstrc);
                }
            }
            else
                ftLogger::logF("No struct found with the type identifier(%d)", node->m_newTypeId);
        }
    }

    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_fblock)
        {
            ::free(node->m_fblock);
            node->m_fblock = 0;
        }
    }
    return ftFile::FS_OK;
}



int ftFile::allocNewBlocks(void)
{
    int          status = FS_OK;
    MemoryChunk* node   = (MemoryChunk*)m_chunks.first;
    MemoryChunk* tnode;

    while (node && status == FS_OK)
    {
        const Chunk& chunk = node->m_chunk;

        ftStruct *fileStruct, *memoryStruct;

        fileStruct   = m_file->findStructByType(chunk.m_typeid);
        memoryStruct = fileStruct->getLink();
        if (memoryStruct)
        {
            fileStruct->setLink(memoryStruct);
            memoryStruct->setLink(fileStruct);

            // link the new type id on the node
            // so it's accessible later
            node->m_newTypeId = memoryStruct->getStructIndex();

            FBThash curHash = memoryStruct->getHashedType();
            if (curHash == DATABLOCK)
            {
                node->m_mblock = ::malloc(chunk.m_len);
                if (!node->m_mblock)
                    status = FS_BAD_ALLOC;
                else
                    ::memcpy(node->m_mblock, node->m_fblock, chunk.m_len);
            }
            else if (!skip(curHash))
            {
                // Change the length of the file structure's memory
                // to account for the memory structures size.
                const FBTuint32 totSize = (chunk.m_nr * memoryStruct->getSizeInBytes());
                node->m_chunk.m_len     = totSize;
                node->m_mblock          = ::malloc(totSize);

                if (!node->m_mblock)
                    status = FS_BAD_ALLOC;
                else
                    ::memset(node->m_mblock, 0, totSize);
            }
        }

        node = node->m_next;
    }
    return status;
}



void ftFile::castMembers(MemoryChunk* node, ftStruct* mstrc)
{
    if (!node->m_mblock)
    {
        ftLogger::logF("Missing memory block.");
        return;
    }

    if (!node->m_fblock)
    {
        ftLogger::logF("Missing file block.");
        return;
    }

    FBTbyte * dst, *src;
    FBTsize * dstPtr, *srcPtr;
    FBTuint32 n;

    ftStruct*    fstrc = mstrc->getLink();
    const Chunk& chunk = node->m_chunk;


    if (fstrc)
    {
        for (n = 0; n < chunk.m_nr; ++n)
        {
            dst = mstrc->getBlock(node->m_mblock, n, chunk.m_nr);
            src = fstrc->getBlock(node->m_fblock, n, chunk.m_nr);

            ftStruct::Members::Iterator it = mstrc->getMemberIterator();
            while (it.hasMoreElements())
            {
                ftMember* dstmbr = it.getNext();
                ftMember* srcmbr = dstmbr->getLink();

                if (srcmbr)
                {
                    dstPtr = dstmbr->jumpToOffset(dst);
                    srcPtr = srcmbr->jumpToOffset(src);

                    castMember(dstmbr,
                               dstPtr,
                               srcmbr,
                               srcPtr);
                }
                else // drop member
                {
                }
            }
        }

        if (node->m_mblock)
            notifyDataRead(node->m_mblock, node->m_chunk);
    }
}

void ftFile::castMember(ftMember* mstrc,
                        FBTsize*& dstPtr,
                        ftMember* fstrc,
                        FBTsize*& srcPtr)
{
    if (mstrc->isPointer())
        castMemberPointer(mstrc, dstPtr, fstrc, srcPtr);
    else
        castMemberVariable(mstrc, dstPtr, fstrc, srcPtr);
}


void ftFile::castMemberPointer(ftMember* mstrc,
                               FBTsize*& dstPtr,
                               ftMember* fstrc,
                               FBTsize*& srcPtr)
{
    if (mstrc->getPointerCount() > 1)
        castPointerToPointer(mstrc, dstPtr, fstrc, srcPtr);
    else
        castPointer(mstrc, dstPtr, fstrc, srcPtr);
}



void ftFile::castPointerToPointer(ftMember* dst,
                                  FBTsize*& dstPtr,
                                  ftMember* src,
                                  FBTsize*& srcPtr)
{
    MemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
    if (bin)
    {
        if (bin->m_flag & MemoryChunk::BLK_MODIFIED)
            (*dstPtr) = (FBTsize)bin->m_mblock;
        else
        {
            FBTsize fps = m_file->getSizeofPointer();
            if (fps == 4 || fps == 8)
            {
                FBTsize i;
                FBTsize total  = bin->m_chunk.m_len / fps;
                FBTsize mptrsz = total * sizeof(FBTsize);

                FBTsize* nptr = (FBTsize*)::calloc(total, sizeof(FBTuintPtr));

                if (nptr == nullptr)
                {
                    ftLogger::log(FS_BAD_ALLOC,
                                  "Failed to allocate new block");
                    return;
                }
                ::memset(nptr, 0, mptrsz);

                // Always use a 32 bit integer,
                // then offset + 2 for a 64 bit pointer.
                FBTuint32* optr = (FBTuint32*)bin->m_fblock;

                for (i = 0; i < total; i++,
                    optr += (fps == 4 ? 1 : 2))
                    nptr[i] = (FBTsize)findPtr((FBTsize)*optr);

                (*dstPtr) = (FBTsize)(nptr);

                bin->m_chunk.m_len = mptrsz;
                bin->m_flag |= MemoryChunk::BLK_MODIFIED;

                ::free(bin->m_mblock);
                bin->m_mblock = nptr;
            }
            else
            {
                ftLogger::logF("Unknown pointer length(%d). Pointers should be either 4 or 8 bytes", fps);
            }
        }
    }
    else
    {
        ftLogger::logF("Failed to find corresponding chunk for address (0x%08X)",
                       (FBTsize)(*srcPtr));

        ftLogger::logF("Source");
        ftLogger::log(src);

        ftLogger::logF("Destination");
        ftLogger::log(dst);

    }
}


void ftFile::castPointer(ftMember* mstrc,
                         FBTsize*& dstPtr,
                         ftMember* fstrc,
                         FBTsize*& srcPtr)
{
    int arrayLen = skMin(mstrc->getArraySize(), fstrc->getArraySize());

    FBTsize fps = m_file->getSizeofPointer();
    if (fps == 4 || fps == 8)
    {
        int      i;
        FBTsize* dptr = (FBTsize*)dstPtr;

        // Always use a 32 bit integer,
        // then offset + 2 for a 64 bit pointer.
        FBTuint32* sptr = (FBTuint32*)srcPtr;

        for (i = 0; i < arrayLen; ++i,
            sptr += (fps == 4 ? 1 : 2))
            dptr[i] = (FBTsize)findPtr((FBTsize)(*sptr));
    }
    else
        ftLogger::logF("Invalid size of pointer.");
}

void ftFile::castMemberVariable(ftMember* dst,
                                FBTsize*& dstPtr,
                                ftMember* src,
                                FBTsize*& srcPtr)
{
    FBTsize fps = m_file->getSizeofPointer();
    FBTsize mps = m_memory->getSizeofPointer();

    FBTsize dstElmSize = dst->getArrayElementSize();
    FBTsize srcElmSize = src->getArrayElementSize();

    bool endianSwap = (m_headerFlags & FH_ENDIAN_SWAP) != 0;
    bool needCast   = false;  //dst->hasFlag(ftStruct::NEED_CAST);

    bool needSwap = endianSwap && srcElmSize > 1;

    FBThash dhash = dst->getTypeName();
    FBThash shash = src->getTypeName();

    if (!needCast && !needSwap && dhash == shash)
        ::memcpy(dstPtr, srcPtr, skMin(srcElmSize, dstElmSize));
    else
    {
        FBTbyte* dstBPtr = reinterpret_cast<FBTbyte*>(dstPtr);
        FBTbyte* srcBPtr = reinterpret_cast<FBTbyte*>(srcPtr);

        ftAtomic stp = ftAtomic::FT_ATOMIC_UNKNOWN, dtp = ftAtomic::FT_ATOMIC_UNKNOWN;
        if (needCast || needSwap)
        {
            stp = src->getAtomicType();
            dtp = src->getAtomicType();
        }

        FBTsize alen = skMin(dst->getArraySize(), src->getArraySize());
        FBTsize elen = skMin(srcElmSize, dstElmSize);

        FBTbyte tmpBuf[8] = {};

        FBTsize i;
        for (i = 0; i < alen; i++)
        {
            FBTbyte* tmp = srcBPtr;
            if (needSwap)
            {
                tmp = tmpBuf;
                ::memcpy(tmpBuf, srcBPtr, srcElmSize);

                if (stp == ftAtomic::FT_ATOMIC_SHORT || stp == ftAtomic::FT_ATOMIC_USHORT)
                    swap16((FBTuint16*)tmpBuf, 1);
                else if (stp >= ftAtomic::FT_ATOMIC_INT && stp <= ftAtomic::FT_ATOMIC_FLOAT)
                    swap32((FBTuint32*)tmpBuf, 1);
                else if (stp == ftAtomic::FT_ATOMIC_DOUBLE)
                    swap64((FBTuint64*)tmpBuf, 1);
                else
                    ::memset(tmpBuf, 0, sizeof(tmpBuf));  //unknown type
            }

            if (needCast)
                ftAtomicUtils::cast((char*)tmp, (char*)dstBPtr, stp, dtp, 1);
            else
                ::memcpy(dstBPtr, tmp, elen);

            dstBPtr += dstElmSize;
            srcBPtr += srcElmSize;
        }
    }
}


ftStruct* ftFile::findInTable(ftStruct* findStruct, ftBinTables* sourceTable, ftBinTables* findInTable)
{
    ftStruct* fstrc = nullptr;

    if (findStruct != nullptr)
    {
        FBTuint16 name;
        FBTbyte*  searchKey;
        name      = findStruct->getTypeIndex();
        searchKey = sourceTable->getTypeNameAt(name);
        if (searchKey != nullptr)
            fstrc = findInTable->findStructByName(searchKey);
    }
    return fstrc;
}

ftStruct* ftFile::findInMemoryTable(ftStruct* fileStruct)
{
    return findInTable(fileStruct, m_file, m_memory);
}

ftStruct* ftFile::findInFileTable(ftStruct* memoryStruct)
{
    return findInTable(memoryStruct, m_memory, m_file);
}

ftMember* ftFile::findInFileTable(ftStruct* fileStruct,
                                  ftMember* memoryMember,
                                  bool      isPointer,
                                  bool&     needCast)
{
    ftStruct::Members::Iterator fmit  = fileStruct->getMemberIterator();
    ftMember*                   fstrc = 0;

    while (!fstrc && fmit.hasMoreElements())
    {
        ftMember* fmbr = fmit.getNext();
        if (fmbr->compare(memoryMember))
            fstrc = fmbr;
    }
    return fstrc;
}

void* ftFile::findPtr(const FBTsize& iptr)
{
    FBTsizeType i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i)->m_mblock;
    return 0;
}


MemoryChunk* ftFile::findBlock(const FBTsize& iptr)
{
    FBTsize i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i);
    return nullptr;
}

ftBinTables* ftFile::getMemoryTable(void)
{
    if (!m_memory)
        initializeMemory();
    return m_memory;
}

int ftFile::initializeMemory(void)
{
    int status = m_memory == 0 ? (int)FS_FAILED : (int)FS_OK;
    if (!m_memory)
    {
        m_memory = new ftBinTables();

        status = initializeTables(m_memory);
        if (status != FS_OK)
            ftLogger::log(status);
    }
    return status;
}



int ftFile::initializeTables(ftBinTables* tables)
{
    // Call to the derived classes
    // to access per class table data.

    void*   tableData = getTables();
    FBTsize tableSize = getTableSize();

    if (tableData != 0 && tableSize > 0 && tableSize != SK_NPOS)
        return tables->read(tableData, tableSize, false) ? (int)FS_OK : (int)FS_FAILED;

    return FS_FAILED;
}


void ftFile::clearStorage(void)
{
    if (m_fileData)
    {
        ::free(m_fileData);
        m_fileData = 0;
    }

    MemoryChunk* node = (MemoryChunk*)m_chunks.first;
    MemoryChunk* tmp;
    while (node)
    {
        if (node->m_fblock)
        {
            ::free(node->m_fblock);
            node->m_fblock = 0;
        }
        if (node->m_mblock)
        {
            ::free(node->m_mblock);
            node->m_mblock = 0;
        }
        tmp  = node;
        node = node->m_next;

        ::free(tmp);
    }

    if (m_file)
    {
        delete m_file;
        m_file = 0;
    }

    if (m_fileData)
    {
        free(m_fileData);
        m_fileData = 0;
    }

    if (m_memory)
    {
        delete m_memory;
        m_memory = 0;
    }
}

int ftFile::save(const char* path, const int mode)
{
    skStream* fs;

    if (mode == PM_COMPRESSED)
        fs = new ftGzStream();
    else
        fs = new skFileStream();

    fs->open(path, skStream::WRITE);

    FBTuint8 cp = FT_VOID8 ? FM_64_BIT : FM_32_BIT;
    FBTuint8 ce = getEndian();
    if (ce == FT_ENDIAN_IS_BIG)
        ce = FM_BIG_ENDIAN;
    else
        ce = FM_LITTLE_ENDIAN;
    
    char header[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char version[33];
    sprintf(version, "%i", m_version);

    strncpy(&header[0], m_uhid, 7);   // 7 first bytes of header
    header[7] = cp;                   // 8th byte = pointer size
    header[8] = ce;                   // 9th byte = endian
    strncpy(&header[9], version, 3);  // last 3 bytes v or 3 version char
    fs->write(header, 12);

    serializeData(fs);

    // write DNA1
    Chunk ch;
    ch.m_code   = DNA1;
    ch.m_len    = getTableSize();
    ch.m_nr     = 1;
    ch.m_old    = 0;
    ch.m_typeid = 0;
    fs->write(&ch, ftChunk::BlockSize);
    fs->write(getTables(), ch.m_len);

    // write ENDB (End Byte | EOF )
    ch.m_code   = ENDB;
    ch.m_len    = 0;
    ch.m_nr     = 0;
    ch.m_old    = 0;
    ch.m_typeid = 0;
    fs->write(&ch, ftChunk::BlockSize);

    delete fs;
    return FS_OK;
}

void ftFile::serialize(skStream*   stream,
                       const char* id,
                       FBTuint32   code,
                       FBTsize     len,
                       void*       writeData)
{
    if (m_memory == 0)
        getMemoryTable();

    FBTuint32 ft = m_memory->findTypeId(id);
    if (ft == SK_NPOS32)
    {
        printf("writeStruct: %s - not found", id);
        return;
    }
    serialize(stream, ft, code, len, writeData);
}

bool ftFile::isValidWriteData(void* writeData, FBTsize len)
{
    if (writeData == nullptr || len == SK_NPOS || len == 0)
    {
        printf("Invalid write data\n");
        return false;
    }
    return true;
}

void ftFile::serialize(skStream* stream,
                       FBTtype   index,
                       FBTuint32 code,
                       FBTsize   len,
                       void*     writeData)
{
    if (isValidWriteData(writeData, len))
    {
        Chunk ch;
        ch.m_code   = code;
        ch.m_len    = len;
        ch.m_nr     = 1;
        ch.m_old    = (FBTsize)writeData;
        ch.m_typeid = index;
        ftChunk::write(&ch, stream);
    }
}

void ftFile::serialize(skStream* stream, FBTsize len, void* writeData, int nr)
{
    if (m_memory == 0)
        getMemoryTable();

    if (isValidWriteData(writeData, len))
    {
        Chunk ch;
        ch.m_code   = DATA;
        ch.m_len    = len;
        ch.m_nr     = nr;
        ch.m_old    = (FBTsize)writeData;
        ch.m_typeid = 0;
        ftChunk::write(&ch, stream);
    }
}

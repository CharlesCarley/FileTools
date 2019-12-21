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
#include "ftEndianUtils.h"
#include "ftLogger.h"
#include "ftMember.h"
#include "ftPlatformHeaders.h"
#include "ftStreams.h"
#include "ftTables.h"


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
    m_loggerFlags(LF_ONLY_ERR),
    m_uhid(uhid),
    m_curFile(0),
    m_memory(0),
    m_file(0),
    m_fileTableData(0)
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
        if (m_loggerFlags != LF_NONE)
            ftLogger::logF("Path name must not be null.");
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
        if (m_loggerFlags != LF_NONE)
            ftLogger::logF("File '%s' loading failed.", path);
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
        if (m_loggerFlags != LF_NONE)
        {
            ftLogger::logF("Memory %p(%d) loading failed!\n",
                           memory,
                           sizeInBytes);
        }
        return FS_FAILED;
    }

    return parseStreamImpl(&ms);
}

int ftFile::parseHeader(skStream* stream)
{
    FBTbyte *hp, *magic;

    m_headerFlags = 0;
    m_fileVersion = 0;

    hp = m_header.ptr();
    stream->read(hp, 12);
    m_header.resize(12);

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


int ftFile::preScan(skStream* stream)
{
    // The idea of the pre-scan is to jump to the
    // DNA1 block first, extract the table data,
    // then seek back to the file header offset
    // and read the chunks up to the DNA1 block.
    // Then, create associations with the structure
    // and member declarations along with every chunk.

    int         status = FS_OK;
    FBTsize     bytesRead;
    ftChunkScan scan = {0, 0};

    while (scan.m_code != ENDB &&
           scan.m_code != DNA1 &&
           status == FS_OK && !stream->eof())
    {
        if ((bytesRead = ftChunkUtils::scan(&scan, stream, m_headerFlags)) <= 0)
            status = FS_INV_READ;
        else if (scan.m_code != ENDB)
        {
            if (scan.m_code == DNA1)
            {
                // This block needs to stay alive as long as m_file is valid.
                // The names of the types and the names of the type-name
                // declarations are referenced out of this block.
                m_fileTableData = ::malloc(scan.m_len);

                if (!m_fileTableData)
                    status = FS_BAD_ALLOC;
                else
                {
                    if (stream->read(m_fileTableData, scan.m_len) <= 0)
                        status = FS_INV_READ;
                    else
                    {
                        m_file = new ftTables();
                        if (!m_file->read(m_fileTableData, scan.m_len, m_headerFlags))
                            status = FS_TABLE_INIT_FAILED;
                    }
                }
            }
            else
            {
                if (scan.m_len > 0 && scan.m_len != SK_NPOS32)
                {
                    if (!stream->seek(scan.m_len, SEEK_CUR))
                        status = FS_INV_READ;
                }
            }
        }
    }
    return status;
}


int ftFile::parseStreamImpl(skStream* stream)
{
    int     status;
    ftChunk chunk     = ftChunkUtils::BLANK_CHUNK;
    FBTsize bytesRead = 0;

    // Ensure that any memory from a previous
    // call has been freed.
    clearStorage();

    status = parseHeader(stream);
    if (status != FS_OK)
    {
        if (m_loggerFlags != LF_NONE)
            ftLogger::log(status, "Failed to extract the file header.");
        return status;
    }

    status = initializeMemory();
    if (status != FS_OK)
    {
        if (m_loggerFlags != LF_NONE)
            ftLogger::log(status, "Failed to initialize the memory tables.");
        return status;
    }

    status = preScan(stream);
    if (status != FS_OK)
    {
        if (m_loggerFlags != LF_NONE)
            ftLogger::log(status, "Failed to pre-scan the file.");
        return status;
    }

    if (!stream->seek(HEADER_OFFSET, SEEK_SET))
    {
        status = FS_INV_READ;
        ftLogger::log(status, "Failed to seek back to the header.");
    }


    // pre allocate the pointer lookup table
    m_map.reserve(FT_DEF_ALLOC);

    while (chunk.m_code != ENDB &&
           chunk.m_code != DNA1 &&
           status == FS_OK && !stream->eof())
    {
        if ((bytesRead = ftChunkUtils::read(&chunk, stream, m_headerFlags)) <= 0)
            status = FS_INV_READ;
        else if (chunk.m_code != ENDB && chunk.m_code != DNA1)
        {
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
                        handleChunk(stream, curPtr, chunk, status);
                }

                if (m_loggerFlags & LF_READ_CHUNKS)
                {
                    ftLogger::log(chunk);
                    ftLogger::log(curPtr, chunk.m_len);
                }
            }
            else
                status = FS_INV_LENGTH;
        }
    }

    if (status == FS_OK)
        status = rebuildStructures();

    if (m_loggerFlags != LF_NONE)
    {
        if (status != FS_OK)
            ftLogger::log(status, "File read failed.");
        else if (chunk.m_code != DNA1)
            ftLogger::logF("Failed to reach the end byte.");
    }
    return status;
}



void ftFile::handleChunk(skStream* stream, void* block, const ftChunk& chunk, int& status)
{
    ftMemoryChunk* bin = (ftMemoryChunk*)(::malloc(sizeof(ftMemoryChunk)));
    if (bin)
    {
        ::memset(bin, 0, sizeof(ftMemoryChunk));
        ::memcpy(&bin->m_chunk, &chunk, sizeof(ftChunk));

        ftPointerHashKey phk(chunk.m_old);
        if (m_map.find(phk) != m_map.npos)
        {
            if (m_loggerFlags & LF_DIAGNOSTICS)
                ftLogger::logF("print (0x%08X)", bin->m_chunk.m_old);
            free(bin);
            status = FS_DUPLICATE_BLOCK;
        }
        else if (!m_map.insert(phk, bin))
        {
            if (m_loggerFlags & LF_DIAGNOSTICS)
                ftLogger::logF("print (0x%08X)", bin->m_chunk.m_old);
            free(bin);

            status = FS_INV_INSERT;
        }
        else
        {
            bin->m_fblock = block;

            if (chunk.m_code == DATA || chunk.m_typeid == 0)
            {
                bin->m_mblock = ::malloc(chunk.m_len);
                if (!bin->m_mblock)
                    status = FS_BAD_ALLOC;
                /*
                else
                    ::memcpy(bin->m_mblock, bin->m_fblock, chunk.m_len);
                */
                memset(bin->m_mblock, 0, chunk.m_len);
            }
            else
            {
                bin->m_fstrc = m_file->findStructByType(bin->m_chunk.m_typeid);
                if (bin->m_fstrc)
                    bin->m_mstrc = findInMemoryTable(bin->m_fstrc);

                if (bin->m_fstrc && bin->m_mstrc)
                {
                    bin->m_newTypeId = bin->m_mstrc->getStructIndex();

                    if (!skip(bin->m_fstrc->getHashedType()))
                    {
                        // Change the length of the file structure's memory
                        // to account for the memory structures size.
                        const FBTuint32 totSize = (chunk.m_nr * bin->m_mstrc->getSizeInBytes());
                        bin->m_chunk.m_len      = totSize;
                        bin->m_mblock           = ::malloc(totSize);
                        if (!bin->m_mblock)
                            status = FS_BAD_ALLOC;
                        else
                            ::memset(bin->m_mblock, 0, totSize);

                        if (status == FS_OK)
                            m_chunks.push_back(bin);
                    }
                }
                else
                {
                    if (m_loggerFlags & LF_DIAGNOSTICS)
                    {
                        ftLogger::seperator();
                        ftLogger::logF("Failed to resolve both file and memory declarations for chunk:");
                        ftLogger::log(bin->m_chunk);
                        ftLogger::logF("File   : %s", bin->m_fstrc ? "Valid" : "Invalid");
                        ftLogger::logF("Memory : %s", bin->m_mstrc ? "Valid" : "Invalid");
                        ftLogger::newline(2);
                    }

                    free(bin);
                }
            }
        }
    }
    else
        status = FS_BAD_ALLOC;
}



int ftFile::rebuildStructures()
{
    FBTuint32 n;

    int status = FS_OK;

    FBTbyte * src, *dst;
    FBTsize * srcPtr, *dstPtr;
    ftStruct *fstrc, *mstrc;

    bool diagnostics = (m_loggerFlags & LF_DIAGNOSTICS) != 0;



    ftMemoryChunk* node;
    for (node = (ftMemoryChunk*)m_chunks.first; node && status == FS_OK; node = node->m_next)
    {
        const ftChunk& chunk = node->m_chunk;
        if (m_loggerFlags & LF_WRITE_LINK)
        {
            ftLogger::log(chunk);
            if (node->m_fstrc)
                ftLogger::log(node->m_fstrc);

            if (node->m_mstrc)
                ftLogger::log(node->m_mstrc);
        }

        fstrc = node->m_fstrc;
        mstrc = node->m_mstrc;

        if (diagnostics)
        {
            ftLogger::logF("casting struct %s -> %s",
                           fstrc->getName(),
                           mstrc->getName());
            ftLogger::newline();
            ftLogger::log(fstrc, mstrc);
        }

        for (n = 0; n < chunk.m_nr && status == FS_OK; ++n)
        {
            dst = mstrc->getBlock(node->m_mblock, n, chunk.m_nr);
            src = fstrc->getBlock(node->m_fblock, n, chunk.m_nr);

            ftStruct::Members::Iterator it = mstrc->getMemberIterator();
            while (it.hasMoreElements())
            {
                ftMember* dstmbr = it.getNext();
                ftMember* srcmbr = findInFileTable(fstrc, dstmbr);

                if (srcmbr)
                {
                    dstPtr = dstmbr->jumpToOffset(dst);
                    srcPtr = srcmbr->jumpToOffset(src);

                    castMember(dstmbr,
                               dstPtr,
                               srcmbr,
                               srcPtr);

                    if (diagnostics)
                    {
                        ftLogger::seperator();
                        ftLogger::width(25);
                        ftLogger::logF("%s %s ==> %s %s",
                                       srcmbr->getType(),
                                       srcmbr->getName(),
                                       dstmbr->getType(),
                                       dstmbr->getName());
                        ftLogger::log(srcPtr, srcmbr->getSizeInBytes());
                        ftLogger::newline();
                        ftLogger::log(dstPtr, dstmbr->getSizeInBytes());
                        ftLogger::newline();
                    }
                }
                else
                {
                    // The memory in destination pointer is already zeroed, and
                    // the data at that offset should already be initialized.
                    // But if it's not it should be zeroed, and its probably
                    // a bug somewhere because something overflowed into it.
                    dstPtr = dstmbr->jumpToOffset(dst);

                    void* test = malloc(dstmbr->getSizeInBytes());
                    memset(test, 0, dstmbr->getSizeInBytes());
                    if (::memcmp(dstPtr, test, dstmbr->getSizeInBytes()) != 0)
                        status = FS_OVERFLOW;
                    else
                    {
                        if (diagnostics)
                        {
                            ftLogger::seperator();
                            ftLogger::logF("MISSING %s %s",
                                           dstmbr->getType(),
                                           dstmbr->getName());
                            ftLogger::divider();
                            ftLogger::log(dstPtr, dstmbr->getSizeInBytes());
                            ftLogger::newline();
                        }
                    }
                    free(test);
                }
            }
        }

        if (node->m_mblock)
            notifyDataRead(node->m_mblock, node->m_chunk);
    }

    return status;
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
    ftMemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
    if (bin)
    {
        if (bin->m_flag & ftMemoryChunk::BLK_MODIFIED)
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
                    if (m_loggerFlags != LF_NONE)
                    {
                        ftLogger::log(FS_BAD_ALLOC,
                                      "Failed to allocate new block");
                    }
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
                bin->m_flag |= ftMemoryChunk::BLK_MODIFIED;

                ::free(bin->m_mblock);
                bin->m_mblock = nptr;
            }
            else if (m_loggerFlags != LF_NONE)
                ftLogger::logF("Unknown pointer length(%d). Pointers should be either 4 or 8 bytes", fps);
        }
    }
    else if (m_loggerFlags != LF_NONE)
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
        int        i;
        FBTuint32* sptr = (FBTuint32*)srcPtr;
        for (i = 0; i < arrayLen; ++i, sptr += (fps == 4 ? 1 : 2))
            dstPtr[i] = (FBTsize)findPtr((FBTsize)(*sptr));
    }
    else if (m_loggerFlags != LF_NONE)
        ftLogger::logF("Invalid size of pointer.");
}


void ftFile::castMemberVariable(ftMember* dst,
                                FBTsize*& dstPtr,
                                ftMember* src,
                                FBTsize*& srcPtr)
{
    FBTsize dstElmSize = dst->getSizeInBytes();
    FBTsize srcElmSize = src->getSizeInBytes();

    bool endianSwap = (m_headerFlags & FH_ENDIAN_SWAP) != 0;
    bool needCast   = false;  //dst->hasFlag(ftStruct::NEED_CAST);
    bool needSwap   = endianSwap && srcElmSize > 1;

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


ftStruct* ftFile::findInTable(ftStruct* findStruct, ftTables* sourceTable, ftTables* findInTable)
{
    ftStruct* fstrc = nullptr;

    if (findStruct != nullptr)
    {
        FBTuint16 strcType;
        FBTbyte*  searchKey;
        strcType  = findStruct->getTypeIndex();
        searchKey = sourceTable->getTypeNameAt(strcType);
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
                                  ftMember* memoryMember)
{
    ftStruct::Members::Iterator fmit  = fileStruct->getMemberIterator();
    ftMember*                   fstrc = 0;

    while (!fstrc && fmit.hasMoreElements())
    {
        ftMember* fmbr = fmit.getNext();
        if (memoryMember->compare(fmbr))
            fstrc = fmbr;
    }
    return fstrc;
}

void* ftFile::findPtr(const FBTsize& iptr)
{
    FBTsize i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i)->m_mblock;
    return 0;
}


ftMemoryChunk* ftFile::findBlock(const FBTsize& iptr)
{
    FBTsize i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i);
    return nullptr;
}

ftTables* ftFile::getMemoryTable(void)
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
        m_memory = new ftTables();

        status = initializeTables(m_memory);
        if (status != FS_OK)
            ftLogger::log(status);
    }
    return status;
}



int ftFile::initializeTables(ftTables* tables)
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
    ftMemoryChunk* node = (ftMemoryChunk*)m_chunks.first;
    ftMemoryChunk* tmp;
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


    if (m_fileTableData)
    {
        free(m_fileTableData);
        m_fileTableData = 0;
    }

    if (m_file)
    {
        delete m_file;
        m_file = 0;
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
    ftChunk ch;
    ch.m_code   = DNA1;
    ch.m_len    = getTableSize();
    ch.m_nr     = 1;
    ch.m_old    = 0;
    ch.m_typeid = 0;
    fs->write(&ch, ftChunkUtils::BlockSize);
    fs->write(getTables(), ch.m_len);

    // write ENDB (End Byte | EOF )
    ch.m_code   = ENDB;
    ch.m_len    = 0;
    ch.m_nr     = 0;
    ch.m_old    = 0;
    ch.m_typeid = 0;
    fs->write(&ch, ftChunkUtils::BlockSize);

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
        if (m_loggerFlags != LF_NONE)
            ftLogger::logF("writeStruct: %s - not found", id);
        return;
    }
    serialize(stream, ft, code, len, writeData);
}


bool ftFile::isValidWriteData(void* writeData, FBTsize len)
{
    if (writeData == nullptr || len == SK_NPOS || len == 0)
    {
        if (m_loggerFlags != LF_NONE)
            ftLogger::logF("Invalid write data\n");
        return false;
    }
    return true;
}


void ftFile::serializeChunk(skStream* stream,
                            FBTuint32 code,
                            FBTuint32 nr,
                            FBTuint32 typeIndex,
                            FBTsize   len,
                            void*     writeData)
{
    if (isValidWriteData(writeData, len))
    {
        ftChunk ch;
        ch.m_code   = code;
        ch.m_len    = len;
        ch.m_nr     = 1;
        ch.m_old    = (FBTsize)writeData;
        ch.m_typeid = typeIndex;
        ftChunkUtils::write(&ch, stream);

        if (m_loggerFlags & LF_WRITE_CHUNKS)
        {
            ftLogger::log(ch);
            ftLogger::log(writeData, len);
        }
    }
}

void ftFile::serialize(skStream* stream,
                       FBTtype   index,
                       FBTuint32 code,
                       FBTsize   len,
                       void*     writeData)
{
    serializeChunk(stream, code, 1, index, len, writeData);
}

void ftFile::serialize(skStream* stream, FBTsize len, void* writeData, int nr)
{
    serializeChunk(stream, DATA, 1, 0, len, writeData);
}

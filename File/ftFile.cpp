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
#include "ftScanDNA.h"
#include "ftStreams.h"
#include "ftTables.h"

using namespace ftEndianUtils;
using namespace ftFlags;



ftFile::ftFile(const char* uhid) :
    m_memoryVersion(-1),
    m_fileVersion(0),
    m_headerFlags(0),
    m_fileFlags(LF_ONLY_ERR),
    m_uhid(uhid),
    m_curFile(0),
    m_memory(0),
    m_file(0),
    m_filterList(0),
    m_filterListLen(0),
    m_castFilter(0),
    m_castFilterLen(0),
    m_inclusive(false),
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
        if (m_fileFlags != LF_NONE)
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
        if (m_fileFlags != LF_NONE)
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
    }
    else
        stream = new skMemoryStream();

    stream->open(path, skStream::READ);
    return stream;
}


int ftFile::load(const void* memory, FBTsize sizeInBytes, int mode)
{
    // Fix this, perhaps move zlib to utils
    // if the gzStream ever materializes..
    // and if there is really every enough use
    skMemoryStream ms;
    ms.open(memory, sizeInBytes, skStream::READ);  //, mode == PM_COMPRESSED);

    if (!ms.isOpen())
    {
        if (m_fileFlags != LF_NONE)
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
    stream->read(hp, HEADER_OFFSET);
    m_header.resize(HEADER_OFFSET);

    if (!ftCharNEq(hp, m_uhid, 7))
        return FS_INV_HEADER_STR;

    magic = (hp + 7);
    if (*(magic++) == FM_64_BIT)
    {
        m_headerFlags |= FH_CHUNK_64;
        if (FT_VOID4)
            m_headerFlags |= FH_VAR_BITS;
    }
    else if (FT_VOID8)
        m_headerFlags |= FH_VAR_BITS;

    int  current = (int)getEndian();
    char endian  = *(magic++);

    if (endian == FM_BIG_ENDIAN)
    {
        if (current == ftEndian::FT_ENDIAN_IS_LITTLE)
            m_headerFlags |= FH_ENDIAN_SWAP;
    }
    else if (endian == FM_LITTLE_ENDIAN)
    {
        if (current == ftEndian::FT_ENDIAN_IS_BIG)
            m_headerFlags |= FH_ENDIAN_SWAP;
    }

    m_fileVersion = atoi(magic);
    return FS_OK;
}


int ftFile::preScan(skStream* stream)
{
    int status = FS_OK;

    ftScanDNA scanner;
    scanner.setFlags(m_headerFlags);

    status = scanner.scan(stream);
    if (status == FS_OK)
    {
        m_fileTableData = scanner.getDNA();
        if (m_fileTableData && scanner.getLength() > 0)
        {
            m_file = new ftTables((m_headerFlags & FH_CHUNK_64) != 0 ? 8 : 4);
            status = m_file->read(m_fileTableData, scanner.getLength(), m_headerFlags, m_fileFlags);
            if (status == FS_OK)
            {
                if (m_fileFlags & LF_DO_CHECKS)
                    status = runTableChecks(m_file);
            }
            else
            {
                if (m_fileFlags != LF_NONE)
                    ftLogger::logF("File table initialization failed.");
            }
        }
    }

    return status;
}



int ftFile::runTableChecks(ftTables* tbltochk)
{
    if (tbltochk)
    {
        if (!tbltochk->testDuplicateKeys())
        {
            if (m_fileFlags != LF_NONE)
                ftLogger::logF("There are duplicate names in the table.");

            return FS_FAILED;
        }
    }
    return FS_OK;
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
        if (m_fileFlags != LF_NONE)
            ftLogger::log(status, "Failed to extract the file header.");
        return status;
    }

    status = initializeMemory();
    if (status != FS_OK)
    {
        if (m_fileFlags != LF_NONE)
            ftLogger::log(status, "Failed to initialize the memory tables.");
        return status;
    }

    status = preScan(stream);
    if (status != FS_OK)
    {
        if (m_fileFlags != LF_NONE)
            ftLogger::log(status, "Failed to pre-scan the file.");
        return status;
    }

    if (!stream->seek(HEADER_OFFSET, SEEK_SET))
    {
        status = FS_INV_READ;
        ftLogger::log(status, "Failed to seek back to the header.");
    }

    m_map.reserve(FT_DEF_ALLOC);


    while (chunk.m_code != ftIdNames::ENDB &&
           chunk.m_code != ftIdNames::DNA1 &&
           status == FS_OK && !stream->eof())
    {
        if ((bytesRead = ftChunkUtils::read(&chunk, stream, m_headerFlags)) <= 0)
            status = FS_INV_READ;
        else if (chunk.m_code == ftIdNames::TEST)
        {
            if (!stream->seek(chunk.m_len, SEEK_CUR))
            {
                if (m_fileFlags != LF_NONE)
                    ftLogger::logF("Failed to skip over a TEST chunk.");
                status = FS_INV_READ;
            }
        }
        else if (chunk.m_code != ftIdNames::ENDB && chunk.m_code != ftIdNames::DNA1)
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
            }
            else
                status = FS_INV_LENGTH;
        }
    }
    if (status == FS_OK)
        status = rebuildStructures();

    if (m_fileFlags != LF_NONE)
    {
        if (status != FS_OK)
            ftLogger::log(status, "File read failed.");
        else if (chunk.m_code != ftIdNames::DNA1)
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


        // This is saved here to recalculate the total
        // number of elements in a pointer array.
        bin->m_pblockLen = chunk.m_len;

        ftPointerHashKey phk(chunk.m_addr);
        if (m_map.find(phk) != m_map.npos)
            freeChunk(bin);
        else
        {
            bin->m_fblock = block;
            ftStruct *fstrc, *mstrc = nullptr;


            if (bin->m_chunk.m_code == ftIdNames::DATA && bin->m_chunk.m_structId <= m_file->getFirstStructType())
            {
                const FBTuint32 totSize = bin->m_chunk.m_len;
                if (totSize > 0 && totSize != SK_NPOS32)
                {
                    bin->m_chunk.m_len = totSize;
                    bin->m_mblock      = ::malloc(totSize);
                    if (!bin->m_mblock)
                        status = FS_BAD_ALLOC;
                    else
                        ::memcpy(bin->m_mblock, bin->m_fblock, totSize);

                    if (status == FS_OK)
                    {
                        insertChunk(phk, bin, status);

                        if (m_fileFlags & LF_READ_CHUNKS)
                            ftLogger::logReadChunk(chunk, bin->m_fblock, chunk.m_len);
                    }
                }
                else
                    status = FS_BAD_ALLOC;
            }
            else
            {
                fstrc = m_file->findStructByType(bin->m_chunk.m_structId);
                if (fstrc)
                    mstrc = findInMemoryTable(fstrc);

                if (fstrc && mstrc)
                {
                    bin->m_fstrc     = fstrc;
                    bin->m_mstrc     = mstrc;
                    bin->m_newTypeId = bin->m_mstrc->getStructIndex();

                    if (!skip(fstrc->getHashedType()))
                    {
                        // Change the length of the file structure's memory
                        // to account for the memory structures size.
                        const FBTuint32 totSize = (chunk.m_nr * mstrc->getSizeInBytes());
                        if (totSize > 0 && totSize != SK_NPOS32)
                        {
                            bin->m_chunk.m_len = totSize;
                            bin->m_mblock      = ::malloc(totSize);
                            if (!bin->m_mblock)
                                status = FS_BAD_ALLOC;
                            else
                                ::memset(bin->m_mblock, 0, totSize);

                            if (status == FS_OK)
                            {
                                insertChunk(phk, bin, status);

                                if (m_fileFlags & LF_READ_CHUNKS)
                                    ftLogger::logReadChunk(chunk, bin->m_fblock, chunk.m_len);
                            }
                        }
                        else
                            status = FS_BAD_ALLOC;
                    }
                    else
                    {
                        if (m_fileFlags & LF_DIAGNOSTICS && m_fileFlags & LF_DUMP_SKIP)
                            ftLogger::logSkipChunk(bin->m_chunk, fstrc, bin->m_fblock, bin->m_chunk.m_len);
                        freeChunk(bin);
                    }
                }
                else
                {
                    if (m_fileFlags & LF_DIAGNOSTICS && m_fileFlags & LF_UNRESOLVED)
                    {
                        ftLogger::seperator();
                        ftLogger::logF("Failed to resolve both file and memory declarations for chunk:");
                        ftLogger::log(bin->m_chunk);
                        ftLogger::logF("File   : %s", fstrc ? "Valid" : "Invalid");
                        ftLogger::logF("Memory : %s", mstrc ? "Valid" : "Invalid");

                        if (fstrc)
                        {
                            ftLogger::log(fstrc);
                            ftLogger::seperator();
                            ftLogger::log(bin->m_fblock, fstrc->getSizeInBytes());
                        }
                        if (mstrc)
                        {
                            ftLogger::log(mstrc);
                            ftLogger::seperator();
                            ftLogger::log(bin->m_mblock, mstrc->getSizeInBytes());
                        }
                        ftLogger::newline(2);
                    }
                    freeChunk(bin);
                }
            }
        }
    }
    else
        status = FS_BAD_ALLOC;
}

void ftFile::insertChunk(const ftPointerHashKey& phk, ftMemoryChunk*& chunk, int& status)
{
    if (!m_map.insert(phk, chunk))
    {
        if (m_fileFlags != LF_NONE)
        {
            ftLogger::seperator();
            ftLogger::logF("Failed to insert chunk");
            ftLogger::log(chunk->m_chunk);
            ftLogger::seperator();
            ftLogger::log(chunk->m_fblock, chunk->m_chunk.m_len);
        }

        freeChunk(chunk);
        status = FS_INV_INSERT;
    }
    else
    {
        m_chunks.push_back(chunk);
    }
}

void ftFile::freeChunk(ftMemoryChunk*& chunk)
{
    if (chunk)
    {
        if (chunk->m_pblock)
            free(chunk->m_pblock);
        if (chunk->m_fblock)
            free(chunk->m_fblock);
        if (chunk->m_mblock)
            free(chunk->m_mblock);
        free(chunk);
        chunk = nullptr;
    }
}


int ftFile::rebuildStructures()
{
    FBTuint32 n;
    int       status = FS_OK;

    FBTbyte * src, *dst;
    FBTsize * srcPtr, *dstPtr;
    ftStruct *fstrc, *mstrc;

    bool diagnostFlag = (m_fileFlags & LF_DIAGNOSTICS) != 0 && (m_fileFlags & LF_DUMP_CAST) != 0;
    bool diagnostics  = false;

    ftMemoryChunk* node;
    for (node = (ftMemoryChunk*)m_chunks.first; node && status == FS_OK; node = node->m_next)
    {
        const ftChunk& chunk = node->m_chunk;
        fstrc                = node->m_fstrc;
        mstrc                = node->m_mstrc;

        if (diagnostFlag && fstrc && mstrc)
        {
            diagnostics = m_castFilter ? searchFilter(m_castFilter, fstrc->getHashedType(), m_castFilterLen) : true;
            if (diagnostics)
                ftLogger::logDiagnosticsCastHeader(chunk, fstrc, mstrc);
        }

        for (n = 0;
             n < chunk.m_nr && status == FS_OK && fstrc && mstrc && (node->m_flag & ftMemoryChunk::BLK_LINKED) == 0;
             ++n)
        {
            dst = mstrc->getBlock(node->m_mblock, n, chunk.m_nr);
            src = fstrc->getBlock(node->m_fblock, n, chunk.m_nr);

            ftStruct::Members::Iterator it = mstrc->getMemberIterator();
            while (it.hasMoreElements())
            {
                ftMember* dstmbr = it.getNext();
                ftMember* srcmbr = fstrc->find(dstmbr);

                if (srcmbr)
                {
                    dstPtr = dstmbr->jumpToOffset(dst);
                    srcPtr = srcmbr->jumpToOffset(src);

                    if (dstPtr && srcPtr)
                    {
                        if (diagnostics)
                        {
                            ftLogger::newline();
                            ftLogger::color(CS_DARKYELLOW);
                            ftLogger::logF("%s %s (%d) ==> %s %s (%d)",
                                           srcmbr->getType(),
                                           srcmbr->getName(),
                                           srcmbr->getOffset(),
                                           dstmbr->getType(),
                                           dstmbr->getName(),
                                           srcmbr->getOffset());
                        }

                        castMember(dstmbr,
                                   dstPtr,
                                   srcmbr,
                                   srcPtr,
                                   status);

                        if (diagnostics)
                        {
                            ftLogger::seperator();
                            ftLogger::log(srcPtr, srcmbr->getSizeInBytes());
                            ftLogger::newline();
                            ftLogger::log(dstPtr, dstmbr->getSizeInBytes());
                            ftLogger::seperator();
                            ftLogger::newline();
                        }

                        if (status != FS_OK)
                        {
                            if (m_fileFlags != LF_NONE)
                                ftLogger::logF("Cast member failed.");
                        }
                    }
                    else if (m_fileFlags != LF_NONE)
                    {
                        ftLogger::seperator();
                        ftLogger::logF("Failed to offset to the member location:");
                        if (!dstPtr)
                        {
                            ftLogger::logF("Destination : %s offset (%d).",
                                           dstmbr->getName(),
                                           dstmbr->getOffset());
                        }
                        if (!srcPtr)
                        {
                            ftLogger::logF("Source      : %s offset (%d).",
                                           srcmbr->getName(),
                                           srcmbr->getOffset());
                        }
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

                    if (m_fileFlags & LF_DO_CHECKS)
                    {
                        void* zeroedMemoryCmp = ::malloc(dstmbr->getSizeInBytes());
                        if (!zeroedMemoryCmp)
                            status = FS_BAD_ALLOC;
                        else
                        {
                            ::memset(zeroedMemoryCmp, 0, dstmbr->getSizeInBytes());

                            if (::memcmp(dstPtr, zeroedMemoryCmp, dstmbr->getSizeInBytes()) != 0)
                                status = FS_OVERFLOW;

                            free(zeroedMemoryCmp);
                        }
                    }

                    if (status == FS_OK)
                    {
                        if (diagnostFlag)
                        {
                            ftLogger::newline();
                            ftLogger::logF("MISSING %s %s",
                                           dstmbr->getType(),
                                           dstmbr->getName());
                            ftLogger::seperator();
                            ftLogger::log(dstPtr, dstmbr->getSizeInBytes());
                            ftLogger::newline();
                        }
                    }
                }
            }
        }

        if (node->m_mblock && status == FS_OK)
        {
            node->m_flag |= ftMemoryChunk::BLK_LINKED;
            notifyDataRead(node->m_mblock, node->m_chunk);
        }
    }

    return status;
}


void ftFile::castMember(ftMember* mstrc,
                        FBTsize*& dstPtr,
                        ftMember* fstrc,
                        FBTsize*& srcPtr,
                        int&      status)
{
    if (mstrc->isPointer())
        castMemberPointer(mstrc, dstPtr, fstrc, srcPtr, status);
    else
        castMemberVariable(mstrc, dstPtr, fstrc, srcPtr, status);
}


void ftFile::castMemberPointer(ftMember* mstrc,
                               FBTsize*& dstPtr,
                               ftMember* fstrc,
                               FBTsize*& srcPtr,
                               int&      status)
{
    if (mstrc->getPointerCount() > 1)
        castPointerToPointer(mstrc, dstPtr, fstrc, srcPtr, status);
    else
        castPointer(mstrc, dstPtr, fstrc, srcPtr, status);
}


template <typename BaseType>
void ftFile::castPointer(FBTsize*& dstPtr, FBTsize* srcPtr, FBTsize arrayLen)
{
    FBTsize i;

    BaseType* sptr = (BaseType*)srcPtr;
    for (i = 0; i < arrayLen; ++i)
    {
        void* vp  = (void*)(FBTsize)(*sptr++);
        dstPtr[i] = (FBTsize)findPointer(ftPointerHashKey(vp));
    }
}



void ftFile::castPointerToPointer(ftMember* dst,
                                  FBTsize*& dstPtr,
                                  ftMember* src,
                                  FBTsize*& srcPtr,
                                  int&      status)
{
    ftMemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
    if (bin)
    {
        if (bin->m_flag & ftMemoryChunk::BLK_MODIFIED && bin->m_pblock)
        {
            (*dstPtr) = (FBTsize)bin->m_pblock;

            if (m_fileFlags != LF_NONE)
                ftLogger::logF("Reusing block %p", bin->m_pblock);
        }
        else
        {
            FBTsize fps = m_file->getSizeofPointer();
            if (fps == 4 || fps == 8)
            {
                FBTsize  total    = bin->m_pblockLen / fps;
                FBTsize* newBlock = (FBTsize*)::calloc(total, sizeof(FBTsize));
                if (newBlock != nullptr)
                {
                    FBTsize* optr = (FBTsize*)bin->m_fblock;
                    if (fps == 4)
                        castPointer<FBTuint32>(newBlock, optr, total);
                    else
                        castPointer<FBTuint64>(newBlock, optr, total);

                    free(bin->m_pblock);
                    bin->m_pblock = newBlock;
                    bin->m_flag |= ftMemoryChunk::BLK_MODIFIED;

                    (*dstPtr) = (FBTsize)newBlock;
                }
                else
                    status = FS_BAD_ALLOC;
            }
            else
            {
                if (m_fileFlags != LF_NONE)
                    ftLogger::logF("Unknown pointer length(%d). Pointers should be either 4 or 8 bytes", fps);
                status = FS_INV_VALUE;
            }
        }
    }
    else if (m_fileFlags & LF_MISSING_PTR_PTR)
    {
        if ((FBTsize)(*srcPtr) != 0)
        {
            ftLogger::logF("Failed to find corresponding chunk for address (0x%08X)",
                           (FBTsize)(*srcPtr));
            ftLogger::logF("Source");
            ftLogger::log(src);
            ftLogger::logF("Destination");
            ftLogger::log(dst);
            ftLogger::log(srcPtr, src->getSizeInBytes());
            ftLogger::newline();
            ftLogger::log(dstPtr, dst->getSizeInBytes());
        }
    }
}


void ftFile::castPointer(ftMember* mstrc,
                         FBTsize*& dstPtr,
                         ftMember* fstrc,
                         FBTsize*& srcPtr,
                         int&      status)
{
    FBTsize arrayLen = skMin(mstrc->getArraySize(), fstrc->getArraySize());

    FBTsize fps = m_file->getSizeofPointer();
    if (fps == 4 || fps == 8)
    {
        if (fps == 4)
            castPointer<FBTuint32>(dstPtr, srcPtr, arrayLen);
        else
            castPointer<FBTuint64>(dstPtr, srcPtr, arrayLen);
    }
    else
    {
        if (m_fileFlags != LF_NONE)
            ftLogger::logF("Unknown pointer length(%d). Pointers should be either 4 or 8 bytes", fps);
        status = FS_INV_VALUE;
    }
}


void ftFile::castMemberVariable(ftMember* dst,
                                FBTsize*& dstPtr,
                                ftMember* src,
                                FBTsize*& srcPtr,
                                int&      status)
{
    FBTsize dstElmSize   = dst->getSizeInBytes();
    FBTsize srcElmSize   = src->getSizeInBytes();
    FBTsize maxAvailable = skMin(srcElmSize, dstElmSize);
    FBTsize alen         = skMax(skMin(dst->getArraySize(), src->getArraySize()), 1);

    // FT_MAX_MBR_RANGE
    // Provides an upper boundary for the number of array
    // elements that can be used
    // for instance: int member_variable[FT_MAX_MBR_RANGE];


    if (maxAvailable <= 0 || alen > FT_MAX_MBR_RANGE)
    {
        // re examine this, it should already be ruled out
        // when the table is compiled

        if (m_fileFlags != LF_NONE)
            ftLogger::logF("Element size is out of range src(%d), dst(%d), max(%d)",
                           srcElmSize,
                           dstElmSize,
                           FT_MAX_MBR_RANGE);
        status = FS_INV_SIZE;
    }
    else if (!src->isValidAtomicType() || !dst->isValidAtomicType())
    {
        // re examine this, it should already be ruled out
        // when the table is compiled

        if (m_fileFlags != LF_NONE)
            ftLogger::logF("Invalid atomic type src(%d), dst(%d)",
                           src->getAtomicType(),
                           dst->getAtomicType());
        status = FS_INV_SIZE;
    }
    else
    {
        bool needsSwapped = (m_headerFlags & FH_ENDIAN_SWAP) != 0 && srcElmSize > 1;
        if (needsSwapped)
            needsSwapped = !src->isCharacterArray();

        bool needsCast, canCopy;
        needsCast = src->getHashedType() != dst->getHashedType();

        canCopy = !needsCast && !needsSwapped;
        if (canCopy)
        {
            if (m_fileFlags & LF_DIAGNOSTICS)
            {
                if (srcElmSize > dstElmSize)
                {
                    ftLogger::logF("The source member is larger than the destination member.");
                    ftLogger::logF("    %d bytes of data will be truncated.", srcElmSize - dstElmSize);
                }
            }

            if (dst->isCharacter())
            {
                // Allow for null terminated strings.
                // This will handle the case when there is extra information in the
                // source buffer. strncpy will copy up to the first null terminator
                // and the extra info in the buffer will be left untouched.
                ::strncpy((FBTbyte*)dstPtr, (FBTbyte*)srcPtr, maxAvailable);
            }
            else
                ::memcpy(dstPtr, srcPtr, maxAvailable);
        }
        else
        {
            FBTbyte* dstBPtr = reinterpret_cast<FBTbyte*>(dstPtr);
            FBTbyte* srcBPtr = reinterpret_cast<FBTbyte*>(srcPtr);


            //if (alen > 1)
            //{
            //    castAtomicMemberArray(dst, dstBPtr, src, srcBPtr, status);
            //}
            //else
            //{
            //    castAtomicMember(dst, dstBPtr, src, srcBPtr, status);
            //}


            ftAtomic stp, dtp;
            stp = src->getAtomicType();
            dtp = dst->getAtomicType();

            FBTbyte tmpBuf[ftEndianUtils::MaxSwapSpace + 1] = {};

            FBTsize i;
            FBTsize elen  = maxAvailable;
            FBTsize cslen = skMin(ftEndianUtils::MaxSwapSpace, srcElmSize / alen);
            FBTsize cdlen = skMin(ftEndianUtils::MaxSwapSpace, maxAvailable / alen);


            int sc;
            if (src->isInteger16())
                sc = 1;
            else if (src->isInteger32())
                sc = 2;
            else if (src->isInteger64())
                sc = 3;
            else
                sc = 0;

            for (i = 0; i < alen; i++)
            {
                if (needsSwapped)
                {
                    ::memcpy(tmpBuf, srcBPtr, cslen);

                    switch (sc)
                    {
                    case 1:
                        swap16((FBTuint16*)tmpBuf, 1);
                        break;
                    case 2:
                        swap32((FBTuint32*)tmpBuf, 1);
                        break;
                    case 3:
                        swap64((FBTuint64*)tmpBuf, 1);
                        break;
                    default:
                        ::memset(tmpBuf, 0, cdlen);
                        break;
                    }
                }
                else
                {
                    ftAtomicUtils::cast((char*)srcBPtr, (char*)dstBPtr, stp, dtp, 1);
                }

                dstBPtr += dstElmSize;
                srcBPtr += srcElmSize;
            }
        }
    }
}


void ftFile::castAtomicMemberArray(ftMember* dst,
                                   FBTbyte*& dstPtr,
                                   ftMember* src,
                                   FBTbyte*& srcPtr,
                                   int&      status)
{
    // TODO
}

void ftFile::castAtomicMember(ftMember* dst,
                              FBTbyte*& dstPtr,
                              ftMember* src,
                              FBTbyte*& srcPtr,
                              int&      status)
{
    // TODO
}


void ftFile::setFilter(FBThash*& dest, FBTint32& destLen, FBThash* filter, FBTint32 length)
{
    if (!filter)
        return;

    dest  = filter;
    int i = 0, j, k;
    while (i < length && dest[i] != 0)
        i++;

    destLen = i;
    for (i = 0; i < destLen - 2; i++)
    {
        k = i;
        for (j = i + 1; j < destLen - 1; ++j)
            if (dest[j] < dest[k])
                k = j;
        if (k != i)
            skSwap(dest[i], dest[k]);
    }
}


bool ftFile::searchFilter(const FBThash* searchIn, const FBThash& searchFor, const FBTint32& len)
{
    if (!searchIn)
        return false;

    int f = 0, l = len - 1, m;
    while (f <= l)
    {
        m = (f + l) / 2;
        if (searchIn[m] == searchFor)
            return true;
        else if (searchIn[m] > searchFor)
            l = m - 1;
        else
            f = m + 1;
    }
    return false;
}

bool ftFile::skip(const FBThash& id)
{
    if (!m_filterList)
        return false;

    bool res = searchFilter(m_filterList, id, m_filterListLen);
    return m_inclusive ? !res : res;
}


void ftFile::setFilterList(FBThash* filter, FBTuint32 length, bool inclusive)
{
    m_inclusive = inclusive;
    setFilter(m_filterList, m_filterListLen, filter, length);
}


void ftFile::setCastFilter(FBThash* filter, FBTuint32 length)
{
    setFilter(m_castFilter, m_castFilterLen, filter, length);
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

void* ftFile::findPointer(const FBTsize& iptr)
{
    FBTsize i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i)->m_mblock;
    return 0;
}

void* ftFile::findPointer(const ftPointerHashKey& iptr)
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
        m_memory = new ftTables(sizeof(void*));

        status = initializeTables(m_memory);
        if (status == FS_OK)
        {
            if (m_fileFlags & LF_DO_CHECKS)
                status = runTableChecks(m_memory);
        }
    }
    return status;
}


int ftFile::initializeTables(ftTables* tables)
{
    // This calls down into derived classes
    // to gain access to the memory table.
    void*   tableData = getTables();
    FBTsize tableSize = getTableSize();

    if (tableData != 0 && tableSize > 0 && tableSize != SK_NPOS)
        return tables->read(tableData, tableSize, 0, m_fileFlags);
    return FS_FAILED;
}


void ftFile::clearStorage(void)
{
    ftMemoryChunk* node = (ftMemoryChunk*)m_chunks.first;
    ftMemoryChunk* tmp;
    while (node)
    {
        tmp  = node;
        node = node->m_next;
        freeChunk(tmp);
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
    FBTuint8  cp, ce;
    skStream* fs;

    if (mode == PM_COMPRESSED)
        fs = new ftGzStream();
    else
        fs = new skFileStream();

    fs->open(path, skStream::WRITE);

    if (!fs->isOpen())
    {
        if (m_fileFlags != LF_NONE)
            ftLogger::logF("Failed to open output file for saving.");
        return FS_FAILED;
    }

    cp = FT_VOID8 ? FM_64_BIT : FM_32_BIT;
    ce = getEndian();
    if (ce == FT_ENDIAN_IS_BIG)
        ce = FM_BIG_ENDIAN;
    else
        ce = FM_LITTLE_ENDIAN;

    ftHeader header;
    char     version[33];
    sprintf(version, "%i", m_memoryVersion);
    header.resize(12);

    strncpy(header.ptr(), m_uhid, 7);  // The first 7 bytes of the header
    header[7] = cp;                    // The 8th byte is the pointer size
    header[8] = ce;                    // The 9th byte is the endian
    strncpy(&header[9], version, 3);   // The last 3 bytes are the version string.
    fs->write(header.ptr(), header.capacity());

    // the main bulk of saving chunks
    // happens in the  derived classes.
    serializeData(fs);

    ftChunk ch;
    ch.m_code     = ftIdNames::DNA1;
    ch.m_len      = (FBTuint32)getTableSize();
    ch.m_nr       = 1;
    ch.m_addr     = 0;  // cannot be looked back up
    ch.m_structId = 0;
    fs->write(&ch, ftChunkUtils::BlockSize);
    fs->write(getTables(), ch.m_len);


    ch.m_code     = ftIdNames::ENDB;
    ch.m_len      = 0;
    ch.m_nr       = 0;
    ch.m_addr     = 0;
    ch.m_structId = 0;
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
        if (m_fileFlags != LF_NONE)
            ftLogger::logF("writeStruct: %s - not found", id);
        return;
    }
    serialize(stream, ft, code, len, writeData);
}


void ftFile::serialize(skStream*   stream,
                       const char* id,
                       FBTuint32   code,
                       FBTsize     len,
                       void*       writeData,
                       int         nr)
{
    if (m_memory == 0)
        getMemoryTable();

    FBTuint32 ft = m_memory->findTypeId(id);
    if (ft == SK_NPOS32)
    {
        if (m_fileFlags != LF_NONE)
            ftLogger::logF("writeStruct: %s - not found", id);
        return;
    }

    serializeChunk(stream, code, nr, ft, len, writeData);
}


bool ftFile::isValidWriteData(void* writeData, FBTsize len)
{
    if (writeData == nullptr || len == SK_NPOS || len == 0)
    {
        if (m_fileFlags != LF_NONE)
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
        ch.m_code     = code;
        ch.m_len      = (FBTuint32)len;
        ch.m_nr       = nr;
        ch.m_addr     = (FBTsize)writeData;
        ch.m_structId = typeIndex;
        ftChunkUtils::write(&ch, stream);

        if (m_fileFlags & LF_WRITE_CHUNKS)
        {
            ftLogger::newline();
            ftLogger::log(ch);
            ftLogger::seperator();
            ftLogger::log(writeData, len);
            ftLogger::seperator();
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
    serializeChunk(stream, ftIdNames::DATA, 1, 0, len, writeData);
}

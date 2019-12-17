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
#include "ftPlatformHeaders.h"
#include "ftStreams.h"
#include "ftTables.h"



const FBTuint32      ENDB      = ftID('E', 'N', 'D', 'B');
const FBTuint32      DNA1      = ftID('D', 'N', 'A', '1');
const FBTuint32      DATA      = ftID('D', 'A', 'T', 'A');
const FBTuint32      SDNA      = ftID('S', 'D', 'N', 'A');
static const FBThash DATABLOCK = ftCharHashKey("Link").hash();


// Compile asserts
SK_ASSERTCOMP(ChunkLen32, sizeof(ftFile::Chunk32) == 20);
SK_ASSERTCOMP(ChunkLen64, sizeof(ftFile::Chunk64) == 24);

#if ftARCH == ftARCH_32
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk32) == sizeof(ftFile::Chunk));
#else
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk64) == sizeof(ftFile::Chunk));
#endif

#define ftMALLOC_FAILED printf("Failed to allocate memory!\n");
#define ftINVALID_READ printf("Invalid read!\n");
#define ftINVALID_LEN printf("Invalid block length!\n");
#define ftINVALID_INS printf("Table insertion failed!\n");
#define ftLINK_FAILED printf("Linking failed!\n");
#define FT_TABLE_FAILED printf("Failed to initialize tables!\n");


struct ftChunk
{
    enum Size
    {
        BlockSize = sizeof(ftFile::Chunk),
        Block32   = sizeof(ftFile::Chunk32),
        Block64   = sizeof(ftFile::Chunk64),
    };
    static FBTsize read(ftFile::Chunk* dest, skStream* stream, int flags);
    static FBTsize write(ftFile::Chunk* src, skStream* stream);
};



ftFile::ftFile(const char* uid) :
    m_version(-1),
    m_fileVersion(0),
    m_hederFlags(0),
    m_uhid(uid),
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
        ::free(m_curFile);
    m_curFile = 0;
    clearStorage();
}

skStream* ftFile::openStream(const char* path, int mode)
{
    skStream* stream = 0;
    if (mode == PM_UNCOMPRESSED || mode == PM_COMPRESSED)
    {
#if ftUSE_GZ_FILE == 1
        if (mode == PM_COMPRESSED)
            stream = new ftGzStream();
        else
#endif
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



int ftFile::load(const void* memory, FBTsize sizeInBytes, int mode)
{
    skMemoryStream ms;
    ms.open(memory, sizeInBytes, skStream::READ);  //, mode == PM_COMPRESSED);
    if (!ms.isOpen())
    {
        printf("Memory %p(%d) loading failed!\n", memory, sizeInBytes);
        return FS_FAILED;
    }

    return parseStreamImpl(&ms);
}


ftBinTables* ftFile::getMemoryTable(void)
{
    if (!m_memory)
        initializeMemory();
    return m_memory;
}


int ftFile::initializeTables(ftBinTables* tables)
{
    void*   tableData = getTables();
    FBTsize tableSize = getTableSize();

    if (tableData != 0 && tableSize > 0 && tableSize != SK_NPOS)
        return tables->read(tableData, tableSize, false) ? (int)FS_OK : (int)FS_FAILED;
    return FS_FAILED;
}


int ftFile::parseHeader(skStream* stream)
{
    m_header.resize(12);
    stream->read(m_header.ptr(), 12);

    if (!ftCharNEq(m_header.c_str(), m_uhid, 6))
        return FS_INV_HEADER_STR;

    char* headerMagic = (m_header.ptr() + 7);

    m_hederFlags  = 0;
    m_fileVersion = 0;


    if (*(headerMagic++) == FM_64_BIT)
    {
        m_hederFlags |= FH_CHUNK_64;
        if (FT_VOID4)
            m_hederFlags |= FH_VAR_BITS;
    }
    else if (FT_VOID8)
        m_hederFlags |= FH_VAR_BITS;

    if (*(headerMagic++) == FM_BIG_ENDIAN)
    {
        if (FT_ENDIAN_IS_LITTLE)
            m_hederFlags |= FH_ENDIAN_SWAP;
    }
    else if (FT_ENDIAN_IS_BIG)
        m_hederFlags |= FH_ENDIAN_SWAP;

    m_fileVersion = atoi(headerMagic);
    return FS_OK;
}


int ftFile::initializeMemory(void)
{
    int status = m_memory == 0 ? (int)FS_FAILED : (int)FS_OK;
    if (!m_memory)
    {
        m_memory = new ftBinTables();

        status = initializeTables(m_memory);
        if (status != FS_OK)
        {
            FT_TABLE_FAILED;
            return status;
        }
    }
    return status;
}


void ftFile::clearStorage(void)
{
    if (m_fileData)
        ::free(m_fileData);
    m_fileData = 0;

    MemoryChunk* node = (MemoryChunk*)m_chunks.first;
    MemoryChunk* tmp;


    while (node)
    {
        if (node->m_block)
            ::free(node->m_block);
        node->m_block = 0;

        if (node->m_newBlock)
            ::free(node->m_newBlock);

        node->m_newBlock = 0;

        tmp  = node;
        node = node->m_next;

        ::free(tmp);
    }

    delete m_file;
    m_file = 0;

    if (m_fileData)
        free(m_fileData);
    m_fileData = 0;

    delete m_memory;
    m_memory = 0;
}



int ftFile::parseStreamImpl(skStream* stream)
{
    int status;

    FBTsize bytesRead = 0;

    // ensure that any previous memory has been freed
    clearStorage();
    status = parseHeader(stream);

    if (status != FS_OK)
    {
        printf("Failed to extract file header!\n");
        return status;
    }

    status = initializeMemory();
    if (status != FS_OK)
    {
        FT_TABLE_FAILED;
        return status;
    }

    m_map.reserve(FT_DEF_ALLOC);
    Chunk chunk;
    do
    {
        if ((bytesRead = ftChunk::read(&chunk, stream, m_hederFlags)) <= 0)
        {
            ftINVALID_READ;
            return FS_INV_READ;
        }

        if (chunk.m_code == SDNA)
        {
            chunk.m_code = DNA1;
            stream->seek(-(int)bytesRead, SEEK_CUR);
            chunk.m_len = (FBTuint32)(stream->size() - stream->position());
        }

        // exit on end byte
        if (chunk.m_code == ENDB)
            break;

        void* curPtr = ::malloc(chunk.m_len);
        if (!curPtr)
        {
            ftMALLOC_FAILED;
            return FS_BAD_ALLOC;
        }

        if (stream->read(curPtr, chunk.m_len) <= 0)
        {
            ftINVALID_READ;
            return FS_INV_READ;
        }

        if (chunk.m_code == DNA1)
        {
            // Save this for later deletion. All other blocks are
            // in the MemoryChunk linked list.
            m_fileData = curPtr;
            m_file     = new ftBinTables(curPtr, chunk.m_len, m_hederFlags & FH_CHUNK_64 ? 8 : 4);

            if (!m_file->read((m_hederFlags & FH_ENDIAN_SWAP) != 0))
            {
                FT_TABLE_FAILED;
                return FS_INV_READ;
            }

            compileOffsets();

            if ((status = link()) != FS_OK)
            {
                ftLINK_FAILED;
                return FS_LINK_FAILED;
            }
            break;
        }
        else
        {
            if (m_map.find(chunk.m_old) != m_map.npos)
            {
                ::free(curPtr);
                curPtr = 0;
            }
            else
            {
                MemoryChunk* bin = static_cast<MemoryChunk*>(::malloc(sizeof(MemoryChunk)));
                if (!bin)
                {
                    ftMALLOC_FAILED;
                    return FS_BAD_ALLOC;
                }
                ::memset(bin, 0, sizeof(MemoryChunk));

                bin->m_block = curPtr;

                Chunk* cp    = &bin->m_chunk;
                cp->m_code   = chunk.m_code;
                cp->m_len    = chunk.m_len;
                cp->m_nr     = chunk.m_nr;
                cp->m_typeid = chunk.m_typeid;
                cp->m_old    = chunk.m_old;
                m_chunks.push_back(bin);

                if (m_map.insert(bin->m_chunk.m_old, bin) == false)
                {
                    ftINVALID_INS;
                    return FS_INV_INSERT;
                }
            }
        }
    } while (!stream->eof());

    return status;
}



class ftLinkCompiler
{
public:
    ftBinTables* m_mp;
    ftBinTables* m_fp;

    ftStruct* find(const ftCharHashKey& kvp);
    ftStruct* find(ftStruct* strc, ftStruct* member, bool isPointer, bool& needCast);
    int       link(void);
};

ftStruct* ftLinkCompiler::find(const ftCharHashKey& kvp)
{
    return m_fp->findStructByName(kvp);
}


ftStruct* ftLinkCompiler::find(ftStruct* strc, ftStruct* member, bool isPointer, bool& needCast)
{
    ftStruct::Members::PointerType mdata = strc->m_members.ptr();
    FBTsizeType                    i, s = strc->m_members.size();

    FBThash mtype = member->m_val.m_type;

    for (i = 0; i < s; i++)
    {
        ftStruct* strc2 = &mdata[i];
        if (strc2->m_nr == member->m_nr && strc2->m_dp == member->m_dp)
        {
            if (strc2->m_val.m_name == member->m_val.m_name)
            {
                if (!strc2->m_keyChain.equals(member->m_keyChain))
                    continue;

                FBThash otype = strc2->m_val.m_type;
                if (mtype == otype)
                    return strc2;

                if (isPointer)
                    continue;

                // Make sure it can cast properly if they are different
                if (ftAtomicUtils::isInteger(mtype) &&
                    ftAtomicUtils::isInteger(otype))
                    return strc2;

                if (ftAtomicUtils::isNumeric(mtype) &&
                    ftAtomicUtils::isNumeric(otype))
                {
                    needCast = true;
                    return strc2;
                }
            }
        }
    }
    return 0;
}



int ftLinkCompiler::link(void)
{
    ftBinTables::OffsM::PointerType mdata = m_mp->getOffsetPtr();
    ftBinTables::OffsM::PointerType fdata = m_fp->getOffsetPtr();

    FBTsizeType                    i, i2;
    ftStruct::Members::PointerType p2;

    for (i = 0; i < m_mp->getOffsetCount(); ++i)
    {
        ftStruct* strc = mdata[i];
        strc->m_link   = find(m_mp->m_type[strc->m_key.k16[0]].m_name);

        if (strc->m_link)
            strc->m_link->m_link = strc;

        p2 = strc->m_members.ptr();
        for (i2 = 0; i2 < strc->m_members.size(); ++i2)
        {
            ftStruct* member = &strc->m_members[i2];
            SK_ASSERT(!member->m_link);

            member->m_flag |= strc->m_link ? 0 : ftStruct::MISSING;

            if (!(member->m_flag & ftStruct::MISSING))
            {
                SK_ASSERT(member->m_key.k16[1] < m_mp->m_nameNr);

                bool isPointer = m_mp->m_name[member->m_key.k16[1]].m_ptrCount > 0;
                bool needCast  = false;
                member->m_link = find(strc->m_link, member, isPointer, needCast);
                if (member->m_link)
                {
                    member->m_link->m_link = member;
                    if (needCast)
                    {
                        member->m_flag |= ftStruct::NEED_CAST;
                        member->m_link->m_flag |= ftStruct::NEED_CAST;
                    }
                }
            }
        }
    }
    return ftFile::FS_OK;
}



int ftFile::allocNewBlocks(void)
{
    int          status = FS_OK;
    MemoryChunk* node   = (MemoryChunk*)m_chunks.first;

    while (node && status == FS_OK)
    {
        const Chunk& chunk = node->m_chunk;

        if (!m_file->isValidType(chunk.m_typeid))
            printf("Chunk type id out of bounds(%d)\n", chunk.m_typeid);
        else if (m_file->isLinkedToMemory(chunk.m_typeid))
        {
            const ftStruct *fileStruct, *memoryStruct;

            fileStruct   = m_file->findStructByType(chunk.m_typeid);
            memoryStruct = fileStruct->m_link;

            // link the new type id on the node
            // so it's accessible later
            node->m_newTypeId = memoryStruct->m_strcId;

            FBThash curHash = m_memory->getTypeHash(memoryStruct->getTypeIndex());

            if (curHash == DATABLOCK)
            {
                node->m_newBlock = ::malloc(chunk.m_len);

                if (!node->m_newBlock)
                {
                    ftMALLOC_FAILED;
                    status = FS_BAD_ALLOC;
                }
                else
                    ::memcpy(node->m_newBlock, node->m_block, chunk.m_len);
            }
            else if (!skip(curHash))
            {
                const FBTuint32 totSize = (chunk.m_nr * memoryStruct->getSizeInBytes());

                node->m_chunk.m_len = totSize;
                node->m_newBlock    = ::malloc(totSize);

                if (!node->m_newBlock)
                {
                    ftMALLOC_FAILED;
                    status = FS_BAD_ALLOC;
                }
                else
                {
                    ::memset(node->m_newBlock, 0, totSize);
                }
            }
            else
            {
                // probably never happens

                if (node->m_newBlock != 0)
                {
                    printf("ftFile::allocNewBlocks - What did you do?");
                    free(node->m_newBlock);
                    node->m_newBlock = 0;
                }
            }
        }
        node = node->m_next;
    }
    return status;
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
            printf("Chunk type id out of bounds(%d)\n", chunk.m_typeid);
        }
        else if (!m_memory->isValidType(node->m_newTypeId))
        {
            ftLogger::log(chunk);
            printf("Memory Type id out of bounds(%d)", node->m_newTypeId);
        }
        else
        {
            ftStruct* currentStruct = m_memory->findStructByType(node->m_newTypeId);
            if (currentStruct != nullptr)
            {
                FBThash curHash = m_memory->getTypeHash(currentStruct->getTypeIndex());

                // filter out plain pointer types,
                // there is no need to attempt to update them.
                if (curHash != DATABLOCK)
                {
                    bool skipBlock = skip(curHash);
                    if (!currentStruct->m_link || skipBlock || !node->m_newBlock)
                    {
                        if (!node->m_newBlock)
                        {
                            printf("Missing block for %s \n", m_memory->getStructName(currentStruct));
                            ::free(node->m_newBlock);
                        }
                        node->m_newBlock = 0;
                    }
                    else
                    {
                        // finally rebuild the data
                        linkMembers(node, currentStruct);
                    }
                }
            }
            else
                printf("No struct found with the type identifier(%d)", node->m_newTypeId);
        }
    }

    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_block)
        {
            ::free(node->m_block);
            node->m_block = 0;
        }
    }
    return ftFile::FS_OK;
}



void ftFile::linkMembers(MemoryChunk* node, ftStruct* currentStruct)
{
    SK_ASSERT(node);
    SK_ASSERT(currentStruct);

    ftStruct::Members::SizeType n, i, memberLen;
    memberLen = currentStruct->m_members.size();

    char *   dst, *src;
    FBTsize *dstPtr, *srcPtr;

    if (!node->m_newBlock)
    {
        printf("Missing new block\n");
        return;
    }
    if (!node->m_block)
    {
        printf("Missing new block\n");
        return;
    }

    for (n = 0; n < node->m_chunk.m_nr; ++n)
    {
        dst = ((char*)node->m_newBlock) + currentStruct->m_len * n;
        src = ((char*)node->m_block) + currentStruct->m_link->m_len * n;

        if (dst != nullptr && src != nullptr)
        {
            for (i = 0; i < memberLen; ++i)
            {
                ftStruct* dstStrc = currentStruct->getMember(i);
                if (dstStrc != nullptr)
                {
                    ftStruct* srcStrc = dstStrc->m_link;
                    if (srcStrc)
                    {
                        dstPtr = reinterpret_cast<FBTsize*>(dst + dstStrc->m_off);
                        srcPtr = reinterpret_cast<FBTsize*>(src + srcStrc->m_off);

                        linkMembers(dstStrc,
                                    dstPtr,
                                    srcStrc,
                                    srcPtr);
                    }
                    else
                    {
                        //printf("Missing structure link\n");
                        //ftLogger::log(dstStrc);
                    }
                }
            }
        }
    }

    notifyDataRead(node->m_newBlock, node->m_chunk);
}



void ftFile::linkMembers(ftStruct* dst,
                         FBTsize*& dstPtr,
                         ftStruct* src,
                         FBTsize*& srcPtr)
{
    const ftName& name = m_memory->getStructNameByIdx(dst->m_key.k16[1]);
    if (name.m_ptrCount > 0)
        castMemberPointer(name, dst, dstPtr, src, srcPtr);
    else
        castMemberVariable(name, dst, dstPtr, src, srcPtr);
}


void ftFile::castMemberPointer(const ftName& name,
                               ftStruct*     dst,
                               FBTsize*&     dstPtr,
                               ftStruct*     src,
                               FBTsize*&     srcPtr)
{
    if (name.m_ptrCount > 1)
        castPointerToPointer(name, dst, dstPtr, src, srcPtr);
    else 
        castPointer(name, dst, dstPtr, src, srcPtr);
}



void ftFile::castPointer(const ftName& name,
                         ftStruct*     dst,
                         FBTsize*&     dstPtr,
                         ftStruct*     src,
                         FBTsize*&     srcPtr)
{
    const ftName& srcname = m_file->getStructNameByIdx(src->m_key.k16[1]);
    if (srcname.m_name != nullptr)
    {
        int arrayLen = skMin(name.m_arraySize, name.m_arraySize);

        FBTsize fps = m_file->getSizeofPointer();
        if (fps == 4 || fps == 8)
        {
            FBTsize* dptr = (FBTsize*)dstPtr;

            // Always use a 32 bit integer,
            // then offset + 2 for a 64 bit pointer.
            FBTuint32* sptr = (FBTuint32*)srcPtr;
            FBTsize*   mptr = dptr;

            int i;
            for (i = 0; i < arrayLen; ++i,
                sptr += (fps == 4 ? 1 : 2))
            {
                (*mptr++) = (FBTsize)findPtr((FBTsize)(*sptr));
            }
        }
        else
            printf("Invalid size of pointer\n");
    }
    else
    {
        printf("Unable to find struct name with index(%d)\n",
               src->m_key.k16[1]);
    }
}

void ftFile::castPointerToPointer(const ftName& name,
                                  ftStruct*     dst,
                                  FBTsize*&     dstPtr,
                                  ftStruct*     src,
                                  FBTsize*&     srcPtr)
{
    // link pointers to pointers
    MemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
    if (bin)
    {
        if (bin->m_flag & MemoryChunk::BLK_MODIFIED)
            (*dstPtr) = (FBTsize)bin->m_newBlock;
        else
        {
            FBTsize fps = m_file->getSizeofPointer();
            if (fps == 4 || fps == 8)
            {
                FBTsize  total  = bin->m_chunk.m_len / fps;
                FBTsize  mptrsz = total * sizeof(FBTsize);
                FBTsize* nptr   = (FBTsize*)::malloc(mptrsz);

                if (nptr == nullptr)
                {
                    // TODO examine the logic for this
                    ftMALLOC_FAILED;
                    return;
                }
                ::memset(nptr, 0, mptrsz);

                // Always use a 32 bit integer,
                // then offset + 2 for a 64 bit pointer.
                FBTuint32* optr = (FBTuint32*)bin->m_block;


                FBTsize* mptr = nptr;

                FBTsize i;
                for (i = 0; i < total; i++,
                    optr += (fps == 4 ? 1 : 2))
                {
                    (*mptr++) = (FBTsize)findPtr((FBTsize)*optr);
                }


                (*dstPtr) = (FBTsize)(nptr);

                bin->m_chunk.m_len = mptrsz;
                bin->m_flag |= MemoryChunk::BLK_MODIFIED;

                ::free(bin->m_newBlock);
                bin->m_newBlock = nptr;
            }
            else
            {
                printf("Unknown pointer len\n");
            }
        }
    }
    else
        printf("failed to find other chunk\n");
}


void ftFile::castMemberVariable(const ftName& name,
                                ftStruct*     dst,
                                FBTsize*&     dstPtr,
                                ftStruct*     src,
                                FBTsize*&     srcPtr)
{
    FBTsize total, malen, pi, mptrsz, a2;

    FBTsize       fps        = m_file->getSizeofPointer();
    FBTsize       mps        = m_memory->getSizeofPointer();
    const ftName& nameS      = m_file->getStructNameByIdx(src->m_key.k16[1]);
    FBTsize       dstElmSize = dst->m_len / name.m_arraySize;
    FBTsize       srcElmSize = src->m_len / nameS.m_arraySize;

    bool endianSwap = (m_hederFlags & FH_ENDIAN_SWAP) != 0;
    bool needCast   = (dst->m_flag & ftStruct::NEED_CAST) != 0;
    bool needSwap   = endianSwap && srcElmSize > 1;

    if (!needCast && !needSwap &&
        dst->m_val.m_type == dst->m_val.m_type)
    {
        ::memcpy(dstPtr, srcPtr, skMin(src->m_len, dst->m_len));
    }
    else
    {
        FBTbyte* dstBPtr = reinterpret_cast<FBTbyte*>(dstPtr);
        FBTbyte* srcBPtr = reinterpret_cast<FBTbyte*>(srcPtr);

        ftAtomic stp = ftAtomic::FT_ATOMIC_UNKNOWN, dtp = ftAtomic::FT_ATOMIC_UNKNOWN;
        if (needCast || needSwap)
        {
            stp = ftAtomicUtils::getPrimitiveType(src->m_val.m_type);
            dtp = ftAtomicUtils::getPrimitiveType(dst->m_val.m_type);
        }

        FBTsize alen = skMin(nameS.m_arraySize, name.m_arraySize);
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
                    ftSwap16((FBTuint16*)tmpBuf, 1);
                else if (stp >= ftAtomic::FT_ATOMIC_INT && stp <= ftAtomic::FT_ATOMIC_FLOAT)
                    ftSwap32((FBTuint32*)tmpBuf, 1);
                else if (stp == ftAtomic::FT_ATOMIC_DOUBLE)
                    ftSwap64((FBTuint64*)tmpBuf, 1);
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

void* ftFile::findPtr(const FBTsize& iptr)
{
    FBTsizeType i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i)->m_newBlock;
    return 0;
}


ftFile::MemoryChunk* ftFile::findBlock(const FBTsize& iptr)
{
    FBTsizeType i;
    if ((i = m_map.find(iptr)) != m_map.npos)
        return m_map.at(i);
    return nullptr;
}

int ftFile::compileOffsets(void)
{
    ftLinkCompiler lnk;
    lnk.m_mp = m_memory;
    lnk.m_fp = m_file;
    return lnk.link();
}

int ftFile::save(const char* path, const int mode)
{
    skStream* fs;

#if ftUSE_GZ_FILE == 1
    if (mode == PM_COMPRESSED)
        fs = new ftGzStream();
    else
#endif
    {
        fs = new skFileStream();
    }

    fs->open(path, skStream::WRITE);


    FBTuint8 cp = FT_VOID8 ? FM_64_BIT : FM_32_BIT;
    FBTuint8 ce = ((FBTuint8)ftGetEndian()) == FT_ENDIAN_IS_BIG ? FM_BIG_ENDIAN : FM_LITTLE_ENDIAN;

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

    if (writeData == nullptr || len <= 0)
    {
        printf("Invalid write data\n");
        return;
    }

    FBTuint32 ft = m_memory->findTypeId(id);
    if (ft == SK_NPOS32)
    {
        printf("writeStruct: %s - not found", id);
        return;
    }

    serialize(stream, ft, code, len, writeData);
}


void ftFile::serialize(skStream* stream,
                       FBTtype   index,
                       FBTuint32 code,
                       FBTsize   len,
                       void*     writeData)
{
    if (writeData == nullptr || len < sizeof(void*))
    {
        printf("Invalid write data\n");
        return;
    }

    Chunk ch;
    ch.m_code   = code;
    ch.m_len    = len;
    ch.m_nr     = 1;
    ch.m_old    = (FBTsize)writeData;
    ch.m_typeid = index;

    ftChunk::write(&ch, stream);
}

void ftFile::serialize(skStream* stream, FBTsize len, void* writeData, int nr)
{
    if (m_memory == 0)
        getMemoryTable();

    if (writeData == nullptr || len < sizeof(void*))
    {
        printf("Invalid write data\n");
        return;
    }

    Chunk ch;
    ch.m_code   = DATA;
    ch.m_len    = len;
    ch.m_nr     = nr;
    ch.m_old    = (FBTsize)writeData;
    ch.m_typeid = 0;
    ftChunk::write(&ch, stream);
}



FBTsize ftChunk::write(ftFile::Chunk* src, skStream* stream)
{
    FBTsize size = 0;
    size += stream->write(src, BlockSize);
    size += stream->write((void*)src->m_old, src->m_len);

    ftLogger::log(*src);
    ftLogger::log((void*)src->m_old, src->m_len);
    return size;
}


FBTsize ftChunk::read(ftFile::Chunk* dest, skStream* stream, int flags)
{
    FBTsize bytesRead  = 0;
    bool    bitsVary   = (flags & ftFile::FH_VAR_BITS) != 0;
    bool    swapEndian = (flags & ftFile::FH_ENDIAN_SWAP) != 0;

    ftFile::Chunk64 c64;
    ftFile::Chunk32 c32;
    ftFile::Chunk*  cpy;

    if (FT_VOID8)
    {
        if (bitsVary)
        {
            ftFile::Chunk32 src;
            if ((bytesRead = stream->read(&src, Block32)) <= 0)
            {
                ftINVALID_READ;
                return ftFile::FS_INV_READ;
            }

            c64.m_code = src.m_code;
            c64.m_len  = src.m_len;
            union {
                FBTuint64 m_ptr;
                FBTuint32 m_doublePtr[2];
            } ptr;
            ptr.m_doublePtr[0] = src.m_old;
            ptr.m_doublePtr[1] = 0;

            c64.m_old    = ptr.m_ptr;
            c64.m_typeid = src.m_typeid;
            c64.m_nr     = src.m_nr;
        }
        else
        {
            if ((bytesRead = stream->read(&c64, BlockSize)) <= 0)
            {
                ftINVALID_READ;
                return ftFile::FS_INV_READ;
            }
        }

        if (swapEndian)
        {
            if ((c64.m_code & 0xFFFF) == 0)
                c64.m_code >>= 16;
            c64.m_len    = ftSwap32(c64.m_len);
            c64.m_nr     = ftSwap32(c64.m_nr);
            c64.m_typeid = ftSwap32(c64.m_typeid);
        }
        cpy = (ftFile::Chunk*)(&c64);
    }
    else
    {
        if (bitsVary)
        {
            ftFile::Chunk64 src;
            if ((bytesRead = stream->read(&src, Block64)) <= 0)
            {
                ftINVALID_READ;
                return ftFile::FS_INV_READ;
            }
            c32.m_code = src.m_code;
            c32.m_len  = src.m_len;
            union {
                FBTuint64 m_ptr;
                FBTuint32 m_doublePtr[2];
            } ptr;
            ptr.m_doublePtr[0] = 0;
            ptr.m_doublePtr[1] = 0;
            ptr.m_ptr          = src.m_old;

            c32.m_old    = ptr.m_doublePtr[0] != 0 ? ptr.m_doublePtr[0] : ptr.m_doublePtr[1];
            c32.m_typeid = src.m_typeid;
            c32.m_nr     = src.m_nr;
        }
        else
        {
            if ((bytesRead = stream->read(&c32, BlockSize)) <= 0)
            {
                ftINVALID_READ;
                return ftFile::FS_INV_READ;
            }
        }
        if (swapEndian)
        {
            if ((c32.m_code & 0xFFFF) == 0)
                c32.m_code >>= 16;
            c32.m_len    = ftSwap32(c32.m_len);
            c32.m_nr     = ftSwap32(c32.m_nr);
            c32.m_typeid = ftSwap32(c32.m_typeid);
        }
        cpy = (ftFile::Chunk*)(&c32);
    }

    if (cpy->m_len == SK_NPOS32)
    {
        ftINVALID_LEN;
        return ftFile::FS_INV_LENGTH;
    }

    ::memcpy(dest, cpy, BlockSize);
    return bytesRead;
}

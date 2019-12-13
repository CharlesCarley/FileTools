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
#include "ftPlatformHeaders.h"
#include "ftStreams.h"
#include "ftTables.h"



// Common Identifiers
const FBTuint32 ENDB = ftID('E', 'N', 'D', 'B');
const FBTuint32 DNA1 = ftID('D', 'N', 'A', '1');
const FBTuint32 DATA = ftID('D', 'A', 'T', 'A');
const FBTuint32 SDNA = ftID('S', 'D', 'N', 'A');


// Compile asserts
ftASSERTCOMP(ChunkLen32, sizeof(ftFile::Chunk32) == 20);
ftASSERTCOMP(ChunkLen64, sizeof(ftFile::Chunk64) == 24);

#if ftARCH == ftARCH_32
ftASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk32) == sizeof(ftFile::Chunk));
#else
ftASSERTCOMP(ChunkLenNative, sizeof(ftFile::Chunk64) == sizeof(ftFile::Chunk));
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
    static int read(ftFile::Chunk* dest, ftStream* stream, int flags);
    static int write(ftFile::Chunk* src, ftStream* stream);
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
    clearStorage();
}


ftStream* ftFile::openStream(const char* path, int mode)
{
    ftStream* stream = 0;
    if (mode == PM_UNCOMPRESSED || mode == PM_COMPRESSED)
    {
#if ftUSE_GZ_FILE == 1
        if (mode == PM_COMPRESSED)
            stream = new ftGzStream();
        else
#endif
            stream = new ftFileStream();
        stream->open(path, ftStream::SM_READ);
    }
    else
    {
        stream = new ftMemoryStream();
        stream->open(path, ftStream::SM_READ);
    }
    return stream;
}



int ftFile::load(const char* path, int mode)
{
    ftStream* stream = 0;
    if (path == 0 || !(*path))
    {
        printf("Path name must not be null\n");
        return FS_FAILED;
    }

    if (mode == PM_COMPRESSED)
    {
        ftFileStream fs;
        fs.open(path, ftStream::SM_READ);

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
    ftMemoryStream ms;
    ms.open(memory, sizeInBytes, ftStream::SM_READ, mode == PM_COMPRESSED);
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

    if (tableData != 0 && tableSize > 0 && tableSize != ftNPOS)
        return tables->read(tableData, tableSize, false) ? (int)FS_OK : (int)FS_FAILED;
    return FS_FAILED;
}


int ftFile::parseHeader(ftStream* stream)
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
        if (ftVOID4)
            m_hederFlags |= FH_VAR_BITS;
    }
    else if (ftVOID8)
        m_hederFlags |= FH_VAR_BITS;

    if (*(headerMagic++) == FM_BIG_ENDIAN)
    {
        if (ftENDIAN_IS_LITTLE)
            m_hederFlags |= FH_ENDIAN_SWAP;
    }
    else if (ftENDIAN_IS_BIG)
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
    if (m_curFile)
        ::free(m_curFile);
    m_curFile = 0;
    if (m_fileData)
        ::free(m_fileData);
    m_fileData = 0;

    MemoryChunk *node = (MemoryChunk*)m_chunks.first,
                *tnd;

    while (node)
    {
        if (node->m_block)
            ::free(node->m_block);
        node->m_block = 0;

        if (node->m_newBlock)
            ::free(node->m_newBlock);

        node->m_newBlock = 0;
        tnd  = node;
        node = node->m_next;
        ::free(tnd);
    }

    delete m_file;
    m_file = 0;


    if (m_fileData)
        free(m_fileData);
    m_fileData = 0;

    delete m_memory;
    m_memory = 0;

}


int ftFile::parseStreamImpl(ftStream* stream)
{


    int status, bytesRead = 0;
   
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
            stream->seek(-bytesRead, SEEK_CUR);
            chunk.m_len = stream->size() - stream->position();
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
            if (m_map.find(chunk.m_old) != ftNPOS)
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
    ftStruct::Members::PointerType md = strc->m_members.ptr();
    FBTsizeType                i, s = strc->m_members.size();

    FBTuint32 k1 = member->m_val.k32[0];
    for (i = 0; i < s; i++)
    {
        ftStruct* strc2 = &md[i];
        if (strc2->m_nr == member->m_nr && strc2->m_dp == member->m_dp)
        {
            if (strc2->m_val.k32[1] == member->m_val.k32[1])
            {
                if (!strc2->m_keyChain.equals(member->m_keyChain))
                    continue;

                FBTuint32 k2 = strc2->m_val.k32[0];
                if (k1 == k2)
                    return strc2;

                if (isPointer)
                    continue;

                if (ftAtomicUtils::isInteger(k1) && ftAtomicUtils::isInteger(k2))
                    return strc2;

                if (ftAtomicUtils::isNumeric(k1) && ftAtomicUtils::isNumeric(k2))
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
    ftBinTables::OffsM::PointerType md = m_mp->getOffsetPtr();
    ftBinTables::OffsM::PointerType fd = m_fp->getOffsetPtr();

    FBTsizeType                i, i2;
    ftStruct::Members::PointerType p2;
    for (i = 0; i < m_mp->getOffsetCount(); ++i)
    {
        ftStruct* strc = md[i];
        strc->m_link   = find(m_mp->m_type[strc->m_key.k16[0]].m_name);

        if (strc->m_link)
            strc->m_link->m_link = strc;

        p2 = strc->m_members.ptr();
        for (i2 = 0; i2 < strc->m_members.size(); ++i2)
        {
            ftStruct* member = &strc->m_members[i2];
            ftASSERT(!member->m_link);

            member->m_flag |= strc->m_link ? 0 : ftStruct::MISSING;
            if (!(member->m_flag & ftStruct::MISSING))
            {
                ftASSERT(member->m_key.k16[1] < m_mp->m_nameNr);

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


int ftFile::link(void)
{
    ftBinTables::OffsM::PointerType md = m_memory->getOffsetPtr();
    ftBinTables::OffsM::PointerType fd = m_file->getOffsetPtr();

    FBTsizeType                s2, i2, a2, n;
    ftStruct::Members::PointerType p2;
    FBTsize                    mlen, malen, total, pi, mptrsz;

    char *   dst, *src;
    FBTsize *dstPtr, *srcPtr;

    bool endianSwap = (m_hederFlags & FH_ENDIAN_SWAP) != 0;

    static const FBThash hk = ftCharHashKey("Link").hash();

    MemoryChunk* node;
    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_chunk.m_typeid > m_file->m_strcNr || !(fd[node->m_chunk.m_typeid]->m_link))
            continue;

        ftStruct *fs, *ms;
        fs = fd[node->m_chunk.m_typeid];
        ms = fs->m_link;

        node->m_newTypeId = ms->m_strcId;

        if (m_memory->m_type[ms->m_key.k16[0]].m_typeId == hk)
        {
            FBTsize totSize  = node->m_chunk.m_len;
            node->m_newBlock = ::malloc(totSize);
            if (!node->m_newBlock)
            {
                ftMALLOC_FAILED;
                return FS_BAD_ALLOC;
            }

            ::memcpy(node->m_newBlock, node->m_block, totSize);
            continue;
        }

        if (skip(m_memory->m_type[ms->m_key.k16[0]].m_typeId))
            continue;

        FBTsize totSize     = (node->m_chunk.m_nr * ms->m_len);
        node->m_chunk.m_len = totSize;

        node->m_newBlock = ::malloc(totSize);
        if (!node->m_newBlock)
        {
            ftMALLOC_FAILED;
            return FS_BAD_ALLOC;
        }

        ::memset(node->m_newBlock, 0, totSize);
    }

    FBTuint8 mps = m_memory->getTablePtrSize(), fps = m_file->getTablePtrSize();

    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
    {
        if (node->m_newTypeId > m_memory->m_strcNr)
            continue;

        ftStruct* cs = md[node->m_newTypeId];
        if (m_memory->m_type[cs->m_key.k16[0]].m_typeId == hk)
            continue;

        if (!cs->m_link || skip(m_memory->m_type[cs->m_key.k16[0]].m_typeId) || !node->m_newBlock)
        {
            ::free(node->m_newBlock);
            node->m_newBlock = 0;
            continue;
        }

        s2 = cs->m_members.size();
        p2 = cs->m_members.ptr();
        for (n = 0; n < node->m_chunk.m_nr; ++n)
        {
            dst = static_cast<char*>(node->m_newBlock) + (cs->m_len * n);
            src = static_cast<char*>(node->m_block) + (cs->m_link->m_len * n);

            for (i2 = 0; i2 < s2; ++i2)
            {
                ftStruct* dstStrc = &p2[i2];
                ftStruct* srcStrc = dstStrc->m_link;

                if (!srcStrc)
                    continue;

                dstPtr = reinterpret_cast<FBTsize*>(dst + dstStrc->m_off);
                srcPtr = reinterpret_cast<FBTsize*>(src + srcStrc->m_off);

                const ftName& nameD = m_memory->m_name[dstStrc->m_key.k16[1]];
                const ftName& nameS = m_file->m_name[srcStrc->m_key.k16[1]];
                if (nameD.m_ptrCount > 0)
                {
                    if ((*srcPtr))
                    {
                        if (nameD.m_ptrCount > 1)
                        {
                            MemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
                            if (bin)
                            {
                                if (bin->m_flag & MemoryChunk::BLK_MODIFIED)
                                    (*dstPtr) = (FBTsize)bin->m_newBlock;
                                else
                                {
                                    total  = bin->m_chunk.m_len / fps;
                                    mptrsz = total * sizeof(FBTuintPtr);

                                    FBTuintPtr* nptr = (FBTuintPtr*)::malloc(mptrsz);
                                    ::memset(nptr, 0, mptrsz);

                                    // Always use a 32 bit integer,
                                    // then offset + 2 for a 64 bit pointer.
                                    FBTuint32* optr = (FBTuint32*)bin->m_block;
                                    FBTuint32* mptr = (FBTuint32*)nptr;

                                    for (pi = 0; pi < total; pi++, optr += (fps == 4 ? 1 : 2))
                                        (*mptr++) = (FBTuintPtr)findPtr((size_t)*optr);

                                    (*dstPtr) = (size_t)(nptr);

                                    bin->m_chunk.m_len = total * mps;
                                    bin->m_flag |= MemoryChunk::BLK_MODIFIED;

                                    ::free(bin->m_newBlock);
                                    bin->m_newBlock = nptr;
                                }
                            }
                        }
                        else
                        {
                            malen         = nameD.m_arraySize > nameS.m_arraySize ? nameS.m_arraySize : nameD.m_arraySize;
                            FBTsize* dptr = (FBTsize*)dstPtr;

                            // always use 32 bit, then offset + 2 for 64 bit
                            // (Old pointers are sorted in this manor)
                            FBTuint32* sptr = (FBTuint32*)srcPtr;

                            for (a2 = 0; a2 < malen; ++a2, sptr += (fps == 4 ? 1 : 2))
                                dptr[a2] = (FBTsize)findPtr((FBTsize)*sptr);
                        }
                    }
                }
                else
                {
                    FBTsize dstElmSize = dstStrc->m_len / nameD.m_arraySize;
                    FBTsize srcElmSize = srcStrc->m_len / nameS.m_arraySize;

                    bool needCast = (dstStrc->m_flag & ftStruct::NEED_CAST) != 0;
                    bool needSwap = endianSwap && srcElmSize > 1;

                    if (!needCast && !needSwap && srcStrc->m_val.k32[0] == dstStrc->m_val.k32[0])  //same type
                    {
                        mlen = ftMin(srcStrc->m_len, dstStrc->m_len);
                        ::memcpy(dstPtr, srcPtr, mlen);
                        continue;
                    }

                    FBTbyte* dstBPtr = reinterpret_cast<FBTbyte*>(dstPtr);
                    FBTbyte* srcBPtr = reinterpret_cast<FBTbyte*>(srcPtr);

                    ftAtomic stp = ftAtomic::FT_ATOMIC_UNKNOWN, dtp = ftAtomic::FT_ATOMIC_UNKNOWN;
                    if (needCast || needSwap)
                    {
                        stp = ftAtomicUtils::getPrimitiveType(srcStrc->m_val.k32[0]);
                        dtp = ftAtomicUtils::getPrimitiveType(dstStrc->m_val.k32[0]);
                    }

                    FBTsize alen = ftMin(nameS.m_arraySize, nameD.m_arraySize);
                    FBTsize elen = ftMin(srcElmSize, dstElmSize);

                    FBTbyte tmpBuf[8] = {
                        0,
                    };
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
        }
        dataRead(node->m_newBlock, node->m_chunk);
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

void* ftFile::findPtr(const FBTsize& iptr)
{
    FBTsizeType i;
    if ((i = m_map.find(iptr)) != ftNPOS)
        return m_map.at(i)->m_newBlock;
    return 0;
}


ftFile::MemoryChunk* ftFile::findBlock(const FBTsize& iptr)
{
    FBTsizeType i;
    if ((i = m_map.find(iptr)) != ftNPOS)
        return m_map.at(i);
    return 0;
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
    ftStream* fs;

#if ftUSE_GZ_FILE == 1
    if (mode == PM_COMPRESSED)
        fs = new ftGzStream();
    else
#endif
    {
        fs = new ftFileStream();
    }

    fs->open(path, ftStream::SM_WRITE);


    FBTuint8 cp = ftVOID8 ? FM_64_BIT : FM_32_BIT;
    FBTuint8 ce = ((FBTuint8)ftGetEndian()) == ftENDIAN_IS_BIG ? FM_BIG_ENDIAN : FM_LITTLE_ENDIAN;

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



void ftFile::serialize(ftStream*   stream,
                       const char* id,
                       FBTuint32   code,
                       FBTsize     len,
                       void*       writeData)
{
    if (m_memory == 0)
        getMemoryTable();

    if (writeData == nullptr || len < sizeof(void*))
    {
        printf("Invalid write data\n");
        return;
    }

    FBTtype ft = m_memory->findTypeId(id);
    if (ft == -1)
    {
        printf("writeStruct: %s - not found", id);
        return;
    }
    serialize(stream, ft, code, len, writeData);
}


void ftFile::serialize(ftStream* stream,
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

void ftFile::serialize(ftStream* stream, FBTsize len, void* writeData, int nr)
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
    ch.m_len    = len * nr;
    ch.m_nr     = 1;
    ch.m_old    = (FBTsize)writeData;
    ch.m_typeid = m_memory->findTypeId("Link");
    ftChunk::write(&ch, stream);
}

int ftChunk::write(ftFile::Chunk* src, ftStream* stream)
{
    int size = 0;
    size += stream->write(src, BlockSize);
    size += stream->write((void*)src->m_old, src->m_len);
    return size;
}


int ftChunk::read(ftFile::Chunk* dest, ftStream* stream, int flags)
{
    int  bytesRead  = 0;
    bool bitsVary   = (flags & ftFile::FH_VAR_BITS) != 0;
    bool swapEndian = (flags & ftFile::FH_ENDIAN_SWAP) != 0;

    ftFile::Chunk64 c64;
    ftFile::Chunk32 c32;
    ftFile::Chunk*  cpy;

    if (ftVOID8)
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

    if (cpy->m_len == ftNPOS)
    {
        ftINVALID_LEN;
        return ftFile::FS_INV_LENGTH;
    }

    ::memcpy(dest, cpy, BlockSize);
    return bytesRead;
}

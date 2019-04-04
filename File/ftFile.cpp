/*
-------------------------------------------------------------------------------
    Copyright (c) 2010 Charlie C & Erwin Coumans.

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
#define ftIN_SOURCE

#include "ftFile.h"
#include "ftStreams.h"
#include "ftTables.h"
#include "ftPlatformHeaders.h"

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

#define ftMALLOC_FAILED   ftPrintf("Failed to allocate memory!\n");
#define ftINVALID_READ    ftPrintf("Invalid read!\n");
#define ftINVALID_LEN     ftPrintf("Invalid block length!\n");
#define ftINVALID_INS     ftPrintf("Table insertion failed!\n");
#define ftLINK_FAILED     ftPrintf("Linking failed!\n");


struct ftChunk
{
	enum Size
	{
		BlockSize   = sizeof (ftFile::Chunk),
		Block32     = sizeof (ftFile::Chunk32),
		Block64     = sizeof (ftFile::Chunk64),
	};
	static int read(ftFile::Chunk* dest, ftStream* stream, int flags);
	static int write(ftFile::Chunk* src, ftStream* stream);
};


ftFile::ftFile(const char* uid)
	:   m_version(-1), 
        m_fileVersion(0), 
        m_fileHeader(0),
        m_uhid(uid), 
        m_aluhid(0),
	    m_memory(0), 
        m_file(0), 
        m_curFile(0)
{
}



ftFile::~ftFile()
{
	if (m_curFile)
		ftFree(m_curFile);
	m_curFile = 0;

	MemoryChunk* node = (MemoryChunk*)m_chunks.first, *tnd;
	while (node)
	{
		if (node->m_block)
			ftFree(node->m_block);
		if (node->m_newBlock)
			ftFree(node->m_newBlock);

        tnd  = node;
		node = node->m_next;
		ftFree(tnd);
	}

	delete m_file;
	delete m_memory;
}



int ftFile::parse(const char* path, int mode)
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

	if (!stream->isOpen())
	{
		ftPrintf("File '%s' loading failed\n", path);
		return FS_FAILED;
	}

	if (m_curFile)
		ftFree(m_curFile);


	FBTsize pl = strlen(path);
	m_curFile = (char*)ftMalloc(pl + 1);
	if (m_curFile)
	{
		ftMemcpy(m_curFile, path, pl);
		m_curFile[pl] = 0;
	}

	int result = parseStreamImpl(stream);
	delete stream;
	return result;
}



int ftFile::parse(const void* memory, FBTsize sizeInBytes, int mode, bool suppressHeaderWarning)
{
	ftMemoryStream ms;
	ms.open( memory, sizeInBytes, ftStream::SM_READ, mode==PM_COMPRESSED );

	if (!ms.isOpen())
	{
		ftPrintf("Memory %p(%i) loading failed\n", memory, sizeInBytes);
		return FS_FAILED;
	}

	return parseStreamImpl(&ms,suppressHeaderWarning);
}



int ftFile::parseHeader(ftStream* stream, bool suppressHeaderWarning)
{
	m_header.resize(12);
	stream->read(m_header.ptr(), 12);

	if (!ftCharNEq(m_header.c_str(), m_uhid, 6) && !ftCharNEq(m_header.c_str(), m_aluhid, 7))
	{
		if (!suppressHeaderWarning)
			ftPrintf("Unknown header ID '%s'\n", m_header.c_str());
		return FS_INV_HEADER_STR;
	}

	char* headerMagic = (m_header.ptr() + 7);

	m_fileHeader = 0;
	m_fileVersion = 0;

	if (*(headerMagic++) == FM_64_BIT)
	{
		m_fileHeader |= FH_CHUNK_64;
		if (ftVOID4)
			m_fileHeader |= FH_VAR_BITS;
	}
	else if (ftVOID8)
		m_fileHeader |= FH_VAR_BITS;


	if (*(headerMagic++) == FM_BIG_ENDIAN)
	{
		if (ftENDIAN_IS_LITTLE)
			m_fileHeader |= FH_ENDIAN_SWAP;
	}
	else if (ftENDIAN_IS_BIG)
		m_fileHeader |= FH_ENDIAN_SWAP;


	m_fileVersion = atoi(headerMagic);

	return FS_OK;
}


int ftFile::initializeMemory(void)
{
    int status = FS_FAILED;
    if (!m_memory)
    {
        m_memory = new ftBinTables();

        status = initializeTables(m_memory);
        if (status != FS_OK)
        {
            ftPrintf("Failed to initialize builtin tables\n");
            return status;
        }
    }

    return status;
}



int ftFile::parseStreamImpl(ftStream* stream, bool suppressHeaderWarning)
{
	int status;

	status = parseHeader(stream,suppressHeaderWarning);
	if (status != FS_OK)
	{
		ftPrintf("Failed to extract header!\n");
		return status;
	}

    status = initializeMemory();
    if (status != FS_OK)
        return status;

	if (m_file)
	{
		delete m_file;
		m_file = 0;
	}


	// preallocate table
	m_map.reserve(ftDefaultAlloc);


	Chunk chunk;


	// Scan chunks

	do
	{
		if ((status = ftChunk::read(&chunk, stream, m_fileHeader)) <= 0)
		{
			ftINVALID_READ;
			return FS_INV_READ;
		}


		if (chunk.m_code == SDNA)
		{
			chunk.m_code = DNA1;
			stream->seek(-status, SEEK_CUR);
			chunk.m_len = stream->size() - stream->position();
		}

		// exit on end byte
		if (chunk.m_code == ENDB)
			break;


		void* curPtr = ftMalloc(chunk.m_len);
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
			m_file = new ftBinTables(curPtr, chunk.m_len);
			m_file->m_ptr = m_fileHeader & FH_CHUNK_64 ? 8 : 4;


			if (!m_file->read((m_fileHeader & FH_ENDIAN_SWAP) != 0))
			{
				ftPrintf("Failed to initialize tables\n");
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
#if ftASSERT_INSERT
			FBTsizeType pos;
			if ((pos = m_map.find(chunk.m_old)) != ftNPOS)
			{
				ftFree(curPtr);
				curPtr = 0;
				int result = ftMemcmp(&m_map.at(pos)->m_chunk, &chunk, ftChunk::BlockSize);
				if (result != 0)
				{
					ftINVALID_READ;
					return FS_INV_READ;
				}
			}
#else
			if (m_map.find(chunk.m_old) != ftNPOS)
			{
				//printf("free  curPtr: 0x%x\n", curPtr);
				ftFree(curPtr);
				curPtr = 0;
			}
#endif
			else
			{
				MemoryChunk* bin = static_cast<MemoryChunk*>(ftMalloc(sizeof(MemoryChunk)));
				if (!bin)
				{
					ftMALLOC_FAILED;
					return FS_BAD_ALLOC;
				}
				ftMemset(bin, 0, sizeof(MemoryChunk));
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
	}
	while (!stream->eof());
	return status;
}


class ftPrimType
{
public:

};

class ftLinkCompiler
{
public:
	ftBinTables* m_mp;
	ftBinTables* m_fp;

	ftStruct* find(const ftCharHashKey& kvp);
	ftStruct* find(ftStruct* strc, ftStruct* member, bool isPointer, bool& needCast);
	int        link(void);
};



ftStruct* ftLinkCompiler::find(const ftCharHashKey& kvp)
{
	FBTtype i;
	if ( (i = m_fp->findTypeId(kvp)) != ((FBTtype)-1))
		return m_fp->m_offs.at(i);
	return 0;
}


ftStruct* ftLinkCompiler::find(ftStruct* strc, ftStruct* member, bool isPointer, bool& needCast)
{
	ftStruct::Members::Pointer md = strc->m_members.ptr();
	FBTsizeType i, s = strc->m_members.size();

	FBTuint32 k1 = member->m_val.k32[0];

	for (i = 0; i < s; i++)
	{
		ftStruct* strc2 = &md[i];
		if (strc2->m_nr == member->m_nr && strc2->m_dp ==  member->m_dp)
		{
			if (strc2->m_val.k32[1] == member->m_val.k32[1])
			{				
				if (!strc2->m_keyChain.equal(member->m_keyChain))
					continue;

				FBTuint32 k2 = strc2->m_val.k32[0];
				if (k1 == k2)
					return strc2;

				if (isPointer)
					continue;

				if (ftIsIntType(k1) && ftIsIntType(k2))
					return strc2;

				if (ftIsNumberType(k1) && ftIsNumberType(k2))
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
	ftBinTables::OffsM::Pointer md = m_mp->m_offs.ptr();
	ftBinTables::OffsM::Pointer fd = m_fp->m_offs.ptr();

	FBTsizeType i, i2; 
	ftStruct::Members::Pointer p2;


	for (i = 0; i < m_mp->m_offs.size(); ++i)
	{
		ftStruct* strc = md[i];
		strc->m_link = find(m_mp->m_type[strc->m_key.k16[0]].m_name);

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
				bool needCast = false;
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

void copyValues(FBTbyte* srcPtr, FBTbyte* dstPtr, FBTsize srcElmSize, FBTsize dstElmSize, FBTsize len)
{
	FBTsize i;
	for (i = 0; i < len; i++)
	{
		ftMemcpy(srcPtr, dstPtr, srcElmSize);
		srcPtr += srcElmSize;
		dstElmSize += dstElmSize;
	}
}

void castValue(FBTsize* srcPtr, FBTsize* dstPtr, ftPRIM_TYPE srctp, ftPRIM_TYPE dsttp, FBTsize len)
{
#define GET_V(value, current, type, cast, ptr, match) \
	if (current == type) \
	{ \
		value = (*(cast*)ptr); \
		ptr += sizeof(cast); \
		if (++match >= 2) continue; \
	}

#define SET_V(value, current, type, cast, ptr, match) \
	if (current == type) \
	{ \
		(*(cast*)ptr) = (cast)value; \
		ptr += sizeof(cast); \
		if (++match >= 2) continue; \
	}
	double value = 0.0;

	FBTsizeType i;
	for (i = 0; i < len; i++)
	{
		int match = 0;
		GET_V(value, srctp, ftPRIM_CHAR,    char,            srcPtr, match);
		SET_V(value, dsttp, ftPRIM_CHAR,    char,            dstPtr, match);
		GET_V(value, srctp, ftPRIM_SHORT,   short,           srcPtr, match);
		SET_V(value, dsttp, ftPRIM_SHORT,   short,           dstPtr, match);
		GET_V(value, srctp, ftPRIM_USHORT,  unsigned short,  srcPtr, match);
		SET_V(value, dsttp, ftPRIM_USHORT,  unsigned short,  dstPtr, match);
		GET_V(value, srctp, ftPRIM_INT,     int,             srcPtr, match);
		SET_V(value, dsttp, ftPRIM_INT,     int,             dstPtr, match);
		GET_V(value, srctp, ftPRIM_LONG,    int,             srcPtr, match);
		SET_V(value, dsttp, ftPRIM_LONG,    int,             dstPtr, match);
		GET_V(value, srctp, ftPRIM_FLOAT,   float,           srcPtr, match);
		SET_V(value, dsttp, ftPRIM_FLOAT,   float,           dstPtr, match);
		GET_V(value, srctp, ftPRIM_DOUBLE,  double,          srcPtr, match);
		SET_V(value, dsttp, ftPRIM_DOUBLE,  double,          dstPtr, match);
	}
#undef GET_V
#undef SET_V
}


int ftFile::link(void)
{
	ftBinTables::OffsM::Pointer md = m_memory->m_offs.ptr();
	ftBinTables::OffsM::Pointer fd = m_file->m_offs.ptr();
	FBTsizeType s2, i2, a2, n;
	ftStruct::Members::Pointer p2;
	FBTsize mlen, malen, total, pi;

	char* dst, *src;
	FBTsize* dstPtr, *srcPtr;

	bool endianSwap = (m_fileHeader & FH_ENDIAN_SWAP) != 0;

	static const FBThash hk = ftCharHashKey("Link").hash();


	MemoryChunk* node;
	for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
	{
		if (node->m_chunk.m_typeid > m_file->m_strcNr || !( fd[node->m_chunk.m_typeid]->m_link))
			continue;

		ftStruct* fs, *ms;
		fs = fd[node->m_chunk.m_typeid];
		ms = fs->m_link;

		node->m_newTypeId = ms->m_strcId;

		if (m_memory->m_type[ms->m_key.k16[0]].m_typeId == hk)
		{
			FBTsize totSize = node->m_chunk.m_len;
			node->m_newBlock = ftMalloc(totSize);
			if (!node->m_newBlock)
			{
				ftMALLOC_FAILED;
				return FS_BAD_ALLOC;
			}

			ftMemcpy(node->m_newBlock, node->m_block, totSize);
			continue;
		}

        if (skip(m_memory->m_type[ms->m_key.k16[0]].m_typeId))
			continue;


		FBTsize totSize = (node->m_chunk.m_nr * ms->m_len);
		node->m_chunk.m_len = totSize;

        node->m_newBlock = ftMalloc(totSize);
		if (!node->m_newBlock)
		{
			ftMALLOC_FAILED;
			return FS_BAD_ALLOC;
		}

        ftMemset(node->m_newBlock, 0, totSize);
	}



	FBTuint8 mps = m_memory->m_ptr, fps = m_file->m_ptr;
	for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
	{
		if (node->m_newTypeId > m_memory->m_strcNr)
			continue;

        ftStruct* cs = md[node->m_newTypeId];
		if (m_memory->m_type[cs->m_key.k16[0]].m_typeId == hk)
			continue;

		if (!cs->m_link || skip(m_memory->m_type[cs->m_key.k16[0]].m_typeId) || !node->m_newBlock)
		{
			ftFree(node->m_newBlock);
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
						if (nameD.m_ptrCount  > 1)
						{
							MemoryChunk* bin = findBlock((FBTsize)(*srcPtr));
							if (bin)
							{
								if (bin->m_flag & MemoryChunk::BLK_MODIFIED)
									(*dstPtr) = (FBTsize)bin->m_newBlock;
								else
								{
									total = bin->m_chunk.m_len / fps;
									FBTsize* nptr = (FBTsize*)ftMalloc(total * mps);
									ftMemset(nptr, 0, total * mps);

									// always use 32 bit, then offset + 2 for 64 bit 
                                    // (Old pointers are sorted in this manor)
									FBTuint32* optr = (FBTuint32*)bin->m_block;

                                    for (pi = 0; pi < total; pi++, optr += (fps == 4 ? 1 : 2))
										nptr[pi] = (FBTsize)findPtr((FBTsize) * optr);

									(*dstPtr) = (FBTsize)(nptr);

									bin->m_chunk.m_len = total * mps;
									bin->m_flag |= MemoryChunk::BLK_MODIFIED;

									ftFree(bin->m_newBlock);
									bin->m_newBlock = nptr;
								}
							}
							else
							{
								//ftPrintf("**block not found @ 0x%p)\n", src);
							}
						}
						else
						{
							malen = nameD.m_arraySize > nameS.m_arraySize ? nameS.m_arraySize : nameD.m_arraySize;

							FBTsize* dptr = (FBTsize*)dstPtr;

							// always use 32 bit, then offset + 2 for 64 bit 
                            // (Old pointers are sorted in this manor)
							FBTuint32* sptr = (FBTuint32*)srcPtr;


							for (a2 = 0; a2 < malen; ++a2, sptr += (fps == 4 ? 1 : 2))
								dptr[a2] = (FBTsize)findPtr((FBTsize) * sptr);
						}
					}
				}
				else
				{
					FBTsize dstElmSize = dstStrc->m_len / nameD.m_arraySize;
					FBTsize srcElmSize = srcStrc->m_len / nameS.m_arraySize;

					bool needCast = (dstStrc->m_flag & ftStruct::NEED_CAST) != 0;
					bool needSwap = endianSwap && srcElmSize > 1;

					if (!needCast && !needSwap && srcStrc->m_val.k32[0] == dstStrc->m_val.k32[0]) //same type
					{						
						mlen = ftMin(srcStrc->m_len, dstStrc->m_len);
						ftMemcpy(dstPtr, srcPtr, mlen);
						continue;
					}

					FBTbyte* dstBPtr = reinterpret_cast<FBTbyte*>(dstPtr);
					FBTbyte* srcBPtr = reinterpret_cast<FBTbyte*>(srcPtr);

					ftPRIM_TYPE stp = ftPRIM_UNKNOWN, dtp  = ftPRIM_UNKNOWN;

                    if (needCast || needSwap)
					{
						stp = ftGetPrimType(srcStrc->m_val.k32[0]);
						dtp = ftGetPrimType(dstStrc->m_val.k32[0]);

						ftASSERT(ftIsNumberType(stp) && ftIsNumberType(dtp) && stp != dtp);
					}

					FBTsize alen = ftMin(nameS.m_arraySize, nameD.m_arraySize);
					FBTsize elen = ftMin(srcElmSize, dstElmSize);

					FBTbyte tmpBuf[8] = {0, };
					FBTsize i;
					for (i = 0; i < alen; i++)
					{
						FBTbyte* tmp = srcBPtr;
						if (needSwap)
						{
							tmp = tmpBuf;
							ftMemcpy(tmpBuf, srcBPtr, srcElmSize);

							if (stp == ftPRIM_SHORT || stp == ftPRIM_USHORT) 
								ftSwap16((FBTuint16*)tmpBuf, 1);
							else if (stp >= ftPRIM_INT && stp <= ftPRIM_FLOAT) 
								ftSwap32((FBTuint32*)tmpBuf, 1);
							else if (stp == ftPRIM_DOUBLE)
								ftSwap64((FBTuint64*)tmpBuf, 1);
							else
								ftMemset(tmpBuf, 0, sizeof(tmpBuf)); //unknown type
						}
						
						if (needCast)
							castValue((FBTsize*)tmp, (FBTsize*)dstBPtr, stp, dtp, 1);
						else
							ftMemcpy(dstBPtr, tmp, elen);

						dstBPtr += dstElmSize;
						srcBPtr += srcElmSize;
					}
				}
			}
		}

		notifyData(node->m_newBlock, node->m_chunk);
	}

    for (node = (MemoryChunk*)m_chunks.first; node; node = node->m_next)
	{
		if (node->m_block)
		{
			ftFree(node->m_block);
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

bool ftFile::_setuid(const char* uid)
{
	if (!uid || strlen(uid) != 7) return false;

	m_uhid = uid;
	return true;
}

int ftFile::save(const char* path, const int mode, const ftEndian& endian)
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

	char header[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	char version[33];
	sprintf(version, "%i",m_version);
	
	strncpy(&header[0], m_uhid, 7); // 7 first bytes of header
	header[7] = cp;					// 8th byte = pointer size
	header[8] = ce;					// 9th byte = endian
	strncpy(&header[9], version, 3);// last 3 bytes v or 3 version char
	fs->write(header, 12);
	
	writeData(fs);

	// write DNA1
	Chunk ch;
	ch.m_code   = DNA1;
	ch.m_len    = getFBTlength();
	ch.m_nr     = 1;
	ch.m_old    = 0;
	ch.m_typeid = 0;
	fs->write(&ch, ftChunk::BlockSize);
	fs->write(getFBT(), ch.m_len);


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

void ftFile::writeStruct(ftStream* stream, FBTtype index, FBTuint32 code, FBTsize len, void* writeData)
{
	Chunk ch;
	ch.m_code   = code;
	ch.m_len    = len;
	ch.m_nr     = 1;
	ch.m_old    = (FBTsize)writeData;
	ch.m_typeid = index;

	ftChunk::write(&ch, stream);
}

void ftFile::writeBuffer(ftStream* stream, FBTsize len, void* writeData)
{
	Chunk ch;
	ch.m_code   = DATA;
	ch.m_len    = len;
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
	int bytesRead = 0;
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

			c64.m_code    = src.m_code;
			c64.m_len     = src.m_len;

			union
			{
				FBTuint64   m_ptr;
				FBTuint32   m_doublePtr[2];
			} ptr;
			ptr.m_doublePtr[0] = src.m_old;
			ptr.m_doublePtr[1] = 0;

			c64.m_old     = ptr.m_ptr;
			c64.m_typeid  = src.m_typeid;
			c64.m_nr      = src.m_nr;
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
			c32.m_code    = src.m_code;
			c32.m_len     = src.m_len;
			union
			{
				FBTuint64   m_ptr;
				FBTuint32   m_doublePtr[2];
			} ptr;
			ptr.m_doublePtr[0] = 0;
			ptr.m_doublePtr[1] = 0;
			ptr.m_ptr = src.m_old;

            c32.m_old       = ptr.m_doublePtr[0] != 0 ? ptr.m_doublePtr[0] : ptr.m_doublePtr[1];
			c32.m_typeid    = src.m_typeid;
			c32.m_nr        = src.m_nr;
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

	ftMemcpy(dest, cpy, BlockSize);
	return bytesRead;
}

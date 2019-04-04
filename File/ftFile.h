/*
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

#include "ftTypes.h"

class ftStream;
class ftBinTables;


class ftFile
{
public:
	enum FileMagic
	{
		FM_BIG_ENDIAN       = 'V',
		FM_LITTLE_ENDIAN    = 'v',
		FM_32_BIT           = '_',
		FM_64_BIT           = '-',

		// place custom rules here, note that the
		// full header string must be <= 12 chars
	};

    enum FileStatus
	{
		FS_LINK_FAILED = -7,
		FS_INV_INSERT,
		FS_BAD_ALLOC,
		FS_INV_READ,
		FS_INV_LENGTH,
		FS_INV_HEADER_STR,
		FS_FAILED,
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
		FH_ENDIAN_SWAP  = (1 << 0),
		FH_CHUNK_64     = (1 << 1),
		FH_VAR_BITS     = (1 << 2),
	};

    struct Chunk32
	{
		FBTuint32       m_code;
		FBTuint32       m_len;
		FBTuint32       m_old;
		FBTuint32       m_typeid;
		FBTuint32       m_nr;
	};

	struct Chunk64
	{
		FBTuint32       m_code;
		FBTuint32       m_len;
		FBTuint64       m_old;
		FBTuint32       m_typeid;
		FBTuint32       m_nr;
	};

    struct Chunk
	{
		FBTuint32       m_code;
		FBTuint32       m_len;
		FBTsize         m_old;
		FBTuint32       m_typeid;
		FBTuint32       m_nr;
	};

    struct MemoryChunk
	{
		enum Flag
		{
			BLK_MODIFIED = (1 << 0),
		};

		MemoryChunk* m_next, *m_prev;
		Chunk        m_chunk;
		void*        m_block;
		void*        m_newBlock;
		FBTuint8     m_flag;
		FBTtype      m_newTypeId;
	};

public:


	ftFile(const char* uid);
	virtual ~ftFile();

    int parse(const char* path, int mode = PM_UNCOMPRESSED);
	int parse(const void* memory, FBTsize sizeInBytes, int mode = PM_UNCOMPRESSED, bool suppressHeaderWarning=false);

	/// Saving in non native endianness is not implemented yet.
	int save(const char* path, const int mode = PM_UNCOMPRESSED, const ftEndian& endian = FileTools_ENDIAN_NATIVE);

    const ftFixedString<12>&    getHeader(void)     const {return m_header;}
	const int&                  getVersion(void)    const {return m_fileVersion;}
	const char*                 getPath(void)       const {return m_curFile; }


	ftBinTables* getMemoryTable(void)  {
        if (!m_memory)
            initializeMemory();
        return m_memory;
    }
	ftBinTables* getFileTable(void)    {return m_file;}

	FileTools_INLINE ftList& getChunks(void) {return m_chunks;}
    virtual void setIgnoreList(FBTuint32 *stripList) {}

	bool _setuid(const char* uid);



    void writeStruct(ftStream* stream, FBTtype index, FBTuint32 code, FBTsize len, void* writeData);
    void writeBuffer(ftStream* stream, FBTsize len, void* writeData);

protected:

	virtual int         initializeTables(ftBinTables* tables) = 0;
    virtual int         initializeMemory(void);

    virtual void*       getFBT(void) = 0;
    virtual FBTsize     getFBTlength(void) = 0;
    virtual int         notifyData(void* p, const Chunk& id) = 0;
    virtual int         writeData(ftStream* stream) = 0;

    // lookup name first 7 of 12
	const char*         m_uhid;
	const char*         m_aluhid; //alternative header string
	ftFixedString<12>   m_header;

    int m_version, m_fileVersion, m_fileHeader;
	char* m_curFile;

	typedef ftHashTable<ftSizeHashKey, MemoryChunk*> ChunkMap;
	ftList     m_chunks;
	ChunkMap    m_map;
	ftBinTables* m_memory, *m_file;


	virtual bool skip(const FBTuint32& id) {return false;}

	void* findPtr(const FBTsize& iptr);
	MemoryChunk* findBlock(const FBTsize& iptr);

private:


	int parseHeader(ftStream* stream, bool suppressHeaderWarning=false);
	int parseStreamImpl(ftStream* stream, bool suppressHeaderWarning=false);

	int compileOffsets(void);
	int link(void);
};

#endif//_ftFile_h_

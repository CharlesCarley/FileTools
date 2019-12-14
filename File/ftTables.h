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
#ifndef _ftTables_h_
#define _ftTables_h_

#include "Utils/skArray.h"
#include "Utils/skMap.h"
#include "ftHashTypes.h"
#include "ftTypes.h"



namespace ftIdNames
{
    const char ftSDNA[4] = {'S', 'D', 'N', 'A'};
    const char ftNAME[4] = {'N', 'A', 'M', 'E'};  // Name array
    const char ftTYPE[4] = {'T', 'Y', 'P', 'E'};  // Type Array
    const char ftTLEN[4] = {'T', 'L', 'E', 'N'};  // Type length array
    const char ftSTRC[4] = {'S', 'T', 'R', 'C'};  // Struct/Class Array
    const char ftOFFS[4] = {'O', 'F', 'F', 'S'};  // Offset map (Optional & TODO)
}  // namespace ftIdNames


FT_INLINE ftFixedString<4> ftByteToString(FBTuint32 i)
{
    union {
        char      ids[4];
        FBTuint32 idi;
    } IDU;
    IDU.idi = i;
    ftFixedString<4> cp;
    cp.push_back(IDU.ids[0]);
    cp.push_back(IDU.ids[1]);
    cp.push_back(IDU.ids[2]);
    cp.push_back(IDU.ids[3]);
    return cp;
}


typedef struct ftName
{
    char*     m_name;  // note: memory is in the main table.
    int       m_loc;
    FBTuint32 m_nameId;
    int       m_ptrCount;
    int       m_numSlots, m_isFptr;
    int       m_arraySize;
    int       m_slots[FT_ARR_DIM_MAX];
} ftName;

typedef struct ftType
{
    char*     m_name;    // note: memory is in the main table.
    FBTuint32 m_typeId;  // ftCharHashKey(typeName)
    FBTuint32 m_strcId;
} ftType;



typedef union ftKey32 {
    FBTint16 k16[2];
    FBTint32 k32;
} ftKey32;

typedef union ftKey64 {
    FBTuint32 k32[2];
    FBTuint64 k64;

} ftKey64;


class ftStruct
{
public:
    typedef skArray<ftStruct> Members;
    typedef skArray<ftKey64>  Keys;

    enum Flag
    {
        CAN_LINK   = 0,
        MISSING    = (1 << 0),
        MISALIGNED = (1 << 1),
        SKIP       = (1 << 2),
        NEED_CAST  = (1 << 3)
    };

public:
    ftStruct() :
        m_key(),
        m_val(),
        m_off(0),
        m_len(0),
        m_nr(0),
        m_dp(0),
        m_strcId(0),
        m_flag(0),
        m_members(),
        m_link(0)
    {
    }

    ~ftStruct()
    {
    }


    inline const FBTint16& getTypeIndex() const
    {
        return m_key.k16[0];
    }

    inline const FBTint16& getNameIndex() const
    {
        return m_key.k16[1];
    }

    inline const FBTint32& getSizeInBytes() const
    {
        return m_len;
    }


    ftKey32   m_key;  // k[0]: type, k[1]: name
    ftKey64   m_val;  // key hash value, k[0]: type hash id, k[1]: member(field) base name hash id or 0(struct)
    FBTint32  m_off;  // offset
    FBTint32  m_len;
    FBTint32  m_nr, m_dp;  //nr: array index, dp: embedded depth
    FBTint32  m_strcId;
    FBTint32  m_flag;
    Members   m_members;
    ftStruct* m_link;      //file/memory table struct link
    Keys      m_keyChain;  //parent key hash chain(0: type hash, 1: name hash), size() == m_dp
};


class ftBinTables
{
public:
    typedef ftName*            Names;  // < FT_MAX_TABLE
    typedef ftType*            Types;  // < FT_MAX_TABLE
    typedef FBTtype*           TypeL;  // < FT_MAX_TABLE
    typedef FBTtype**          Strcs;  // < FT_MAX_TABLE * FT_MAX_MEMBERS;
    typedef skArray<FBTuint32> NameB;
    typedef skArray<ftStruct*> OffsM;

    typedef skHashTable<ftCharHashKey, ftType> TypeFinder;

public:
    ftBinTables();
    ftBinTables(void* ptr, FBTsize len, FBTuint8 ptrSize);
    ~ftBinTables();

    bool read(bool swap);
    bool read(const void* ptr, const FBTsize& len, bool swap);

    FBTtype     findTypeId(const ftCharHashKey& cp);
    const char* getStructType(const ftStruct* strc);
    const char* getStructName(const ftStruct* strc);
    const char* getOwnerStructName(const ftStruct* strc);


    OffsM::PointerType getOffsetPtr();
    OffsM::SizeType    getOffsetCount();

    // Access to the size of a pointer when it was saved in the table.
    // Which this is cheating a bit here, it depends on the correct flag
    // being set when loaded it's not actually being computed.
    inline FBTuint8 getTablePtrSize()
    {
        return m_ptrLength;
    }

    inline bool isValidType(const FBTuint32& typeidx) const
    {
        // look at this, i think they are the same...
        return typeidx < m_strcNr && typeidx < m_offs.size();
    }





    ftStruct* findStructByName(const ftCharHashKey& kvp);


    // accessors
    ftStruct* findStructByType(const FBTuint16& type);
    bool      isLinkedToMemory(const FBTuint16& type);

    FBTuint32 getTypeId(const FBTuint16& type)
    {
        if (type < m_typeNr)
            return m_type[type].m_typeId;
        return SK_NPOS32;
    }


    // make these private
    Names m_name;
    Types m_type;
    TypeL m_tlen;
    Strcs m_strc;
    NameB m_base;

    FBTuint32 m_nameNr;
    FBTuint32 m_typeNr;
    FBTuint32 m_strcNr;
    void*     m_otherBlock;
    FBTsize   m_otherLen;

private:
    OffsM m_offs;

    // It's safe to assume that memory len is FT_VOIDP and file len is FH_CHUNK_64 ? 8 : 4
    // Otherwise this library will not even compile (no more need for 'sizeof(ListBase) / 2')
    FBTuint8 m_ptrLength;

    TypeFinder m_typeFinder;

    void putMember(FBTtype* cp, ftStruct* off, FBTtype nr, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys);
    void compile(FBTtype i, FBTtype nr, ftStruct* off, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys);
    void compile(void);
};
#endif  //_ftTables_h_

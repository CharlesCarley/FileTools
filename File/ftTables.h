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
#ifndef _ftTables_h_
#define _ftTables_h_

#include "ftTypes.h"




namespace ftIdNames
{
    const char ftSDNA[4] = {'S', 'D', 'N', 'A'};
    const char ftNAME[4] = {'N', 'A', 'M', 'E'}; // Name array
    const char ftTYPE[4] = {'T', 'Y', 'P', 'E'}; // Type Array
    const char ftTLEN[4] = {'T', 'L', 'E', 'N'}; // Type length array
    const char ftSTRC[4] = {'S', 'T', 'R', 'C'}; // Struct/Class Array
    const char ftOFFS[4] = {'O', 'F', 'F', 'S'}; // Offset map (Optional & TODO)
}


ftINLINE ftFixedString<4> ftByteToString(FBTuint32 i)
{
    union
    {
        char        ids[4];
        FBTuint32   idi;
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
    char*           m_name;     // note: memory is in the raw table.
    int             m_loc;
    FBTuint32       m_nameId;
    int             m_ptrCount;
    int             m_numSlots, m_isFptr;
    int             m_arraySize;
    int             m_slots[ftARRAY_SLOTS];
} ftName;

typedef struct ftType
{
    char*           m_name;     // note: memory is in the raw table.
    FBTuint32       m_typeId;	// ftCharHashKey(typeName)
    FBTuint32       m_strcId;
} ftType;



typedef union ftKey32
{
    FBTint16 k16[2];
    FBTint32 k32;
} ftKey32;

typedef union ftKey64
{
    FBTuint32 k32[2];
    FBTuint64 k64;

} ftKey64;


class ftStruct
{
public:
    typedef ftArray<ftStruct> Members;
    typedef ftArray<ftKey64>  Keys;

    enum Flag
    {
        CAN_LINK    = 0,
        MISSING     = (1 << 0),
        MISALIGNED  = (1 << 1),
        SKIP        = (1 << 2),
        NEED_CAST	= (1 << 3)
    };


    ftStruct()
        :	m_key(),
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
    ~ftStruct()    {}


    ftKey32         m_key;		//k[0]: type, k[1]: name
    ftKey64         m_val;		//key hash value, k[0]: type hash id, k[1]: member(field) base name hash id or 0(struct)
    FBTint32        m_off;		//offset
    FBTint32        m_len;
    FBTint32        m_nr, m_dp; //nr: array index, dp: embedded depth
    FBTint32        m_strcId;
    FBTint32        m_flag;
    Members         m_members;
    ftStruct*       m_link;		//file/memory table struct link
    Keys            m_keyChain; //parent key hash chain(0: type hash, 1: name hash), size() == m_dp

    FBTsizeType     getUnlinkedMemberCount();
};


class ftBinTables
{
public:
    typedef ftName*    Names;  // < ftMaxTable
    typedef ftType*    Types;  // < ftMaxTable
    typedef FBTtype*   TypeL;  // < ftMaxTable
    typedef FBTtype**  Strcs;  // < ftMaxTable * ftMaxMember;


    // Base name trim (*[0-9]) for partial type, name matching
    // Example: M(char m_var[32]) F(char m_var[24])
    //
    //          result = M(char m_var[24] = F(char m_var[24]) then (M(char m_var[24->32]) = 0)
    //
    // (Note: bParse will skip m_var all together because of 'strcmp(Mtype, Ftype) && strcmp(Mname, Fname)')
    //
    typedef ftArray<FBTuint32>     NameB;
    typedef ftArray<ftStruct*>    OffsM;

    typedef ftHashTable<ftCharHashKey, ftType> TypeFinder;

public:

    ftBinTables();
    ftBinTables(void* ptr, const FBTsize& len);
    ~ftBinTables();

    bool read(bool swap);
    bool read(const void* ptr, const FBTsize& len, bool swap);

    FBTtype findTypeId(const ftCharHashKey& cp);

    const char* getStructType(const ftStruct* strc);
    const char* getStructName(const ftStruct* strc);
    const char* getOwnerStructName(const ftStruct* strc);


    Names   m_name;
    Types   m_type;
    TypeL   m_tlen;
    Strcs   m_strc;
    OffsM   m_offs;
    NameB   m_base;

    FBTuint32 m_nameNr;
    FBTuint32 m_typeNr;
    FBTuint32 m_strcNr;

    // It's safe to assume that memory len is ftVOID and file len is FH_CHUNK_64 ? 8 : 4
    // Otherwise this library will not even compile (no more need for 'sizeof(ListBase) / 2')
    FBTuint8    m_ptr;
    void*       m_otherBlock;
    FBTsize     m_otherLen;


private:

    TypeFinder m_typeFinder;

    void putMember(FBTtype* cp, ftStruct* off, FBTtype nr, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys);
    void compile(FBTtype i, FBTtype nr, ftStruct* off, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys);
    void compile(void);
    bool sikp(const FBTuint32& type);

};


/** @}*/
#endif//_ftTables_h_

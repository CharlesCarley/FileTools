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
#include "ftStruct.h"
#include "ftTypes.h"


namespace ftIdNames
{
    const char FT_SDNA[4] = {'S', 'D', 'N', 'A'};
    const char FT_NAME[4] = {'N', 'A', 'M', 'E'};  // Name array
    const char FT_TYPE[4] = {'T', 'Y', 'P', 'E'};  // Type Array
    const char FT_TLEN[4] = {'T', 'L', 'E', 'N'};  // Type length array
    const char FT_STRC[4] = {'S', 'T', 'R', 'C'};  // Struct/Class Array
    const char FT_OFFS[4] = {'O', 'F', 'F', 'S'};  // Offset map (Optional & TODO)


}  // namespace ftIdNames

extern ftFixedString<4> ftByteToString(FBTuint32 i);


class ftBinTables
{
public:
    typedef ftName*            Names;  // < FT_MAX_TABLE
    typedef ftType*            Types;  // < FT_MAX_TABLE
    typedef FBTtype*           TypeL;  // < FT_MAX_TABLE
    typedef FBTtype**          Strcs;  // < FT_MAX_TABLE * FT_MAX_MEMBERS;
    typedef skArray<SKsize>    NameB;
    typedef skArray<ftStruct*> OffsM;

    typedef skHashTable<ftCharHashKey, ftType> TypeFinder;


    static const ftName INVALID_NAME;
    static const ftType INVALID_TYPE;

public:
    ftBinTables();
    ftBinTables(void* ptr, FBTsize len, FBTuint8 ptrSize);
    ~ftBinTables();

    bool read(bool swap);
    bool read(const void* ptr, const FBTsize& len, bool swap);

    FBTuint32     findTypeId(const ftCharHashKey& cp);
    ftCharHashKey getStructHashByType(const FBTuint16& type);

    const ftName& getStructNameByIdx(const FBTuint16& idx) const;
    FBThash       getTypeHash(const FBTuint16& type) const;
    bool          isPointer(const FBTuint16& name) const;



    inline OffsM::Iterator getOffsetIterator()
    {
        return m_structures.iterator();
    }

    OffsM::PointerType getOffsetPtr();
    OffsM::SizeType    getOffsetCount();

    // Access to the size of a pointer when it was saved in the table.
    // Which this is cheating a bit here, it depends on the correct flag
    // being set when loaded it's not actually being computed.
    inline FBTuint8 getSizeofPointer()
    {
        return m_ptrLength;
    }

    inline bool isValidType(const FBTuint32& typeidx) const
    {
        return typeidx < m_strcNr && typeidx < m_structures.size();
    }

    ftStruct* findStructByName(const ftCharHashKey& kvp);
    ftStruct* findStructByType(const FBTuint16& type);
    bool      isLinkedToMemory(const FBTuint16& type);



    // Direct access to the table

    inline FBTuint32 getNumberOfNames() const
    {
        return m_nameNr;
    }

    inline const ftName& getNameAt(FBTuint32 idx) const
    {
        if (idx < m_nameNr)
            return m_name[idx];
        return INVALID_NAME;
    }

    inline const char* getStringNameAt(FBTuint32 idx) const
    {
        if (idx < m_nameNr)
            return m_name[idx].m_name;
        return "";
    }


    inline FBTuint32 getNumberOfTypes() const
    {
        return m_typeNr;
    }

    inline const ftType& getTypeAt(FBTuint32 idx) const
    {
        if (idx < m_typeNr)
            return m_type[idx];
        return INVALID_TYPE;
    }

    inline char* getTypeNameAt(FBTuint32 idx) const
    {
        if (idx < m_typeNr)
            return m_type[idx].m_name;
        return nullptr;
    }



    inline FBTuint32 getNumberOfTypeLengths() const
    {
        return m_typeNr;
    }

    inline const FBTtype& getTypeLengthAt(FBTuint32 idx) const
    {
        if (idx < m_typeNr)
            return m_tlen[idx];
        return SK_NPOS16;
    }

    inline FBTuint32 getNumberOfStructs() const
    {
        return m_strcNr;
    }

    inline FBTtype* getStructAt(FBTuint32 idx) const
    {
        if (idx < m_strcNr)
            return m_strc[idx];
        return 0;
    }


private:

    friend class ftStruct;
    friend class ftMember;

    Names m_name;
    Types m_type;
    TypeL m_tlen;
    Strcs m_strc;
    NameB m_base;

    FBTuint32 m_nameNr;
    FBTuint32 m_typeNr;
    FBTuint32 m_strcNr;
    void*     m_block;
    FBTsize   m_blockLen;
    FBTuint16 m_firstStruct;

    OffsM      m_structures;
    FBTuint8   m_ptrLength;
    TypeFinder m_typeFinder;

    void putMember(FBTtype*        cp,
                   ftStruct*       off,
                   FBTtype         nr,
                   FBTuint32&      cof,
                   FBTuint32       depth);

    void compile(FBTtype         i,
                 FBTtype         nr,
                 ftStruct*       off,
                 FBTuint32&      cof,
                 FBTuint32       depth);

    void compile(void);
};
#endif  //_ftTables_h_

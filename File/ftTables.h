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
#include "ftStruct.h"


namespace ftIdNames
{
    const char ftSDNA[4] = {'S', 'D', 'N', 'A'};
    const char ftNAME[4] = {'N', 'A', 'M', 'E'};  // Name array
    const char ftTYPE[4] = {'T', 'Y', 'P', 'E'};  // Type Array
    const char ftTLEN[4] = {'T', 'L', 'E', 'N'};  // Type length array
    const char ftSTRC[4] = {'S', 'T', 'R', 'C'};  // Struct/Class Array
    const char ftOFFS[4] = {'O', 'F', 'F', 'S'};  // Offset map (Optional & TODO)


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

public:
    ftBinTables();
    ftBinTables(void* ptr, FBTsize len, FBTuint8 ptrSize);
    ~ftBinTables();

    bool read(bool swap);
    bool read(const void* ptr, const FBTsize& len, bool swap);

    FBTuint32   findTypeId(const ftCharHashKey& cp);
    const char* getStructType(const ftStruct* strc);
    const char* getStructName(const ftStruct* strc);
    const char* getOwnerStructName(const ftStruct* strc);

    const ftName& getStructNameByIdx(const FBTuint16& idx) const;
    FBThash       getTypeHash(const FBTuint16& type) const;



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
        return typeidx < m_strcNr && typeidx < m_offs.size();
    }

    ftStruct* findStructByName(const ftCharHashKey& kvp);
    ftStruct* findStructByType(const FBTuint16& type);
    bool      isLinkedToMemory(const FBTuint16& type);



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
    OffsM      m_offs;
    FBTuint8   m_ptrLength;
    TypeFinder m_typeFinder;

    void putMember(FBTtype*        cp,
                   ftStruct*       off,
                   FBTtype         nr,
                   FBTuint32&      cof,
                   FBTuint32       depth,
                   ftStruct::Keys& keys);
    
    void compile(FBTtype         i,
                 FBTtype         nr,
                 ftStruct*       off,
                 FBTuint32&      cof,
                 FBTuint32       depth,
                 ftStruct::Keys& keys);

    void compile(void);
};
#endif  //_ftTables_h_

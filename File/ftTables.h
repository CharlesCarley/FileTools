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
#include "ftStreams.h"
#include "ftStruct.h"
#include "ftTypes.h"

namespace ftIdNames
{
    const char FT_SDNA[5] = {'S', 'D', 'N', 'A', '\0'};
    const char FT_NAME[5] = {'N', 'A', 'M', 'E', '\0'};  // Name array
    const char FT_TYPE[5] = {'T', 'Y', 'P', 'E', '\0'};  // Type Array
    const char FT_TLEN[5] = {'T', 'L', 'E', 'N', '\0'};  // Type length array
    const char FT_STRC[5] = {'S', 'T', 'R', 'C', '\0'};  // Struct/Class Array
    const char FT_OFFS[5] = {'O', 'F', 'F', 'S', '\0'};  // Offset map (Optional & TODO)

    const SKuint32 SDNA = FT_TYPEID('S', 'D', 'N', 'A');
    const SKuint32 DNA1 = FT_TYPEID('D', 'N', 'A', '1');
    const SKuint32 ENDB = FT_TYPEID('E', 'N', 'D', 'B');
    const SKuint32 DATA = FT_TYPEID('D', 'A', 'T', 'A');
    const SKuint32 TEST = FT_TYPEID('T', 'E', 'S', 'T');
}  // namespace ftIdNames

class ftTables
{
public:
    typedef ftName*                            Names;  // < FT_MAX_TABLE
    typedef ftType*                            Types;  // < FT_MAX_TABLE
    typedef SKtype*                            TypeL;  // < FT_MAX_TABLE
    typedef SKtype**                           Strcs;  // < FT_MAX_TABLE * FT_MAX_MEMBERS;
    typedef skArray<SKsize>                    NameHash;
    typedef skArray<ftStruct*>                 Structures;
    typedef skHashTable<ftCharHashKey, ftType> TypeFinder;

    static const ftName INVALID_NAME;
    static const ftType INVALID_TYPE;

public:
    ftTables(int pointerLength);
    ~ftTables();

    int read(const void* ptr, const SKsize& len, int headerFlags, int fileFlags);

    SKuint32      findTypeId(const ftCharHashKey& cp);
    ftCharHashKey getStructHashByType(const SKuint16& type);

    const ftName& getStructNameByIdx(const SKuint16& idx) const;
    SKhash        getTypeHash(const SKuint16& type) const;
    bool          isPointer(const SKuint16& name) const;

    const Structures& getStructureArray() const
    {
        return m_structures;
    }

    Structures::Iterator getStructIterator()
    {
        return m_structures.iterator();
    }

    // Access to the size of a pointer when it was saved in the table.
    // This is cheating a bit here, it depends on the correct flag
    // being set when loaded it's not actually being computed.
    SKuint8 getSizeofPointer() const
    {
        return m_ptrLength;
    }

    bool isValidType(const SKuint32& typeidx) const
    {
        return typeidx < m_strcCount && typeidx < m_structures.size();
    }

    ftStruct* findStructByName(const ftCharHashKey& kvp);
    ftStruct* findStructByType(const SKuint16& type);
    SKuint32  findStructIdByType(const SKuint16& type);

    SKuint32 getNumberOfNames() const
    {
        return m_nameCount;
    }

    const ftName& getNameAt(SKuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx];
        return INVALID_NAME;
    }

    const char* getStringNameAt(SKuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx].m_name;
        return "";
    }

    SKuint32 getNumberOfTypes() const
    {
        return m_typeCount;
    }

    const ftType& getTypeAt(SKuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_types[idx];
        return INVALID_TYPE;
    }

    char* getTypeNameAt(SKuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_types[idx].m_name;
        return nullptr;
    }

    SKuint32 getNumberOfTypeLengths() const
    {
        return m_typeCount;
    }

    const SKtype& getTypeLengthAt(SKuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_tlens[idx];
        return SK_NPOS16;
    }

    SKuint32 getNumberOfStructs() const
    {
        return m_strcCount;
    }

    SKtype* getStructAt(SKuint32 idx) const
    {
        if (idx < m_strcCount)
            return m_strcs[idx];
        return 0;
    }

    SKuint16 getFirstStructType() const
    {
        return m_firstStruct;
    }

    bool testDuplicateKeys();

private:
    friend class ftStruct;
    friend class ftMember;

    Names m_names;
    Types m_types;
    TypeL m_tlens;
    Strcs m_strcs;

    NameHash   m_hashedNames;
    SKuint16   m_nameCount;
    SKuint16   m_typeCount;
    SKuint16   m_strcCount;
    SKuint16   m_firstStruct;
    Structures m_structures;
    SKuint8    m_ptrLength;
    TypeFinder m_typeFinder;

    static int readTableHeader(
        ftMemoryStream& stream,
        const char*     headerName,
        int             fileFlags);

    int readNameTable(
        ftMemoryStream& stream,
        int             headerFlags,
        int             fileFlags);

    int readTypeTable(
        ftMemoryStream& stream,
        int             headerFlags,
        int             fileFlags);

    int readSizeTable(
        ftMemoryStream& stream,
        int             headerFlags,
        int             fileFlags);

    int readStructureTable(
        ftMemoryStream& stream,
        int             headerFlags,
        int             fileFlags);

    int allocateTable(
        void** dest,
        SKsize numberOfEntries,
        SKsize sizeofEntry,
        int    fileFlags);

    void clearTables(void);

    void convertName(ftName& destName, char* convString) const;

    int buildStruct(SKuint16*& buf, SKuint16 current, int headerFlags, int fileFlags);

    int isValidTypeName(const SKuint16& type, const SKuint16& name, int fileFlags);

    void putMember(SKtype    owningStructureType,
                   ftName*   owningStructureName,
                   SKtype*   currentMemeber,
                   ftStruct* root,
                   SKtype    index,
                   SKuint32& currentOffset,
                   SKuint32  recursiveDepth,
                   int       fileFlags,
                   int&      status);

    void compile(SKtype    owningStructureType,
                 ftName*   owningStructureName,
                 SKtype    memberCount,
                 ftStruct* root,
                 SKuint32& currentOffset,
                 SKuint32  recursiveDepth,
                 int       fileFlags,
                 int&      status);

    int compile(int fileFlags);

    void hashMember(class skString& name,
                    SKhash          parentStructName,
                    SKhash          owningStructType,
                    SKhash          owningStructMemeberName,
                    SKhash          memberType,
                    SKhash          memberName);
};

#endif  //_ftTables_h_

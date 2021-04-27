/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

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
    const SKuint32 REND = FT_TYPEID('R', 'E', 'N', 'D');
}  // namespace ftIdNames

/// <summary>
/// Class for handling the structure table.
/// </summary>
class ftTable
{
public:
    /// <summary>
    ///
    /// </summary>
    typedef ftName* Names;  // < FT_MAX_TABLE

    /// <summary>
    ///
    /// </summary>
    typedef ftType* Types;  // < FT_MAX_TABLE

    /// <summary>
    ///
    /// </summary>
    typedef FTtype* TypeLengths;  // < FT_MAX_TABLE

    /// <summary>
    ///
    /// </summary>
    typedef FTtype** StructurePointers;  // < FT_MAX_TABLE * FT_MAX_MEMBERS;

    /// <summary>
    ///
    /// </summary>
    typedef skArray<SKsize> NameHash;

    /// <summary>
    ///
    /// </summary>
    typedef skArray<ftStruct*> StructureArray;

    /// <summary>
    ///
    /// </summary>
    typedef skHashTable<ftCharHashKey, ftType> TypeFinder;

    static const ftName InvalidName;
    static const ftType InvalidType;

public:
    ftTable(SKuint8 pointerLength);
    ~ftTable();

    int read(const void*   tableSource,
             const SKsize& tableLength,
             int           headerFlags,
             int           fileFlags);

    /// <summary>
    ///
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    SKuint32 findTypeId(const ftCharHashKey& type);

    ftCharHashKey getStructHashByType(const SKuint16& type) const;

    const ftName& getStructNameByIdx(const SKuint16& idx) const;
    SKhash        getTypeHash(const SKuint16& type) const;
    bool          isPointer(const SKuint16& name) const;

    const StructureArray& getStructureArray() const
    {
        return m_structures;
    }

    StructureArray::Iterator getStructIterator()
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

    bool isValidType(const SKuint32& typeIdx) const
    {
        return typeIdx < m_strcCount && typeIdx < m_structures.size();
    }

    ftStruct* findStructByName(const ftCharHashKey& kvp);
    ftStruct* findStructByType(const SKint32& type);
    SKuint32  findStructIdByType(const SKuint16& type) const;

    SKuint32 getNumberOfNames() const
    {
        return m_nameCount;
    }

    const ftName& getNameAt(SKuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx];
        return InvalidName;
    }

    const char* getStringNameAt(SKuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx].name;
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
        return InvalidType;
    }

    char* getTypeNameAt(SKuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_types[idx].name;
        return nullptr;
    }

    SKuint32 getNumberOfTypeLengths() const
    {
        return m_typeCount;
    }

    const FTtype& getTypeLengthAt(SKuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_tlens[idx];
        return SK_NPOS16;
    }

    SKuint32 getNumberOfStructs() const
    {
        return m_strcCount;
    }

    FTtype* getStructAt(SKuint32 idx) const
    {
        if (idx < m_strcCount)
            return m_strcs[idx];
        return 0;
    }

    SKuint16 getFirstStructType() const
    {
        return m_firstStruct;
    }

    bool testDuplicateKeys() const;

    const Types& getTypes() const
    {
        return m_types;
    }

private:
    friend class ftStruct;
    friend class ftMember;

    Names             m_names;
    Types             m_types;
    TypeLengths       m_tlens;
    StructurePointers m_strcs;

    NameHash       m_hashedNames;
    SKuint16       m_nameCount;
    SKuint16       m_typeCount;
    SKuint16       m_strcCount;
    SKuint16       m_firstStruct;
    StructureArray m_structures;
    SKuint8        m_ptrLength;
    TypeFinder     m_typeFinder;

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

    static int allocateTable(
        void** destination,
        SKsize numberOfEntries,
        SKsize sizeOfEntry,
        int    fileFlags);

    void clearTables(void);

    int convertName(ftName& dest, char* cp) const;

    int buildStruct(SKuint16*& structure, SKuint16 current, int headerFlags, int fileFlags);

    int isValidTypeName(const SKuint16& type, const SKuint16& name, int fileFlags) const;

    void putMember(FTtype        owningStructureType,
                   ftName*       owningStructureName,
                   const FTtype* currentMember,
                   ftStruct*     root,
                   FTtype        index,
                   SKuint32&     currentOffset,
                   SKuint32      recursiveDepth,
                   int           fileFlags,
                   int&          status) const;

    void compile(FTtype    owningStructureType,
                 ftName*   owningStructureName,
                 FTtype    memberCount,
                 ftStruct* root,
                 SKuint32& currentOffset,
                 SKuint32  recursiveDepth,
                 int       fileFlags,
                 int&      status) const;

    int compile(int fileFlags);

    void hashMember(class skString& name,
                    SKhash          parentStructName,
                    SKhash          owningStructType,
                    SKhash          owningStructMemberName,
                    SKhash          memberType,
                    SKhash          memberName) const;
};

#endif  //_ftTables_h_

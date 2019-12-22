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

    const FBTuint32 SDNA = ftID('S', 'D', 'N', 'A');
    const FBTuint32 DNA1 = ftID('D', 'N', 'A', '1');
    const FBTuint32 ENDB = ftID('E', 'N', 'D', 'B');
    const FBTuint32 DATA = ftID('D', 'A', 'T', 'A');
}  // namespace ftIdNames


class ftTables
{
public:
    typedef ftName*                            Names;  // < FT_MAX_TABLE
    typedef ftType*                            Types;  // < FT_MAX_TABLE
    typedef FBTtype*                           TypeL;  // < FT_MAX_TABLE
    typedef FBTtype**                          Strcs;  // < FT_MAX_TABLE * FT_MAX_MEMBERS;
    typedef skArray<SKsize>                    NameHash;
    typedef skArray<ftStruct*>                 Structures;
    typedef skHashTable<ftCharHashKey, ftType> TypeFinder;

    static const ftName INVALID_NAME;
    static const ftType INVALID_TYPE;

public:
    ftTables(int pointerLength);
    ~ftTables();

    int read(const void* ptr, const FBTsize& len, int headerFlags, int fileFlags);

    FBTuint32     findTypeId(const ftCharHashKey& cp);
    ftCharHashKey getStructHashByType(const FBTuint16& type);

    const ftName& getStructNameByIdx(const FBTuint16& idx) const;
    FBThash       getTypeHash(const FBTuint16& type) const;
    bool          isPointer(const FBTuint16& name) const;



    inline Structures::Iterator getStructIterator()
    {
        return m_structures.iterator();
    }


    // Access to the size of a pointer when it was saved in the table.
    // This is cheating a bit here, it depends on the correct flag
    // being set when loaded it's not actually being computed.
    inline FBTuint8 getSizeofPointer()
    {
        return m_ptrLength;
    }

    inline bool isValidType(const FBTuint32& typeidx) const
    {
        return typeidx < m_strcCount && typeidx < m_structures.size();
    }

    ftStruct* findStructByName(const ftCharHashKey& kvp);
    ftStruct* findStructByType(const FBTuint16& type);
    FBTuint32 findStructIdByType(const FBTuint16& type);

    bool isLinkedToMemory(const FBTuint16& type);

    inline FBTuint32 getNumberOfNames() const
    {
        return m_nameCount;
    }

    inline const ftName& getNameAt(FBTuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx];
        return INVALID_NAME;
    }

    inline const char* getStringNameAt(FBTuint32 idx) const
    {
        if (idx < m_nameCount)
            return m_names[idx].m_name;
        return "";
    }


    inline FBTuint32 getNumberOfTypes() const
    {
        return m_typeCount;
    }

    inline const ftType& getTypeAt(FBTuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_types[idx];
        return INVALID_TYPE;
    }

    inline char* getTypeNameAt(FBTuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_types[idx].m_name;
        return nullptr;
    }

    inline FBTuint32 getNumberOfTypeLengths() const
    {
        return m_typeCount;
    }

    inline const FBTtype& getTypeLengthAt(FBTuint32 idx) const
    {
        if (idx < m_typeCount)
            return m_tlens[idx];
        return SK_NPOS16;
    }

    inline FBTuint32 getNumberOfStructs() const
    {
        return m_strcCount;
    }

    inline FBTtype* getStructAt(FBTuint32 idx) const
    {
        if (idx < m_strcCount)
            return m_strcs[idx];
        return 0;
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
    FBTuint16  m_nameCount;
    FBTuint16  m_typeCount;
    FBTuint16  m_strcCount;
    FBTuint16  m_firstStruct;
    Structures m_structures;
    FBTuint8   m_ptrLength;
    TypeFinder m_typeFinder;



    int allocateTable(void**  dest,
                      FBTsize numberOfEntries,
                      FBTsize sizeofEntry,
                      int     fileFlags); 


    void convertName(ftName& destName, char*& convString);
    int  buildStruct(FBTuint16*& buf, FBTuint16 current, int headerFlags, int fileFlags);

    int isValidTypeName(const FBTuint16& type, const FBTuint16& name, int fileFlags);


    void putMember(FBTtype*   cp,
                   ftStruct*  off,
                   FBTtype    nr,
                   FBTuint32& cof,
                   FBTuint32  depth,
                   int&       status);

    void compile(FBTtype    i,
                 FBTtype    nr,
                 ftStruct*  off,
                 FBTuint32& cof,
                 FBTuint32  depth,
                 int&       status);
    int compile(int fileFlags);
};


#endif  //_ftTables_h_

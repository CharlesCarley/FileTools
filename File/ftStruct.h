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
#ifndef _ftStruct_h_
#define _ftStruct_h_

#include "Utils/skBinarySearchTree.h"
#include "ftAtomic.h"
#include "ftTypes.h"

class ftTables;
class ftStruct;
class ftMember;

struct ftName
{
    char*  m_name;
    SKhash m_hash;
    int    m_ptrCount;
    int    m_numDimensions;
    int    m_isFunctionPointer;
    int    m_arraySize;
    int    m_dimensions[FT_ARR_DIM_MAX];
};

struct ftType
{
    char*  m_name;  // note: memory is in the main table.
    SKhash m_hash;  // ftCharHashKey(typeName)

    // This must be checked against ftTables::getFirstStructType
    //
    //  [0-NumberOfBuiltin] = SK_NPOS32
    //  (NumberOfBuiltin, NumberOfStructs]
    SKuint32 m_strcId;
};

struct MemberSearchKey
{
    SKhash    m_hash;
    ftMember* m_member;
};

inline bool operator<(const MemberSearchKey& a, const MemberSearchKey& b)
{
    return a.m_hash < b.m_hash;
}

inline bool operator==(const MemberSearchKey& a, const MemberSearchKey& b)
{
    return a.m_hash == b.m_hash;
}

class ftStruct
{
public:
    typedef skArray<ftMember*>                  Members;
    typedef skBinarySearchTree<MemberSearchKey> MemberLookup;

    enum Flag
    {
        CAN_LINK      = 0,
        MISSING       = 1 << 0,
        MISALIGNED    = 1 << 1,
        NEED_CAST     = 1 << 2,
        HAS_DEPENDENT = 1 << 3,
    };

public:
    ftStruct(ftTables* parent);
    ~ftStruct();

    // String type name
    const char* getName() const;

    ftMember* getMember(Members::SizeType idx);
    ftMember* find(ftMember* oth) const;

    Members::SizeType getMemberCount() const
    {
        return m_members.size();
    }

    Members::Iterator getMemberIterator()
    {
        return m_members.iterator();
    }

    // Returns the base address as byte pointer of the nth block of base.
    SKbyte* getBlock(void* base, SKsize idx, SKsize max) const;

    ftTables* getParent() const
    {
        return m_table;
    }

    SKint16 getTypeIndex() const
    {
        return m_type;
    }

    const SKhash& getHashedType() const
    {
        return m_hashedType;
    }

    SKint32 getStructIndex() const
    {
        return m_structureId;
    }

    const SKint32& getSizeInBytes() const
    {
        return m_sizeInBytes;
    }

    void setLink(ftStruct* structure)
    {
        m_link = structure;
    }

    ftStruct* getLink()
    {
        return m_link;
    }

    const ftStruct* getLink() const
    {
        return m_link;
    }

    bool hasLink() const
    {
        return m_link != nullptr;
    }

    // If this bit is set, it means that the struct contains
    // references to other structures that are not pointers.
    bool hasDependentTypes() const
    {
        return (m_flag & HAS_DEPENDENT) != 0;
    }

    SKint32 getFlag() const
    {
        return m_flag;
    }

    void setFlag(const SKint32& bits)
    {
        m_flag = bits;
    }

    void addFlag(const SKint32& bit)
    {
        m_flag |= bit;
    }

    bool hasFlag(const SKint32& bit) const
    {
        return (m_flag & bit) != 0;
    }

    // Returns the number of dependent structures
    SKint32 getReferences() const
    {
        return m_refs;
    }

    void addRef()
    {
        m_refs++;
    }

    // This is used when sorting the parent table's
    // structures before decompiling them into code.

    void lock()
    {
        m_lock = 1;
    }

    bool isLocked() const
    {
        return m_lock != 0;
    }

private:
    friend class ftTables;
    friend class ftMember;

    ftMember* createMember();

    SKuint16     m_type;
    SKhash       m_hashedType;
    void*        m_attached;
    SKint32      m_sizeInBytes;
    SKint32      m_refs, m_lock;
    SKint32      m_structureId;
    SKint32      m_flag;
    Members      m_members;
    ftTables*    m_table;
    ftStruct*    m_link;
    MemberLookup m_memberSearch;
};

#endif  //_ftStruct_h_

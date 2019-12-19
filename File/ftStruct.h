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

#include "ftAtomic.h"
#include "ftTypes.h"

class ftBinTables;
class ftStruct;


struct ftName
{
    char*   m_name;
    FBThash m_hashedName;
    int     m_ptrCount;
    int     m_numDimensions;
    int     m_isFunctionPointer;
    int     m_arraySize;
    int     m_dimensions[FT_ARR_DIM_MAX];
};


struct ftType
{
    char*     m_name;    // note: memory is in the main table.
    FBThash   m_typeId;  // ftCharHashKey(typeName)
    FBTuint32 m_strcId;  // [0-NumberOfBuiltin] = SK_NPOS32
                         // (NumberOfBuiltin, NumberOfStructs]
};



union ftKey32 {
    FBTint16 k16[2];
    FBTint32 k32;
};


struct ftKey64
{
    FBThash m_type;
    FBThash m_name;
};



class ftMember
{
public:
    ftMember(ftStruct* owner);
    ~ftMember();


    void setNameIndex(const FBTuint16& idx);
    void setTypeIndex(const FBTuint16& idx);

    bool     isBuiltinType();
    bool     isStructure();
    bool     isPointer();
    bool     isArray();
    int      getArraySize();
    int      getPointerCount();
    int      getArrayElementSize();
    ftAtomic getAtomicType();

    bool     compare(ftMember* rhs);
    FBTsize* jumpToOffset(void* base);


    void* getChunk();

    inline FBTsize getSizeInBytes()
    {
        return m_sizeInBytes;
    }

    inline const FBThash& getTypeName() const
    {
        return m_hashedType;
    }

    inline const FBThash& getHashedName() const
    {
        return m_hashedName;
    }

    inline ftStruct* getOwner()
    {
        return m_owner;
    }


    inline ftMember* getLink()
    {
        return m_link;
    }

    inline bool hasLink()
    {
        return m_link != 0;
    }


    inline void setLink(ftMember* member)
    {
        m_link = member;
    }


private:
    friend class ftBinTables;
    friend class ftStruct;

    ftStruct* m_owner;
    ftMember* m_link;

    FBTint32 m_location;
    FBTint32 m_offset;
    FBTint32 m_recursiveDepth;
    FBTint32 m_sizeInBytes;

    FBTint16 m_type;
    FBTint16 m_name;
    FBThash  m_hashedType;
    FBThash  m_hashedName;
};



class ftStruct
{
public:
    typedef skArray<ftMember*> Members;

    enum Flag
    {
        CAN_LINK   = 0,
        MISSING    = (1 << 0),
        MISALIGNED = (1 << 1),
        SKIP       = (1 << 2),
        NEED_CAST  = (1 << 3)
    };

public:
    ftStruct(ftBinTables* parent);
    ~ftStruct();

    ftMember* createMember();
    ftMember* getMember(Members::SizeType idx);
    FBTbyte*  getBlock(void* base, SKsize idx, const SKsize max);

    void attachChunk(void* ptr)
    {
        m_attached = ptr;
    }
    void* getAttached()
    {
        return m_attached;
    }

    inline ftBinTables* getParent()
    {
        return m_table;
    }

    inline const FBTint16& getTypeIndex() const
    {
        return m_key.k16[0];
    }

    inline const FBTint16& getNameIndex() const
    {
        return m_key.k16[1];
    }

    inline const FBThash& getHashedType() const
    {
        return m_val.m_type;
    }

    inline const FBThash& getHashedName() const
    {
        return m_val.m_name;
    }

    inline FBTint32 getStructIndex() const
    {
        return m_strcId;
    }


    inline const FBTint32& getSizeInBytes() const
    {
        return m_len;
    }


    inline Members::Iterator getMemberIterator()
    {
        return m_members.iterator();
    }


    inline Members::SizeType getMemberCount() const
    {
        return m_members.size();
    }


    inline FBTsize getOffset()
    {
        return m_off;
    }



    inline void setLink(ftStruct* strc)
    {
        m_link = strc;
    }


    inline ftStruct* getLink()
    {
        return m_link;
    }

    inline const ftStruct* getLink() const
    {
        return m_link;
    }

    inline bool hasLink() const
    {
        return m_link != nullptr;
    }

    inline FBTint32 getFlag() const
    {
        return m_flag;
    }

    inline void setFlag(const FBTint32& bits)
    {
        m_flag = bits;
    }

    inline void addFlag(const FBTint32& bit)
    {
        m_flag |= bit;
    }

    inline bool hasFlag(const FBTint32& bit) const
    {
        return (m_flag & bit) != 0;
    }


    inline FBTint32 getBufferOffset()
    {
        return m_off;
    }



private:
    inline void setNameIndex(const FBTuint16& idx)
    {
        m_key.k16[0] = idx;
    }

    inline void setTypeIndex(const FBTuint16& idx)
    {
        m_key.k16[1] = idx;
    }



    inline void setHashedType(const FBThash& hash)
    {
        m_val.m_type = hash;
    }

    inline void setHashedName(const FBThash& hash)
    {
        m_val.m_name = hash;
    }

    friend class ftBinTables;
    friend class ftMember;

    ftKey32  m_key;
    ftKey64  m_val;
    FBTint32 m_off;
    void*    m_attached;

    FBTint32 m_len;
    FBTint32 m_nr, m_dp;
    FBTint32 m_strcId;
    FBTint32 m_flag;
    Members  m_members;

    ftBinTables* m_table;
    ftStruct*    m_link;
};



#endif  //_ftStruct_h_

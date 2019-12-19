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
class ftMember;


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


    // String type name
    const char* getName() const;

    ftMember* getMember(Members::SizeType idx);

    inline Members::SizeType getMemberCount() const
    {
        return m_members.size();
    }

    inline Members::Iterator getMemberIterator()
    {
        return m_members.iterator();
    }


    // Returns the base address as byte pointer of the nth block of base.
    FBTbyte* getBlock(void* base, SKsize n, const SKsize max);



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

    inline const FBTint16 getTypeIndex() const
    {
        return m_type;
    }

    inline const FBThash& getHashedType() const
    {
        return m_hashedType;
    }


    inline FBTint32 getStructIndex() const
    {
        return m_strcId;
    }


    inline const FBTint32& getSizeInBytes() const
    {
        return m_sizeInBytes;
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


private:
    ftMember* createMember();


    friend class ftBinTables;
    friend class ftMember;

    FBTuint16 m_type;
    FBThash   m_hashedType;
    void*     m_attached;

    FBTint32 m_sizeInBytes;
    FBTint32 m_strcId;
    FBTint32 m_flag;
    Members  m_members;


    ftBinTables* m_table;
    ftStruct*    m_link;
};



#endif  //_ftStruct_h_

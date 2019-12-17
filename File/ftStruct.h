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

#include "ftTypes.h"


struct ftName
{
    char*   m_name;
    int     m_loc;
    FBThash m_nameId;
    int     m_ptrCount;
    int     m_numSlots, m_isFptr;
    int     m_arraySize;
    int     m_slots[FT_ARR_DIM_MAX];
};

struct ftType
{
    char*     m_name;    // note: memory is in the main table.
    FBThash   m_typeId;  // ftCharHashKey(typeName)
    FBTuint32 m_strcId;
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
    ftStruct();
    ~ftStruct();


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

    ftStruct* getMember(Members::SizeType idx);

    inline Members::SizeType getMemberCount() const
    {
        return m_members.size();
    }



    bool     isDifferent(ftStruct* rhs);

    FBTbyte* getBlock(void* base, SKsize idx, const SKsize max);
    FBTsize* jumpToOffset(void* base);


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

    /// temp
    friend class ftBinTables;

    ftKey32  m_key;
    ftKey64  m_val;
    FBTint32 m_off;

    FBTint32  m_len;
    FBTint32  m_nr, m_dp;
    FBTint32  m_strcId;
    FBTint32  m_flag;
    Members   m_members;
    ftStruct* m_link;
    Keys      m_keyChain;
};



#endif  //_ftStruct_h_

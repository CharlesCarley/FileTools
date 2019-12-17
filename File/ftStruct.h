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

    inline const FBTint32& getSizeInBytes() const
    {
        return m_len;
    }

    ftStruct* getMember(Members::SizeType idx);


public:

    ftKey32   m_key;  // k[0]: type, k[1]: name
    ftKey64   m_val;  // key hash value, k[0]: type hash id, k[1]: member(field) base name hash id or 0(struct)
    FBTint32  m_off;  // offset
    FBTint32  m_len;
    FBTint32  m_nr, m_dp;  //nr: array index, dp: embedded depth
    FBTint32  m_strcId;
    FBTint32  m_flag;
    Members   m_members;
    ftStruct* m_link;      //file/memory table struct link
    Keys      m_keyChain;  //parent key hash chain(0: type hash, 1: name hash), size() == m_dp
};



#endif  //_ftStruct_h_

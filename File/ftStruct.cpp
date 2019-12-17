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
#include "ftStruct.h"
#include <stdio.h>



ftStruct::ftStruct() :
    m_key(),
    m_val(),
    m_off(0),
    m_len(0),
    m_nr(0),
    m_dp(0),
    m_strcId(0),
    m_flag(0),
    m_members(),
    m_link(0)
{
    m_key.k32 = 0;
}


ftStruct::~ftStruct()
{
}


ftStruct* ftStruct::getMember(Members::SizeType idx)
{
    if (idx < m_members.size())
        return &m_members.ptr()[idx];
    return nullptr;
}



bool ftStruct::isDifferent(ftStruct* rhs)
{
    if (!rhs)
        return false;

    bool result = m_nr == rhs->m_nr;
    if (result)
    {
        result = m_dp == rhs->m_dp;
        if (result)
        {
            result = m_val.m_name == rhs->m_val.m_name;
            if (result)
                result = m_keyChain.equals(rhs->m_keyChain);
        }
    }
    return result;
}


FBTbyte* ftStruct::getBlock(void* base, SKsize idx, const SKsize max)
{
    FBTbyte* val = 0;
    if (base && idx < max)
    {
        val = ((FBTbyte*)base);
        val += m_len * idx;
    }
    return val;
}



FBTsize* ftStruct::jumpToOffset(void* base)
{
    FBTbyte* val = (FBTbyte*)base;

    if (m_off < m_len)
        val += m_off;
    else
    {
        printf("offset exceeds length\n");
    }
    return (FBTsize*)val;
}

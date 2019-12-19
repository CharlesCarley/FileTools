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
#include "ftAtomic.h"
#include "ftTables.h"
#include "ftMember.h"




ftStruct::ftStruct(ftBinTables* parent) :
    m_table(parent),
    m_type(0),
    m_hashedType(SK_NPOS),
    m_sizeInBytes(0),
    m_strcId(0),
    m_flag(0),
    m_attached(0),
    m_members(),
    m_link(0)
{
}


ftStruct::~ftStruct()
{
    Members::Iterator it = m_members.iterator();
    while (it.hasMoreElements())
        delete it.getNext();
}


ftMember* ftStruct::createMember()
{
    ftMember* mbr = new ftMember(this);
    m_members.push_back(mbr);
    return mbr;
}



ftMember* ftStruct::getMember(Members::SizeType idx)
{
    if (idx < m_members.size())
        return m_members[idx];
    return nullptr;
}


FBTbyte* ftStruct::getBlock(void* base, SKsize idx, const SKsize max)
{
    FBTbyte* val = 0;
    if (base && idx < max)
    {
        val = ((FBTbyte*)base);
        val += m_sizeInBytes * idx;
    }
    return val;
}


const char* ftStruct::getName() const
{
    if (m_table && m_type < m_table->m_typeNr)
        return m_table->m_type[m_type].m_name;
    return "";
}

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
#include "ftMember.h"
#include "ftAtomic.h"
#include "ftStruct.h"
#include "ftTables.h"



ftMember::ftMember(ftStruct* owner) :
    m_owner(owner),
    m_offset(0),
    m_type(0),
    m_name(0),
    m_hashedType(SK_NPOS),
    m_hashedName(SK_NPOS),
    m_link(0),
    m_location(0),
    m_recursiveDepth(0),
    m_sizeInBytes(0)
{
}

ftMember::~ftMember()
{
}


const char* ftMember::getName()
{
    if (m_owner && m_owner->m_table)
    {
        if (m_name < m_owner->m_table->m_nameNr)
            return m_owner->m_table->m_name[m_name].m_name;
    }
    return "";
}


const char* ftMember::getType()
{
    if (m_owner && m_owner->m_table)
    {
        if (m_type < m_owner->m_table->m_typeNr)
            return m_owner->m_table->m_type[m_type].m_name;
    }
    return "";
}


bool ftMember::isBuiltinType()
{
    if (m_owner && m_owner->m_table)
        return m_type < m_owner->m_table->m_firstStruct;
    return false;
}

bool ftMember::isStructure()
{
    if (m_owner && m_owner->m_table)
        return m_type >= m_owner->m_table->m_firstStruct;
    return false;
}

bool ftMember::isPointer()
{
    return getPointerCount() > 0;
}

bool ftMember::isArray()
{
    if (m_owner && m_owner->m_table)
    {
        if (m_name < m_owner->m_table->m_nameNr)
            return m_owner->m_table->m_name[m_name].m_arraySize > 1;
    }
    return false;
}

int ftMember::getArraySize()
{
    if (m_owner && m_owner->m_table)
    {
        if (m_name < m_owner->m_table->m_nameNr)
            return m_owner->m_table->m_name[m_name].m_arraySize;
    }
    return 0;
}


int ftMember::getArrayElementSize()
{
    int arraySize = skMax(getArraySize(), 1);
    return m_sizeInBytes / arraySize;
}


ftAtomic ftMember::getAtomicType()
{
    return ftAtomicUtils::getPrimitiveType(m_hashedType);
}


int ftMember::getPointerCount()
{
    if (m_owner && m_owner->m_table)
    {
        if (m_name < m_owner->m_table->m_nameNr)
            return m_owner->m_table->m_name[m_name].m_ptrCount;
    }
    return 0;
}



void ftMember::setNameIndex(const FBTuint16& idx)
{
    m_name = idx;
    if (m_owner && m_owner->m_table)
    {
        if (m_name < (FBTuint16)m_owner->m_table->m_base.size())
            m_hashedName = m_owner->m_table->m_base.at(m_name);
    }
}

void ftMember::setTypeIndex(const FBTuint16& idx)
{
    m_type = idx;

    if (m_owner && m_owner->m_table)
    {
        if (m_name < (FBTint16)m_owner->m_table->m_base.size())
            m_hashedType = m_owner->m_table->m_type[m_type].m_typeId;
    }
}

bool ftMember::compare(ftMember* rhs)
{
    if (!rhs)
        return false;

    bool result;
    result = m_recursiveDepth <= rhs->m_recursiveDepth;
    if (result)
    {
        result = m_hashedName == rhs->m_hashedName;
        if (result)
            result = m_hashedType == rhs->m_hashedType;
    }
    return result;
}

void* ftMember::getChunk()
{
    return m_owner ? m_owner->m_attached : 0;
}

FBTsize* ftMember::jumpToOffset(void* base)
{
    if (m_offset < m_owner->m_sizeInBytes)
        return reinterpret_cast<FBTsize*>(reinterpret_cast<FBTbyte*>(base) + m_offset);
    return nullptr;
}

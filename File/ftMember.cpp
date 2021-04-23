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
    m_parent(owner),
    m_offset(0),
    m_type(0),
    m_name(0),
    m_hashedType(SK_NPOS),
    m_hashedName(SK_NPOS),
    m_link(0),
    m_location(0),
    m_recursiveDepth(0),
    m_sizeInBytes(0),
    m_atomic(-1),
    m_searchKey(SK_NPOS)
{
}

ftMember::~ftMember() = default;

const char* ftMember::getName() const
{
    if (m_parent && m_parent->m_table)
    {
        if (m_name < m_parent->m_table->m_nameCount)
            return m_parent->m_table->m_names[m_name].m_name;
    }
    return "";
}

const char* ftMember::getType() const
{
    if (m_parent && m_parent->m_table)
    {
        if (m_type < m_parent->m_table->m_typeCount)
            return m_parent->m_table->m_types[m_type].m_name;
    }
    return "";
}

bool ftMember::isBuiltinType() const
{
    if (m_parent && m_parent->m_table)
        return m_type < m_parent->m_table->m_firstStruct;
    return false;
}

bool ftMember::isStructure() const
{
    if (m_parent && m_parent->m_table)
        return m_type >= m_parent->m_table->m_firstStruct;
    return false;
}

bool ftMember::isPointer() const
{
    return getPointerCount() > 0;
}

bool ftMember::isArray() const
{
    if (m_parent && m_parent->m_table)
    {
        if (m_name < m_parent->m_table->m_nameCount)
            return m_parent->m_table->m_names[m_name].m_arraySize > 1;
    }
    return false;
}

bool ftMember::isCharacter()
{
    return getAtomicType() == ftAtomic::FT_ATOMIC_CHAR;
}

bool ftMember::isInteger16()
{
    const ftAtomic atomic = getAtomicType();
    return atomic == ftAtomic::FT_ATOMIC_SHORT || atomic == ftAtomic::FT_ATOMIC_USHORT;
}

bool ftMember::isInteger32()
{
    // TODO: ftSCALAR_DOUBLE
    const ftAtomic atomic = getAtomicType();
    return atomic >= ftAtomic::FT_ATOMIC_INT && atomic <= ftAtomic::FT_ATOMIC_FLOAT;
}

bool ftMember::isInteger64()
{
    // TODO: ftSCALAR_DOUBLE
    const ftAtomic atomic = getAtomicType();
    return atomic >= ftAtomic::FT_ATOMIC_DOUBLE && atomic <= ftAtomic::FT_ATOMIC_UINT64_T;
}

int ftMember::getArraySize() const
{
    if (m_parent && m_parent->m_table)
    {
        if (m_name < m_parent->m_table->m_nameCount)
            return m_parent->m_table->m_names[m_name].m_arraySize;
    }
    return 1;
}

int ftMember::getArrayElementSize() const
{
    const int arraySize = skMax(getArraySize(), 1);
    return m_sizeInBytes / arraySize;
}

bool ftMember::isValidAtomicType()
{
    return isBuiltinType();
}

ftAtomic ftMember::getAtomicType()
{
    if (m_atomic == -1)
        m_atomic = (SKint32)ftAtomicUtils::getPrimitiveType(m_hashedType);
    return (ftAtomic)m_atomic;
}

int ftMember::getPointerCount() const
{
    if (m_parent && m_parent->m_table)
    {
        if (m_name < m_parent->m_table->m_nameCount)
            return m_parent->m_table->m_names[m_name].m_ptrCount;
    }
    return 0;
}

void ftMember::setNameIndex(const SKuint16& idx)
{
    m_name = idx;
    if (m_parent && m_parent->m_table)
    {
        if (m_name < (SKuint16)m_parent->m_table->m_hashedNames.size())
            m_hashedName = m_parent->m_table->m_hashedNames[m_name];
    }
}

void ftMember::setTypeIndex(const SKuint16& idx)
{
    m_type = idx;

    if (m_parent && m_parent->m_table)
    {
        if (m_type < (SKint16)m_parent->m_table->m_typeCount)
            m_hashedType = m_parent->m_table->m_types[m_type].m_hash;
    }
}

bool ftMember::compare(ftMember* rhs) const
{
    if (!rhs)
        return false;
    return m_searchKey == rhs->m_searchKey;
}

void* ftMember::getChunk() const
{
    return m_parent ? m_parent->m_attached : nullptr;
}

SKsize* ftMember::jumpToOffset(void* base) const
{
    if (base && m_offset < m_parent->m_sizeInBytes)
        return (SKsize*)(static_cast<SKbyte*>(base) + m_offset);
    return nullptr;
}

/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

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

#ifndef _ftMember_h_
#define _ftMember_h_

#include "ftAtomic.h"
#include "ftTypes.h"

class ftMember
{
public:
    ftMember(ftStruct* owner);
    ~ftMember();

    const char* getName() const;
    const char* getType() const;

    void setNameIndex(const SKuint16& idx);
    void setTypeIndex(const SKuint16& idx);
    bool isBuiltinType() const;
    bool isStructure() const;
    bool isPointer() const;
    bool isArray() const;
    bool isCharacter();

    bool isInteger16();
    bool isInteger32();
    bool isInteger64();

    int getArraySize() const;
    int getPointerCount() const;
    int getArrayElementSize() const;

    ftAtomic getAtomicType();

    bool compare(ftMember* rhs) const;

    SKsize* jumpToOffset(void* base) const;
    void*   getChunk() const;

    bool isValidAtomicType() const;

    bool isCharacterArray()
    {
        return isCharacter() && isArray();
    }

    SKsize getSizeInBytes() const
    {
        return m_sizeInBytes;
    }

    SKsize getOffset() const
    {
        return m_offset;
    }

    const SKhash& getHashedType() const
    {
        return m_hashedType;
    }

    const SKhash& getHashedName() const
    {
        return m_hashedName;
    }

    ftStruct* getParent() const
    {
        return m_parent;
    }

    ftMember* getLink() const
    {
        return m_link;
    }

    bool hasLink() const
    {
        return m_link != nullptr;
    }

    void setLink(ftMember* member)
    {
        m_link = member;
    }

private:
    friend class ftTable;
    friend class ftStruct;

    ftStruct* m_parent;
    ftMember* m_link;

    // TODO: Look at this these should all be unsigned.
    // Find all references, and look for specific reasons
    // as to why they are declared signed.
    SKint32 m_location;
    SKint32 m_offset;
    SKint32 m_recursiveDepth;
    SKint32 m_sizeInBytes;

    // Keep this signed.
    SKint32 m_atomic;

    SKint16 m_type;
    SKint16 m_name;
    SKhash  m_hashedType;
    SKhash  m_hashedName;
    SKhash  m_searchKey;
};

#endif  //_ftMember_h_

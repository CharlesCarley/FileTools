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
#ifndef _ftMember_h_
#define _ftMember_h_

#include "ftAtomic.h"
#include "ftTypes.h"

class ftMember
{
public:
    ftMember(ftStruct* owner);
    ~ftMember();

    const char* getName();
    const char* getType();

    void setNameIndex(const SKuint16& idx);
    void setTypeIndex(const SKuint16& idx);
    bool isBuiltinType();
    bool isStructure();
    bool isPointer();
    bool isArray();
    bool isCharacter();

    bool isInteger16();
    bool isInteger32();
    bool isInteger64();

    int getArraySize();
    int getPointerCount();
    int getArrayElementSize();

    ftAtomic getAtomicType();

    bool compare(ftMember* rhs);


    SKsize* jumpToOffset(void* base);
    void*    getChunk();


    bool isValidAtomicType();

    bool isCharacterArray()
    {
        return isCharacter() && isArray();
    }

    inline SKsize getSizeInBytes()
    {
        return m_sizeInBytes;
    }

    inline SKsize getOffset()
    {
        return m_offset;
    }

    inline const SKhash& getHashedType() const
    {
        return m_hashedType;
    }

    inline const SKhash& getHashedName() const
    {
        return m_hashedName;
    }

    inline ftStruct* getParent()
    {
        return m_parent;
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
    friend class ftTables;
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

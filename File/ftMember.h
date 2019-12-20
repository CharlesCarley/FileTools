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

class ftBinTables;
class ftStruct;


class ftMember
{
public:
    ftMember(ftStruct* owner);
    ~ftMember();


    const char* getName();
    const char* getType();

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
    void*    getChunk();


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

    inline ftStruct* getParent()
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


#endif  //_ftMember_h_

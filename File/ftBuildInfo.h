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
#ifndef _ftBuildInfo_h_
#define _ftBuildInfo_h_

#include "ftBuildMember.h"
#include "ftBuildStruct.h"

typedef skArray<FBTtype> IntPtrArray;
typedef skArray<FBTtype> TypeArray;


struct MaxAllocSize
{
    FBTuint32 m_name;
    FBTuint32 m_type;
    FBTuint32 m_tlen;
    FBTuint32 m_strc;
    FBTuint32 m_structures;
};

class ftBuildInfo
{
public:
    ftBuildInfo();
    ~ftBuildInfo();

    void    reserve(void);
    int     getLengths(ftBuildStruct::Array& struct_builders);
    int     getTLengths(ftBuildStruct::Array& struct_builders);
    void    makeBuiltinTypes(void);
    bool    hasType(const ftId& type);
    FBTsize addType(const ftId& type, const FBTuint32& len);
    FBTsize addName(const ftId& name);

    MaxAllocSize     m_alloc;
    ftStringPtrArray m_name;
    ftStringPtrArray m_typeLookup;
    IntPtrArray      m_tlen;
    IntPtrArray      m_64ln;
    TypeArray        m_strc;
    ftStringPtrArray m_undef;
    FBTuint32        m_numberOfBuiltIn;
};


#endif//_ftBuildInfo_h_

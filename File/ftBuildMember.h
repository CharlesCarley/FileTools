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
#ifndef _ftBuildMember_h_
#define _ftBuildMember_h_

#include "Utils/skArray.h"
#include "Utils/skFixedString.h"
#include "Utils/skMap.h"
#include "ftTables.h"
#include "ftTypes.h"


class ftBuildMember
{
public:
    ftBuildMember();

    ftId         m_type;
    ftId         m_name;
    int          m_typeId;
    FBTtype      m_hashedName;
    int          m_ptrCount;
    int          m_numDimensions;
    int          m_isFunctionPointer;
    int          m_lstat;
    int          m_undefined;
    bool         m_isDependentType;
    ftArraySlots m_arrays;
    FBTsize      m_arraySize;
    ftPath       m_path;
    FBTsize      m_line;
};

#endif  //_ftBuildMember_h_

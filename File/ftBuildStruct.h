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
#ifndef _ftBuildStruct_h_
#define _ftBuildStruct_h_

#include "ftTables.h"
#include "ftTypes.h"
#include "ftBuildMember.h"


class ftBuildStruct
{
public:
    typedef skArray<ftBuildStruct> Array;
    typedef skArray<ftBuildMember>      Variables;

public:
    ftBuildStruct();

    FBTsize     m_structId;
    ftId        m_name;
    Variables   m_data;
    FBTsize     m_nrDependentTypes;
    ftPath      m_path;
    FBTsize     m_line;
};

#endif  //_ftBuildStruct_h_

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
#include "ftBuildMember.h"

ftBuildMember::ftBuildMember() :
    m_typeId(-1),
    m_hashedName(-1),
    m_ptrCount(0),
    m_numDimensions(0),
    m_isFunctionPointer(0),
    m_lstat(0),
    m_undefined(0),
    m_isDependentType(false),
    m_arrays(),
    m_arraySize(1),
    m_line(-1)
{
}

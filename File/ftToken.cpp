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
#include "ftToken.h"

ftToken::ftToken() :
    m_id(ftFlags::FT_NULL_TOKEN),
    m_value(),
    m_arrayConstant(0)
{
}

ftToken::ftToken(int id, const String& val) :
    m_id(id),
    m_value(val),
    m_arrayConstant(0)
{
}

ftToken::ftToken(const ftToken& tok) :
    m_id(tok.m_id),
    m_value(tok.m_value),
    m_arrayConstant(tok.m_arrayConstant)
{
}
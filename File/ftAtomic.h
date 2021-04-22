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
#ifndef _ftAtomic_h_
#define _ftAtomic_h_

#include "ftTypes.h"

enum class ftAtomic
{
    FT_ATOMIC_CHAR,      // 0
    FT_ATOMIC_UCHAR,     // 1
    FT_ATOMIC_SHORT,     // 2
    FT_ATOMIC_USHORT,    // 3
    FT_ATOMIC_INT,       // 4
    FT_ATOMIC_LONG,      // 5
    FT_ATOMIC_ULONG,     // 6
    FT_ATOMIC_FLOAT,     // 7
    FT_ATOMIC_DOUBLE,    // 8
    FT_ATOMIC_INT64_T,   // 9
    FT_ATOMIC_UINT64_T,  // 10
    FT_ATOMIC_SCALAR_T,  // 11

    // Placement order matters
    // valid types are < FT_ATOMIC_VOID
    FT_ATOMIC_VOID,    // 13
    FT_ATOMIC_UNKNOWN  // 14
};

// Structure to manage the default data types
struct ftAtomicType
{
    const char* m_name;
    SKuint16    m_sizeof;
    ftAtomic    m_type;
    SKhash      m_hash;
};

class ftAtomicUtils
{
public:
    static ftAtomic getPrimitiveType(SKhash typeKey);
    static ftAtomic getPrimitiveType(const char* typeName);
    static bool     isInteger(SKhash typeKey);
    static bool     isReal(SKhash typeKey);
    static bool     isNumeric(SKhash typeKey);

    static bool canCast(SKhash typeKeyA, SKhash typeKeyB);

    static void cast(char*    source,
                     char*    destination,
                     ftAtomic sourceType,
                     ftAtomic destinationType,
                     SKsize   length);

    static void cast(char*    source,
                     SKsize   srcOffs,
                     char*    destination,
                     SKsize   dstOffs,
                     ftAtomic sourceType,
                     ftAtomic destinationType,
                     SKsize   length);

    static const ftAtomicType Types[];
    static const SKsize       NumberOfTypes;
};

#endif  //_ftAtomic_h_

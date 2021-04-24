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
#ifndef _ftAtomic_h_
#define _ftAtomic_h_

#include "ftTypes.h"

/// <summary>
/// Enumeration of the builtin primitive data types.
/// </summary>
enum class ftAtomic
{
    /// <summary>
    /// Definition for the 'char' data type
    /// </summary>
    FT_ATOMIC_CHAR,
    /// <summary>
    /// Definition for the 'uchar' data type
    /// </summary>
    FT_ATOMIC_UCHAR,
    /// <summary>
    /// Definition for the 'short' data type
    /// </summary>
    FT_ATOMIC_SHORT,
    /// <summary>
    /// Definition for the 'ushort' data type
    /// </summary>
    FT_ATOMIC_USHORT,
    /// <summary>
    /// Definition for the 'int' data type
    /// </summary>
    FT_ATOMIC_INT,
    /// <summary>
    /// Definition for the 'long' data type
    /// </summary>
    FT_ATOMIC_LONG,
    /// <summary>
    /// Definition for the 'ulong' data type
    /// </summary>
    FT_ATOMIC_ULONG,
    /// <summary>
    /// Definition for the 'float' data type
    /// </summary>
    FT_ATOMIC_FLOAT,
    /// <summary>
    /// Definition for the 'double' data type
    /// </summary>
    FT_ATOMIC_DOUBLE,
    /// <summary>
    /// Definition for the 'int64_t' data type
    /// </summary>
    FT_ATOMIC_INT64_T,
    /// <summary>
    /// Definition for the 'uint64_t' data type
    /// </summary>
    FT_ATOMIC_UINT64_T,
    /// <summary>
    /// Definition for the 'scalar_t' varying data type
    /// </summary>
    FT_ATOMIC_SCALAR_T,
    /// <summary>
    /// Definition for the 'scalar_t' varying data type
    /// </summary>
    /// <remarks>
    /// Placement order of the enumerated types matter.
    /// Valid types are tested to be less than FT_ATOMIC_VOID.
    /// </remarks>
    FT_ATOMIC_VOID,
    /// <summary>
    /// Max value to indicate an unknown data type.
    /// </summary>
    FT_ATOMIC_UNKNOWN
};

/// <summary>
/// Structure to manage the default data types
/// </summary>
struct ftAtomicType
{
    /// <summary>
    /// Defines the data type name.
    /// </summary>
    const char* name;

    /// <summary>
    /// Defines the size in bytes of the data type.
    /// </summary>
    SKuint16 size;

    /// <summary>
    /// Enumerated type code of the data type.
    /// </summary>
    ftAtomic type;

    /// <summary>
    /// Computed hash of the data type.
    /// </summary>
    SKhash hash;
};

/// <summary>
/// Utility class to aid with casting struct members.
/// </summary>
class ftAtomicUtils
{
public:
    /// <summary>
    /// Preforms a linear search of the builtin types to determine if
    /// the supplied key matches a builtin type.
    /// </summary>
    /// <param name="typeKey">The key to test.</param>
    /// <returns>the corresponding ftAtomic enumeration or FT_ATOMIC_UNKNOWN
    /// if it is not a primitive type.
    /// </returns>
    static ftAtomic getPrimitiveType(const SKhash& typeKey);

    /// <summary>
    /// Hashes the type name then calls getPrimitiveType with the hash key.
    /// </summary>
    /// <param name="typeName">The name to hash.</param>
    /// <returns>the corresponding ftAtomic enumeration or FT_ATOMIC_UNKNOWN
    /// if it is not a primitive type.
    /// </returns>
    static ftAtomic getPrimitiveType(const char* typeName);

    /// <summary>
    /// Test to determine if the supplied key is an integer type.
    /// </summary>
    /// <param name="typeKey">The key to test.</param>
    /// <returns>True if it is an integer type in the range of [FT_ATOMIC_CHAR,  FT_ATOMIC_SCALAR_T)</returns>
    static bool isInteger(const SKhash& typeKey);

    /// <summary>
    /// Test to determine if the supplied key is a floating point type.
    /// </summary>
    /// <param name="typeKey">The key to test.</param>
    /// <returns>True if it is a type in the range of [FT_ATOMIC_FLOAT,  FT_ATOMIC_SCALAR_T]</returns>
    static bool isReal(const SKhash& typeKey);

    /// <summary>
    /// Test to determine if it is an integer or a real number.
    /// </summary>
    /// <param name="typeKey">The key to test.</param>
    /// <returns>True if it is an integer or a real number otherwise returns false.</returns>
    static bool isNumeric(const SKhash& typeKey);

    /// <summary>
    /// Test to determine if the types can be cast together.
    /// </summary>
    /// <param name="typeKeyA">The LHS key to test.</param>
    /// <param name="typeKeyB">The RHS key to test.</param>
    /// <returns>True if the types are compatible otherwise returns false.</returns>
    static bool canCast(const SKhash& typeKeyA, const SKhash& typeKeyB);

    /// <summary>
    /// Casts the memory from the source into the destination memory.
    /// </summary>
    /// <param name="source">The source memory.</param>
    /// <param name="destination">The destination memory.</param>
    /// <param name="sourceType">The atomic type  for the source memory.</param>
    /// <param name="destinationType">The atomic type for the destination memory.</param>
    /// <param name="length">The total length of the memory.</param>
    static void cast(char*    source,
                     char*    destination,
                     ftAtomic sourceType,
                     ftAtomic destinationType,
                     SKsize   length);

    /// <summary>
    /// Casts the memory from the source into the destination memory with the option of separate offsets.
    /// </summary>
    /// <param name="source">The source memory.</param>
    /// <param name="srcOffs">The offset to the address in the source memory.</param>
    /// <param name="destination">The destination memory.</param>
    /// <param name="dstOffs">The offset to the address in the destination memory.</param>
    /// <param name="sourceType">The atomic type  for the source memory.</param>
    /// <param name="destinationType">The atomic type for the destination memory.</param>
    /// <param name="length">The total length of the memory.</param>
    static void cast(char*    source,
                     SKsize   srcOffs,
                     char*    destination,
                     SKsize   dstOffs,
                     ftAtomic sourceType,
                     ftAtomic destinationType,
                     SKsize   length);

    /// <summary>
    /// Static type table for the builtin atomic types.
    /// </summary>
    static const ftAtomicType Types[];

    /// <summary>
    /// The total size of the type table.
    /// </summary>
    static const SKsize NumberOfTypes;
};

#endif  //_ftAtomic_h_

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
#ifndef _ftEndianUtils_h_
#define _ftEndianUtils_h_

#include "ftTypes.h"

/// <summary>
/// Enumerated value for handling byte order
/// </summary>
enum ftEndian
{
    FT_ENDIAN_IS_BIG    = 0,
    FT_ENDIAN_IS_LITTLE = 1,
    FT_ENDIAN_NATIVE,
};

/// <summary>
/// Union to test the byte order.
/// </summary>
union ftEndianTest
{
    SKbyte bo[4];
    SKint32 test;
};

namespace ftEndianUtils
{
    const SKsize MaxSwapSpace = 8;

    /// <summary>
    /// Returns the current platforms byte order.
    /// </summary>
    /// <returns>The order if the MSB</returns>
    extern ftEndian getEndian();

    /// <summary>
    /// Tests the current platform's byte order against
    /// the supplied argument.
    /// </summary>
    /// <param name="endian">The desired byte order to test.</param>
    /// <returns>
    /// True if the current platform matches the supplied argument otherwise
    /// returns false.
    /// </returns>
    extern bool isEndian(const ftEndian& endian);

    /// <summary>
    /// Swaps the supplied 16bit unsigned integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKuint16 swap16(SKuint16 in);

    /// <summary>
    /// Swaps the supplied 32bit unsigned integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKuint32 swap32(const SKuint32& in);

    /// <summary>
    /// Swaps the supplied 16bit signed integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKint16 swap16(SKint16 in);

    /// <summary>
    /// Swaps the supplied 32bit signed integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKint32 swap32(const SKint32& in);

    /// <summary>
    /// Swaps the supplied 64bit unsigned integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKuint64 swap64(const SKuint64& in);

    /// <summary>
    /// Swaps the supplied 64bit signed integer.
    /// </summary>
    /// <param name="in">The input integer that should be swapped.</param>
    /// <returns>A swapped copy of the input.</returns>
    extern SKint64 swap64(const SKint64& in);

    /// <summary>
    /// Preforms a swap on the supplied array of 16bit unsigned integers.
    /// </summary>
    /// <param name="sp">The input integer array that should be swapped.</param>
    /// <param name="len">The number of elements in the array.</param>
    extern void swap16(SKuint16* sp, SKsize len);

    /// <summary>
    /// Preforms a swap on the supplied array of 32bit unsigned integers.
    /// </summary>
    /// <param name="ip">The input integer array that should be swapped.</param>
    /// <param name="len">The number of elements in the array.</param>
    extern void swap32(SKuint32* ip, SKsize len);

    /// <summary>
    /// Preforms a swap on the supplied array of 64bit unsigned integers.
    /// </summary>
    /// <param name="dp">The input integer array that should be swapped.</param>
    /// <param name="len">The number of elements in the array.</param>
    extern void swap64(SKuint64* dp, SKsize len);

};  // namespace ftEndianUtils

#endif  //_ftEndianUtils_h_

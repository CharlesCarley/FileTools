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
#ifndef _ftEndianUtils_h_
#define _ftEndianUtils_h_

#include "ftTypes.h"


typedef enum ftEndian
{
    FT_ENDIAN_IS_BIG    = 0,
    FT_ENDIAN_IS_LITTLE = 1,
    FT_ENDIAN_NATIVE,
} ftEndian;

typedef union ftEndianTest {
    SKbyte  bo[4];
    SKint32 test;
} ftEndianTest;

namespace ftEndianUtils
{
    // Swap64 needs 8 bytes to switch endian.
    // If there is ever a need to swap more than eight bytes
    // at a time this will have to change
    const SKsize MaxSwapSpace = 8; 

    extern ftEndian  getEndian(void);
    extern bool      isEndian(const ftEndian& endian);
    extern SKuint16 swap16(SKuint16 in);
    extern SKuint32 swap32(const SKuint32& in);
    extern SKint16  swap16(SKint16 in);
    extern SKint32  swap32(const SKint32& in);
    extern SKuint64 swap64(const SKuint64& in);
    extern void      swap16(SKuint16* sp, SKsize len);
    extern void      swap32(SKuint32* ip, SKsize len);
    extern void      swap64(SKuint64* dp, SKsize len);
    extern SKint64  swap64(const SKint64& in);
};


#endif  //_ftEndianUtils_h_
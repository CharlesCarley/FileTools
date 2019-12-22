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
    FBTbyte  bo[4];
    FBTint32 test;
} ftEndianTest;

namespace ftEndianUtils
{
    // Swap64 needs 8 bytes to switch endian.
    // If there is ever a need to swap more than eight bytes
    // at a time this will have to change
    const FBTsize MaxSwapSpace = 8; 

    extern ftEndian  getEndian(void);
    extern bool      isEndian(const ftEndian& endian);
    extern FBTuint16 swap16(FBTuint16 in);
    extern FBTuint32 swap32(const FBTuint32& in);
    extern FBTint16  swap16(FBTint16 in);
    extern FBTint32  swap32(const FBTint32& in);
    extern FBTuint64 swap64(const FBTuint64& in);
    extern void      swap16(FBTuint16* sp, FBTsize len);
    extern void      swap32(FBTuint32* ip, FBTsize len);
    extern void      swap64(FBTuint64* dp, FBTsize len);
    extern FBTint64  swap64(const FBTint64& in);
};


#endif  //_ftEndianUtils_h_
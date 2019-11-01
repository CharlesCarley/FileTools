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
#include "ftTypes.h"
#define FT_IN_SOURCE_FILE
#include "ftPlatformHeaders.h"

static const FBTuint32 charT   = ftCharHashKey("char").hash();
static const FBTuint32 ucharT  = ftCharHashKey("uchar").hash();
static const FBTuint32 shortT  = ftCharHashKey("short").hash();
static const FBTuint32 ushortT = ftCharHashKey("ushort").hash();
static const FBTuint32 intT    = ftCharHashKey("int").hash();
static const FBTuint32 longT   = ftCharHashKey("long").hash();
static const FBTuint32 ulongT  = ftCharHashKey("ulong").hash();
static const FBTuint32 floatT  = ftCharHashKey("float").hash();
static const FBTuint32 doubleT = ftCharHashKey("double").hash();
static const FBTuint32 voidT   = ftCharHashKey("void").hash();


ftAtomic ftGetPrimType(FBTuint32 typeKey)
{
    if (typeKey == charT)	return FT_ATOMIC_CHAR;
    if (typeKey == ucharT)	return FT_ATOMIC_UCHAR;
    if (typeKey == shortT)	return FT_ATOMIC_SHORT;
    if (typeKey == ushortT)	return FT_ATOMIC_USHORT;
    if (typeKey == intT)	return FT_ATOMIC_INT;
    if (typeKey == longT)	return FT_ATOMIC_LONG;
    if (typeKey == ulongT)	return FT_ATOMIC_ULONG;
    if (typeKey == floatT)	return FT_ATOMIC_FLOAT;
    if (typeKey == doubleT)	return FT_ATOMIC_DOUBLE;
    if (typeKey == voidT)	return FT_ATOMIC_VOID;
    return FT_ATOMIC_UNKNOWN;
}



ftAtomic ftGetPrimType(const char* typeName)
{
    if (!typeName)
        return ftAtomic::FT_ATOMIC_UNKNOWN;
    return ftGetPrimType(ftCharHashKey(typeName).hash());
}

bool ftIsIntType(FBTuint32 typeKey)
{
    ftAtomic tp = ftGetPrimType(typeKey);
    return tp < FT_ATOMIC_FLOAT;
}

bool ftIsFloatType(FBTuint32 typeKey)
{
    ftAtomic tp = ftGetPrimType(typeKey);
    return tp == FT_ATOMIC_FLOAT || tp == FT_ATOMIC_DOUBLE;
}

bool ftIsNumberType(FBTuint32 typeKey)
{
    ftAtomic tp = ftGetPrimType(typeKey);
    return tp != FT_ATOMIC_VOID && tp != FT_ATOMIC_UNKNOWN;
}

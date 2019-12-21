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
#include "ftAtomic.h"
#include "ftHashTypes.h"
#define FT_IN_SOURCE_FILE
#include "ftPlatformHeaders.h"


FBThash mkhash(const char* name)
{
    if (!name || !(*name))
        return -1;
    return ftCharHashKey(name).hash();
}



ftAtomic ftAtomicUtils::getPrimitiveType(FBThash typeKey)
{
    ftAtomic res = ftAtomic::FT_ATOMIC_UNKNOWN;
    size_t   i;
    for (i = 0; i < NumberOfTypes && res == ftAtomic::FT_ATOMIC_UNKNOWN; ++i)
    {
        if (Types[i].m_hash == typeKey)
            res = Types[i].m_type;
    }
    return res;
}


bool ftAtomicUtils::canCast(FBThash typeKeyA, FBThash typeKeyB)
{
    return isNumeric(typeKeyA) && isNumeric(typeKeyB);
}

ftAtomic ftAtomicUtils::getPrimitiveType(const char* typeName)
{
    if (!typeName || !(*typeName))
        return ftAtomic::FT_ATOMIC_UNKNOWN;
    return getPrimitiveType(ftCharHashKey(typeName).hash());
}


bool ftAtomicUtils::isInteger(FBThash typeKey)
{
    ftAtomic tp = getPrimitiveType(typeKey);
    return tp < ftAtomic::FT_ATOMIC_FLOAT;
}

bool ftAtomicUtils::isReal(FBThash typeKey)
{
    ftAtomic tp = getPrimitiveType(typeKey);
    return tp == ftAtomic::FT_ATOMIC_FLOAT || tp == ftAtomic::FT_ATOMIC_DOUBLE;
}

bool ftAtomicUtils::isNumeric(FBThash typeKey)
{
    ftAtomic tp = getPrimitiveType(typeKey);
    return tp != ftAtomic::FT_ATOMIC_VOID && tp != ftAtomic::FT_ATOMIC_UNKNOWN;
}


template <typename T>
double ftAtomicUtils_getValue(char*& src)
{
    double value = (double)(T)(*(T*)src);
    src += sizeof(T);
    return value;
}


template <typename T>
void ftAtomicUtils_setValue(char*& dest, const double& value)
{
    (*(T*)(dest)) = (T)(value);
    dest += sizeof(T);
}



void ftAtomicUtils_set(char*& destination, ftAtomic destinationType, const double& value)
{
    switch (destinationType)
    {
    case ftAtomic::FT_ATOMIC_CHAR:
        ftAtomicUtils_setValue<char>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_SHORT:
        ftAtomicUtils_setValue<short>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_USHORT:
        ftAtomicUtils_setValue<unsigned short>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_INT:
        ftAtomicUtils_setValue<int>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_LONG:
        ftAtomicUtils_setValue<long>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_FLOAT:
        ftAtomicUtils_setValue<float>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_DOUBLE:
        ftAtomicUtils_setValue<double>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_INT64_T:
        ftAtomicUtils_setValue<FBTint64>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_UINT64_T:
        ftAtomicUtils_setValue<FBTuint64>(destination, value);
        break;
    case ftAtomic::FT_ATOMIC_SCALAR_T:
        ftAtomicUtils_setValue<scalar_t>(destination, value);
        break;
    }
}

void ftAtomicUtils::cast(char*    source,
                         char*    destination,
                         ftAtomic sourceType,
                         ftAtomic destinationType,
                         FBTsize  length)
{
    if (!source || !destination)
    {
        printf("Invalid source and destination pointers!\n");
        return;
    }

    double  value = 0;
    FBTsize i     = 0;

    while (i < length)
    {
        switch (sourceType)
        {
        case ftAtomic::FT_ATOMIC_CHAR:
            value = ftAtomicUtils_getValue<char>(source);
            break;
        case ftAtomic::FT_ATOMIC_SHORT:
            value = ftAtomicUtils_getValue<short>(source);
            break;
        case ftAtomic::FT_ATOMIC_USHORT:
            value = ftAtomicUtils_getValue<unsigned short>(source);
            break;
        case ftAtomic::FT_ATOMIC_INT:
            value = ftAtomicUtils_getValue<int>(source);
            break;
        case ftAtomic::FT_ATOMIC_LONG:
            value = ftAtomicUtils_getValue<long>(source);
            break;
        case ftAtomic::FT_ATOMIC_FLOAT:
            value = ftAtomicUtils_getValue<float>(source);
            break;
        case ftAtomic::FT_ATOMIC_DOUBLE:
            value = ftAtomicUtils_getValue<double>(source);
            break;
        case ftAtomic::FT_ATOMIC_INT64_T:
            value = ftAtomicUtils_getValue<FBTint64>(source);
            break;
        case ftAtomic::FT_ATOMIC_UINT64_T:
            value = ftAtomicUtils_getValue<FBTuint64>(source);
            break;
        case ftAtomic::FT_ATOMIC_SCALAR_T:
            value = ftAtomicUtils_getValue<scalar_t>(source);
            break;
        default:
            value = 0;
            break;
        }

        ftAtomicUtils_set(destination, destinationType, value);
        ++i;
    }
}


void ftAtomicUtils::cast(char*    source,
                         FBTsize  srcoffs,
                         char*    destination,
                         FBTsize  dstoffs,
                         ftAtomic sourceType,
                         ftAtomic destinationType,
                         FBTsize  length)
{
    if (srcoffs != -1 && dstoffs != -1)
    {
        cast(source + srcoffs,
             destination + dstoffs,
             sourceType,
             destinationType,
             length);
    }
}



const ftAtomicType ftAtomicUtils::Types[] = {
    {"char", sizeof(char), ftAtomic::FT_ATOMIC_CHAR, mkhash("char")},
    {"uchar", sizeof(char), ftAtomic::FT_ATOMIC_UCHAR, mkhash("uchar")},
    {"short", sizeof(short), ftAtomic::FT_ATOMIC_SHORT, mkhash("short")},
    {"ushort", sizeof(short), ftAtomic::FT_ATOMIC_USHORT, mkhash("ushort")},
    {"int", sizeof(int), ftAtomic::FT_ATOMIC_INT, mkhash("int")},
    {"long", sizeof(long), ftAtomic::FT_ATOMIC_LONG, mkhash("long")},
    {"ulong", sizeof(long), ftAtomic::FT_ATOMIC_ULONG, mkhash("ulong")},
    {"float", sizeof(float), ftAtomic::FT_ATOMIC_FLOAT, mkhash("float")},
    {"double", sizeof(double), ftAtomic::FT_ATOMIC_DOUBLE, mkhash("double")},
    {"int64_t", sizeof(FBTint64), ftAtomic::FT_ATOMIC_INT64_T, mkhash("int64_t")},
    {"uint64_t", sizeof(FBTint64), ftAtomic::FT_ATOMIC_UINT64_T, mkhash("uint64_t")},
#ifdef ftSCALAR_DOUBLE
    {"scalar_t", sizeof(double), ftAtomic::FT_ATOMIC_SCALAR_T, mkhash("scalar_t")},
#else
    {"scalar_t", sizeof(float), ftAtomic::FT_ATOMIC_SCALAR_T, mkhash("scalar_t")},
#endif
    {"void", 0, ftAtomic::FT_ATOMIC_VOID, mkhash("void")},
};

const size_t ftAtomicUtils::NumberOfTypes = sizeof(ftAtomicUtils::Types) / sizeof(ftAtomicType);

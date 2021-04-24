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
#ifndef _ftTableBuilder_h_
#define _ftTableBuilder_h_

#include "Utils/skFixedString.h"
#include "ftTypes.h"

struct ftMaxAllocSize
{
    SKuint32 names;
    SKuint32 types;
    SKuint32 lengths;
    SKuint32 structTable;
    SKuint32 structures;
};

struct ftBuildMember
{
    ftBuildMember() :
        typeId(-1),
        hashedName(-1),
        ptrCount(0),
        numDimensions(0),
        isFunctionPointer(0),
        undefined(0),
        isDependentType(false),
        arrays(),
        arraySize(1),
        line(-1)
    {
    }

    ftId         type;
    ftId         name;
    FTtype       typeId;
    FTtype       hashedName;
    SKuint16     ptrCount;
    SKuint16     numDimensions;
    int          isFunctionPointer;
    int          undefined;
    bool         isDependentType;
    ftArraySlots arrays;
    SKsize       arraySize;
    ftPath       path;
    SKsize       line;
};

class ftBuildStruct
{
public:
    typedef skArray<ftBuildStruct> Array;
    typedef skArray<ftBuildMember> Variables;

public:
    ftBuildStruct() :
        structureId(SK_NPOS),
        nrDependentTypes(0),
        line(-1)
    {
    }

    SKsize    structureId;
    ftId      name;
    Variables data;
    SKsize    nrDependentTypes;
    ftPath    path;
    SKsize    line;
};

/// <summary>
/// Utility class to build a table
/// </summary>
class ftTableBuilder
{
public:
    typedef skArray<FTtype> IntPtrArray;
    typedef skArray<FTtype> TypeArray;

public:
    /// <summary>
    /// Default constructor.
    /// </summary>
    ftTableBuilder();
    ~ftTableBuilder();

    int getLengths(ftBuildStruct::Array& structBuilders);

    int    getTLengths(ftBuildStruct::Array& structBuilders);
    void   makeBuiltinTypes();
    bool   hasType(const ftId& type) const;
    FTtype addType(const ftId& type, const SKuint16& len);
    SKsize addName(const ftId& lookup);

    ftMaxAllocSize   allocationSizes;
    ftStringPtrArray name;
    ftStringPtrArray typeLookup;
    IntPtrArray      typeLengths;
    IntPtrArray      typeLengths64Bit;
    TypeArray        structures;
    ftStringPtrArray undefined;
    SKuint32         numberOfBuiltIn;
};

#endif  //_ftTableBuilder_h_

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

#include <cstdio>
#include "Utils/skArray.h"
#include "Utils/skHash.h"
#include "ftAtomic.h"
#include "ftCompiler.h"
#include "ftConfig.h"
#include "ftScanner.h"

#define FT_IS_VALID_TOKEN(x) ((x) > 0)

using namespace ftFlags;

ftTableBuilder::ftTableBuilder() :
    allocationSizes{},
    numberOfBuiltIn(0)
{
    allocationSizes.names       = 0;
    allocationSizes.types       = 0;
    allocationSizes.lengths     = 0;
    allocationSizes.structTable = 0;
    allocationSizes.structures  = 0;
}

ftTableBuilder::~ftTableBuilder() = default;

void ftTableBuilder::makeBuiltinTypes()
{
    for (size_t i = 0; i < ftAtomicUtils::NumberOfTypes; ++i)
    {
        const ftAtomicType& type = ftAtomicUtils::Types[i];
        addType(type.name, type.size);
    }

    numberOfBuiltIn = typeLookup.size();
}

FTtype ftTableBuilder::addType(const ftId& type, const SKuint16& len)
{
    SKsize loc = typeLookup.find(type);
    if (loc == typeLookup.npos)
    {
        allocationSizes.types += type.size() + 1;
        allocationSizes.lengths += sizeof(FTtype);
        loc = typeLookup.size();
        if (loc > 0xFFFF)
        {
            printf("Type limit exceeded\n");
            return SK_NPOS16;
        }
        typeLookup.push_back(type);
        typeLengths.push_back(len);
        typeLengths64Bit.push_back(len);
    }
    return (FTtype)loc;
}

bool ftTableBuilder::hasType(const ftId& type) const
{
    return typeLookup.find(type) != typeLookup.npos;
}

SKsize ftTableBuilder::addName(const ftId& lookup)
{
    SKsize loc;
    if ((loc = name.find(lookup)) == name.npos)
    {
        allocationSizes.names += name.size() + 1;
        loc = name.size();
        name.push_back(lookup);
    }
    return loc;
}

int ftTableBuilder::getLengths(ftBuildStruct::Array& structBuilders)
{
    makeBuiltinTypes();

    ftBuildStruct::Array::Iterator bit = structBuilders.iterator();
    while (bit.hasMoreElements())
    {
        ftBuildStruct& bs = bit.getNext();

        bs.structureId = addType(bs.name, 0);

        structures.push_back((SKuint16)bs.structureId);
        structures.push_back((SKuint16)bs.data.size());

        allocationSizes.structures += sizeof(FTtype) * 2;

        ftBuildStruct::Variables::Iterator it = bs.data.iterator();
        while (it.hasMoreElements())
        {
            ftBuildMember& cVar = it.getNext();

            cVar.typeId     = addType(cVar.type, 0);
            cVar.hashedName = (FTtype)addName(cVar.name);

            structures.push_back(cVar.typeId);
            structures.push_back(cVar.hashedName);

            allocationSizes.structures += sizeof(FTtype) * 2;
        }
    }

    return getTLengths(structBuilders);
}

int ftTableBuilder::getTLengths(ftBuildStruct::Array& structBuilders)
{
    ftBuildStruct* structs = structBuilders.ptr();
    const SKsize   total   = structBuilders.size();

    SKsize next = total, prev = 0;

    FTtype*          tln64  = typeLengths64Bit.ptr();
    FTtype*          tLens  = typeLengths.ptr();
    int              status = LNK_OK;
    ftStringPtrArray missingReport, missing;

    FTtype firstNonAtomic = 0;
    if (structs)
        firstNonAtomic = (FTtype)structs[0].structureId;

    while (next != prev && structs)
    {
        prev = next;
        next = 0;

        for (SKsize i = 0; i < total; ++i)
        {
            ftBuildStruct& cur = structs[i];

            if (tLens[cur.structureId] != 0)
            {
                SKuint32 pos;
                if ((pos = missingReport.find(cur.name)) != missingReport.npos)
                    missingReport.remove(pos);
            }
            else
            {
                ftBuildMember* member = cur.data.ptr();
                const SKsize   nrEle  = cur.data.size();

                SKsize len    = 0;
                SKsize fake64 = 0;
                bool   hasPtr = false;

                for (SKsize e = 0; e < nrEle; ++e)
                {
                    ftBuildMember& v  = member[e];
                    const SKsize   ct = v.typeId;

                    if (v.ptrCount > 0)
                    {
                        hasPtr = true;
                        if (len % FT_VOID_P)
                        {
                            printf(v.path.c_str(),
                                   v.line,
                                   "align %i: %s %s add %i bytes\n",
                                   FT_VOID_P,
                                   v.type.c_str(),
                                   v.name.c_str(),
                                   FT_VOID_P - len % FT_VOID_P);

                            status |= LNK_ALIGNMENT_P;
                        }
                        if (fake64 % 8)
                        {
                            printf(v.path.c_str(),
                                   v.line,
                                   "align %i: %s %s add %i bytes\n",
                                   8,
                                   v.type.c_str(),
                                   v.name.c_str(),
                                   8 - fake64 % 8);

                            status |= LNK_ALIGNMENT_P;
                        }
                        len += FT_VOID_P * v.arraySize;
                        fake64 += 8 * v.arraySize;
                    }
                    else if (tLens[ct])
                    {
                        if (ct >= firstNonAtomic)
                        {
                            if (FT_VOID_8 && len % 8)
                            {
                                printf(v.path.c_str(),
                                       v.line,
                                       "align: %i alignment error add %i bytes\n",
                                       8,
                                       8 - len % 8);
                                status |= LNK_ALIGNMENT_S;
                            }
                        }

                        if (tLens[ct] > 3 && len % 4)
                        {
                            printf(cur.path.c_str(),
                                   v.line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   4,
                                   cur.name.c_str(),
                                   v.name.c_str(),
                                   4 - len % 4);
                            status |= LNK_ALIGNMENT_4;
                        }
                        else if (tLens[ct] == 2 && len % 2)
                        {
                            printf(cur.path.c_str(),
                                   v.line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   2,
                                   cur.name.c_str(),
                                   v.name.c_str(),
                                   2 - len % 2);
                            status |= LNK_ALIGNMENT_2;
                        }

                        len += tLens[ct] * v.arraySize;
                        fake64 += tln64[ct] * v.arraySize;
                    }
                    else
                    {
                        next++;
                        len = 0;
                        if (missingReport.find(cur.name) == missingReport.npos)
                            missingReport.push_back(cur.name);

                        tln64[cur.structureId] = 0;
                        tLens[cur.structureId] = 0;
                        break;
                    }
                }

                tln64[cur.structureId] = (FTtype)fake64;
                tLens[cur.structureId] = (FTtype)len;

                if (len != 0)
                {
                    if (hasPtr || fake64 != len)
                    {
                        if (fake64 % 8)
                        {
                            printf(cur.path.c_str(),
                                   cur.line,
                                   "64Bit alignment, in %s add %i bytes\n",
                                   cur.name.c_str(),
                                   8 - fake64 % 8);
                            status |= LNK_ALIGNMENT_8;
                        }
                    }

                    if (len % 4)
                    {
                        printf(cur.path.c_str(),
                               cur.line,
                               "align 4: in %s add %i bytes\n",
                               cur.name.c_str(),
                               4 - len % 4);
                        status |= LNK_ALIGNMENT_4;
                    }
                }
            }
        }
    }

    if (!missingReport.empty())
    {
        status |= LNK_UNDEFINED_TYPES;
        ftStringPtrArray::Iterator it = missingReport.iterator();
        while (it.hasMoreElements())
        {
            ftId& id = it.getNext();
            printf("Link error: undefined reference to type '%s'\n", id.c_str());
        }
    }

    if (FT_DEBUG >= 3)
    {
        ftBuildStruct::Array::Iterator bit = structBuilders.iterator();

        while (bit.hasMoreElements())
        {
            ftBuildStruct& bs = bit.getNext();

            printf(bs.path.c_str(),
                   bs.line,
                   "typeid (%s):%i\n",
                   bs.name.c_str(),
                   bs.structureId);

#if FT_DEBUG > 0

            if (!bs.data.empty())
            {
                ftBuildStruct::Variables::Iterator it = bs.data.iterator();
                while (it.hasMoreElements())
                {
                    ftBuildMember& var = it.getNext();
                    printf(var.path.c_str(),
                           var.line,
                           "typeid:%-8inameid:%-8isizeof:%-8i%s %s\n",
                           var.typeId,
                           var.hashedName,
                           (var.ptrCount > 0 ? FT_VOID_P : tLens[var.typeId]) * var.arraySize,
                           var.type.c_str(),
                           var.name.c_str());
                }
            }
#endif
        }
    }
    return status;
}

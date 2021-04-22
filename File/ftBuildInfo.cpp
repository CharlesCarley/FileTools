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
#include <cstdio>
#include "Utils/skArray.h"
#include "Utils/skHash.h"
#include "ftAtomic.h"
#include "ftCompiler.h"
#include "ftConfig.h"
#include "ftScanner.h"

#define FT_IS_VALID_TOKEN(x) ((x) > 0)

using namespace ftFlags;

ftBuildInfo::ftBuildInfo() :
    m_alloc{},
    m_numberOfBuiltIn(0)
{
    m_alloc.m_name       = 0;
    m_alloc.m_type       = 0;
    m_alloc.m_tlen       = 0;
    m_alloc.m_strc       = 0;
    m_alloc.m_structures = 0;
}

ftBuildInfo::~ftBuildInfo() = default;

void ftBuildInfo::reserve(void)
{
    m_name.reserve(FT_MAX_TABLE);
    m_typeLookup.reserve(FT_MAX_TABLE);
    m_tlen.reserve(FT_MAX_TABLE);
    m_strc.reserve(FT_MAX_TABLE * FT_MAX_MEMBERS);
}

void ftBuildInfo::makeBuiltinTypes(void)
{
    for (size_t i = 0; i < ftAtomicUtils::NumberOfTypes; ++i)
    {
        const ftAtomicType& type = ftAtomicUtils::Types[i];
        addType(type.m_name, type.m_sizeof);
    }

    m_numberOfBuiltIn = m_typeLookup.size();
}

SKtype ftBuildInfo::addType(const ftId& type, const SKuint16& len)
{
    SKsize loc = m_typeLookup.find(type);
    if (loc == m_typeLookup.npos)
    {
        m_alloc.m_type += type.size() + 1;
        m_alloc.m_tlen += sizeof(SKtype);
        loc = m_typeLookup.size();
        if (loc > 0xFFFF)
        {
            printf("Type limit exceeded\n");
            return SK_NPOS16;
        }
        m_typeLookup.push_back(type);
        m_tlen.push_back(len);
        m_64ln.push_back(len);
    }
    return (SKtype)loc;
}

bool ftBuildInfo::hasType(const ftId& type) const
{
    return m_typeLookup.find(type) != m_typeLookup.npos;
}

SKsize ftBuildInfo::addName(const ftId& name)
{
    SKsize loc;
    if ((loc = m_name.find(name)) == m_name.npos)
    {
        m_alloc.m_name += name.size() + 1;
        loc = m_name.size();
        m_name.push_back(name);
    }
    return loc;
}

int ftBuildInfo::getLengths(ftBuildStruct::Array& structBuilders)
{
    makeBuiltinTypes();

    ftBuildStruct::Array::Iterator bit = structBuilders.iterator();
    while (bit.hasMoreElements())
    {
        ftBuildStruct& bs = bit.getNext();

        bs.m_structId     = addType(bs.m_name, 0);

        m_strc.push_back((SKuint16)bs.m_structId);
        m_strc.push_back((SKuint16)bs.m_data.size());

        m_alloc.m_strc += sizeof(SKtype) * 2;

        ftBuildStruct::Variables::Iterator it = bs.m_data.iterator();
        while (it.hasMoreElements())
        {
            ftBuildMember& cVar = it.getNext();

            cVar.m_typeId     = addType(cVar.m_type, 0);
            cVar.m_hashedName = (SKtype)addName(cVar.m_name);

            m_strc.push_back(cVar.m_typeId);
            m_strc.push_back(cVar.m_hashedName);

            m_alloc.m_strc += sizeof(SKtype) * 2;
        }
    }

    return getTLengths(structBuilders);
}

int ftBuildInfo::getTLengths(ftBuildStruct::Array& structBuilders)
{
    ftBuildStruct* structs = structBuilders.ptr();
    const SKsize  total   = structBuilders.size();

    SKsize next = total, prev = 0;

    SKtype*         tln64  = m_64ln.ptr();
    SKtype*         tLens  = m_tlen.ptr();
    int              status = LNK_OK;
    ftStringPtrArray missingReport, missing;

    SKtype firstNonAtomic = 0;
    if (structs)
        firstNonAtomic = (SKtype)structs[0].m_structId;

    while (next != prev && structs)
    {
        prev = next;
        next = 0;

        for (SKsize i = 0; i < total; ++i)
        {
            ftBuildStruct& cur = structs[i];

            if (tLens[cur.m_structId] != 0)
            {
                SKuint32 pos;
                if ((pos = missingReport.find(cur.m_name)) != missingReport.npos)
                    missingReport.remove(pos);
            }
            else
            {
                ftBuildMember* member = cur.m_data.ptr();
                const SKsize  nrEle  = cur.m_data.size();

                SKsize len    = 0;
                SKsize fake64 = 0;
                bool    hasPtr = false;

                for (SKsize e = 0; e < nrEle; ++e)
                {
                    ftBuildMember& v  = member[e];
                    const SKsize  ct = v.m_typeId;

                    if (v.m_ptrCount > 0)
                    {
                        hasPtr = true;
                        if (len % FT_VOIDP)
                        {
                            printf(v.m_path.c_str(),
                                   v.m_line,
                                   "align %i: %s %s add %i bytes\n",
                                   FT_VOIDP,
                                   v.m_type.c_str(),
                                   v.m_name.c_str(),
                                   FT_VOIDP - len % FT_VOIDP);

                            status |= LNK_ALIGNEMENTP;
                        }
                        if (fake64 % 8)
                        {
                            printf(v.m_path.c_str(),
                                   v.m_line,
                                   "align %i: %s %s add %i bytes\n",
                                   8,
                                   v.m_type.c_str(),
                                   v.m_name.c_str(),
                                   8 - fake64 % 8);

                            status |= LNK_ALIGNEMENTP;
                        }
                        len += FT_VOIDP * v.m_arraySize;
                        fake64 += 8 * v.m_arraySize;
                    }
                    else if (tLens[ct])
                    {
                        if (ct >= firstNonAtomic)
                        {
                            if (FT_VOID8 && len % 8)
                            {
                                printf(v.m_path.c_str(),
                                       v.m_line,
                                       "align: %i alignment error add %i bytes\n",
                                       8,
                                       8 - len % 8);
                                status |= LNK_ALIGNEMENTS;
                            }
                        }

                        if (tLens[ct] > 3 && len % 4)
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   4,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   4 - len % 4);
                            status |= LNK_ALIGNEMENT4;
                        }
                        else if (tLens[ct] == 2 && len % 2)
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   2,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   2 - len % 2);
                            status |= LNK_ALIGNEMENT2;
                        }

                        len += tLens[ct] * v.m_arraySize;
                        fake64 += tln64[ct] * v.m_arraySize;
                    }
                    else
                    {
                        next++;
                        len = 0;
                        if (missingReport.find(cur.m_name) == missingReport.npos)
                            missingReport.push_back(cur.m_name);

                        tln64[cur.m_structId] = 0;
                        tLens[cur.m_structId] = 0;
                        break;
                    }
                }

                tln64[cur.m_structId] = (SKtype)fake64;
                tLens[cur.m_structId] = (SKtype)len;

                if (len != 0)
                {
                    if (hasPtr || fake64 != len)
                    {
                        if (fake64 % 8)
                        {
                            printf(cur.m_path.c_str(),
                                   cur.m_line,
                                   "64Bit alignment, in %s add %i bytes\n",
                                   cur.m_name.c_str(),
                                   8 - fake64 % 8);
                            status |= LNK_ALIGNEMENT8;
                        }
                    }

                    if (len % 4)
                    {
                        printf(cur.m_path.c_str(),
                               cur.m_line,
                               "align 4: in %s add %i bytes\n",
                               cur.m_name.c_str(),
                               4 - len % 4);
                        status |= LNK_ALIGNEMENT4;
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

            printf(bs.m_path.c_str(),
                   bs.m_line,
                   "typeid (%s):%i\n",
                   bs.m_name.c_str(),
                   bs.m_structId);

#if FT_DEBUG > 0

            if (!bs.m_data.empty())
            {
                ftBuildStruct::Variables::Iterator it = bs.m_data.iterator();
                while (it.hasMoreElements())
                {
                    ftBuildMember& var = it.getNext();
                    printf(var.m_path.c_str(),
                           var.m_line,
                           "typeid:%-8inameid:%-8isizeof:%-8i%s %s\n",
                           var.m_typeId,
                           var.m_hashedName,
                           (var.m_ptrCount > 0 ? FT_VOIDP : tLens[var.m_typeId]) * var.m_arraySize,
                           var.m_type.c_str(),
                           var.m_name.c_str());
                }
            }
#endif
        }
    }
    return status;
}

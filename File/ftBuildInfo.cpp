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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Utils/skArray.h"
#include "Utils/skHash.h"
#include "ftAtomic.h"
#include "ftCompiler.h"
#include "ftConfig.h"
#include "ftLogger.h"
#include "ftScanner.h"
#include "ftStreams.h"
#define ftValidToken(x) (x > 0)

using namespace ftFlags;

ftBuildInfo::ftBuildInfo()
{
    m_numberOfBuiltIn    = 0;
    m_alloc.m_name       = 0;
    m_alloc.m_type       = 0;
    m_alloc.m_tlen       = 0;
    m_alloc.m_strc       = 0;
    m_alloc.m_structures = 0;
}

ftBuildInfo::~ftBuildInfo()
{
}


void ftBuildInfo::reserve(void)
{
    m_name.reserve(FT_MAX_TABLE);
    m_typeLookup.reserve(FT_MAX_TABLE);
    m_tlen.reserve(FT_MAX_TABLE);
    m_strc.reserve(FT_MAX_TABLE * FT_MAX_MEMBERS);
}

void ftBuildInfo::makeBuiltinTypes(void)
{
    size_t i;
    for (i = 0; i < ftAtomicUtils::NumberOfTypes; ++i)
    {
        const ftAtomicType& type = ftAtomicUtils::Types[i];
        addType(type.m_name, (SKuint32)type.m_sizeof);
    }
    m_numberOfBuiltIn = m_typeLookup.size();
}

FBTsize ftBuildInfo::addType(const ftId& type, const FBTuint32& len)
{
    FBTsize loc = m_typeLookup.find(type);
    if (loc == m_typeLookup.npos)
    {
        m_alloc.m_type += type.size() + 1;
        m_alloc.m_tlen += sizeof(FBTtype);
        loc = m_typeLookup.size();

        m_typeLookup.push_back(type);
        m_tlen.push_back(len);
        m_64ln.push_back(len);
    }
    return loc;
}

bool ftBuildInfo::hasType(const ftId& type)
{
    return m_typeLookup.find(type) != m_typeLookup.npos;
}

FBTsize ftBuildInfo::addName(const ftId& name)
{
    FBTsize loc;
    if ((loc = m_name.find(name)) == m_name.npos)
    {
        m_alloc.m_name += name.size() + 1;
        loc = m_name.size();
        m_name.push_back(name);
    }
    return loc;
}

int ftBuildInfo::getLengths(ftCompileStruct::Array& struct_builders)
{
    makeBuiltinTypes();

    ftCompileStruct::Array::Iterator bit = struct_builders.iterator();
    while (bit.hasMoreElements())
    {
        ftCompileStruct& bs = bit.getNext();
        bs.m_structId       = addType(bs.m_name, 0);

        m_strc.push_back((SKuint16)bs.m_structId);
        m_strc.push_back((SKuint16)bs.m_data.size());

        m_alloc.m_strc += (sizeof(FBTtype) * 2);

        ftCompileStruct::Variables::Iterator it = bs.m_data.iterator();
        while (it.hasMoreElements())
        {
            ftVariable& cvar = it.getNext();

            cvar.m_typeId     = (int)addType(cvar.m_type, 0);
            cvar.m_hashedName = (FBTtype)addName(cvar.m_name);

            m_strc.push_back(cvar.m_typeId);
            m_strc.push_back(cvar.m_hashedName);

            m_alloc.m_strc += (sizeof(FBTtype) * 2);
        }
    }

    return getTLengths(struct_builders);
}


int ftBuildInfo::getTLengths(ftCompileStruct::Array& struct_builders)
{
    ftCompileStruct* strcs = struct_builders.ptr();
    FBTsize          tot   = struct_builders.size(), i, e;

    FBTsize next = tot, prev = 0;

    FBTtype*         tln64  = m_64ln.ptr();
    FBTtype*         tlens  = m_tlen.ptr();
    FBTsize          nrel   = 0, ct, len, fake64;
    int              status = LNK_OK;
    ftStringPtrArray m_missingReport, m_zpdef;

    FBTtype ft_start = 0;
    if (strcs)
        ft_start = (FBTtype)strcs[0].m_structId;

    ftVariable* vptr = 0;
    while (next != prev && strcs)
    {
        prev = next;
        next = 0;

        for (i = 0; i < tot; ++i)
        {
            ftCompileStruct& cur = strcs[i];
            if (tlens[cur.m_structId] != 0)
            {
                FBTuint32 pos;
                if ((pos = m_missingReport.find(cur.m_name)) != m_missingReport.npos)
                    m_missingReport.remove(pos);
            }
            else
            {
                vptr = cur.m_data.ptr();
                nrel = cur.m_data.size();

                len         = 0;
                fake64      = 0;
                bool hasPtr = false;
                for (e = 0; e < nrel; ++e)
                {
                    ftVariable& v = vptr[e];
                    ct            = v.m_typeId;

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
                                   FT_VOIDP - (len % FT_VOIDP));

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
                                   8 - (fake64 % 8));

                            status |= LNK_ALIGNEMENTP;
                        }
                        len += FT_VOIDP * v.m_arraySize;
                        fake64 += 8 * v.m_arraySize;
                    }
                    else if (tlens[ct])
                    {
                        if (ct >= ft_start)
                        {
                            if (FT_VOID8 && (len % 8))
                            {
                                printf(v.m_path.c_str(),
                                       v.m_line,
                                       "align: %i alignment error add %i bytes\n",
                                       8,
                                       8 - (len % 8));
                                status |= LNK_ALIGNEMENTS;
                            }
                        }

                        if (tlens[ct] > 3 && (len % 4))
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   4,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   4 - (len % 4));
                            status |= LNK_ALIGNEMENT4;
                        }
                        else if (tlens[ct] == 2 && (len % 2))
                        {
                            printf(cur.m_path.c_str(),
                                   v.m_line,
                                   "align %i: in %s::%s add %i bytes\n",
                                   2,
                                   cur.m_name.c_str(),
                                   v.m_name.c_str(),
                                   2 - (len % 2));
                            status |= LNK_ALIGNEMENT2;
                        }

                        len += tlens[ct] * v.m_arraySize;
                        fake64 += tln64[ct] * v.m_arraySize;
                    }
                    else
                    {
                        next++;
                        len = 0;
                        if (m_missingReport.find(cur.m_name) == m_missingReport.npos)
                            m_missingReport.push_back(cur.m_name);

                        tln64[cur.m_structId] = 0;
                        tlens[cur.m_structId] = 0;
                        break;
                    }
                }

                tln64[cur.m_structId] = (FBTtype)fake64;
                tlens[cur.m_structId] = (FBTtype)len;

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
                                   8 - (fake64 % 8));
                            status |= LNK_ALIGNEMENT8;
                        }
                    }

                    if (len % 4)
                    {
                        printf(cur.m_path.c_str(),
                               cur.m_line,
                               "align 4: in %s add %i bytes\n",
                               cur.m_name.c_str(),
                               4 - (len % 4));
                        status |= LNK_ALIGNEMENT4;
                    }
                }
            }
        }
    }


    if (!m_missingReport.empty())
    {
        status |= LNK_UNDEFINED_TYPES;
        ftStringPtrArray::Iterator it = m_missingReport.iterator();
        while (it.hasMoreElements())
        {
            ftId& id = it.getNext();
            printf("Link error: undefined reference to type '%s'\n", id.c_str());
        }
    }

    if (FT_DEBUG >= 3)
    {
        ftCompileStruct::Array::Iterator bit = struct_builders.iterator();

        while (bit.hasMoreElements())
        {
            ftCompileStruct& bs = bit.getNext();

            printf(bs.m_path.c_str(), bs.m_line, "typeid (%s):%i\n", bs.m_name.c_str(), bs.m_structId);
            if (FT_DEBUG > 0 && !bs.m_data.empty())
            {
                ftCompileStruct::Variables::Iterator it = bs.m_data.iterator();
                while (it.hasMoreElements())
                {
                    ftVariable& cvar = it.getNext();
                    printf(cvar.m_path.c_str(), cvar.m_line, "typeid:%-8inameid:%-8isizeof:%-8i%s %s\n", cvar.m_typeId, cvar.m_hashedName, (cvar.m_ptrCount > 0 ? FT_VOIDP : tlens[cvar.m_typeId]) * cvar.m_arraySize, cvar.m_type.c_str(), cvar.m_name.c_str());
                }
            }
        }
    }
    return status;
}

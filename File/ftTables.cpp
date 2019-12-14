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
#define FT_IN_SOURCE_FILE
#include "ftTables.h"
#include "ftPlatformHeaders.h"
#include "ftHashTypes.h"



ftBinTables::ftBinTables() :
    m_name(0),
    m_type(0),
    m_tlen(0),
    m_strc(0),
    m_nameNr(0),
    m_typeNr(0),
    m_strcNr(0),
    m_ptrLength(FT_VOIDP),
    m_otherBlock(0),
    m_otherLen(0)
{
}

ftBinTables::ftBinTables(void* ptr, FBTsize len, FBTuint8 ptrSize) :
    m_name(0),
    m_type(0),
    m_tlen(0),
    m_strc(0),
    m_nameNr(0),
    m_typeNr(0),
    m_strcNr(0),
    m_ptrLength(ptrSize),
    m_otherBlock(ptr),
    m_otherLen(len)
{
}



ftBinTables::~ftBinTables()
{
    OffsM::Iterator it = m_offs.iterator();
    while (it.hasMoreElements())
        delete it.getNext();

    ::free(m_name);
    ::free(m_type);
    ::free(m_tlen);
    ::free(m_strc);
}


ftBinTables::OffsM::PointerType ftBinTables::getOffsetPtr()
{
    return m_offs.ptr();
}


ftBinTables::OffsM::SizeType ftBinTables::getOffsetCount()
{
    return m_offs.size();
}

ftStruct* ftBinTables::findStructByName(const ftCharHashKey& kvp)
{
    FBTtype i;
    i = findTypeId(kvp);
    if (i != SK_NPOS16)
        return m_offs.at(i);
    return nullptr;
}


bool ftBinTables::read(bool swap)
{
    if (m_otherBlock && m_otherLen > 0)
        return read(m_otherBlock, m_otherLen, swap);
    return false;
}


bool ftBinTables::read(const void* ptr, const FBTsize& len, bool swap)
{
    FBTuint32 *ip = 0, i, j, k, nl;
    FBTtype*   tp = 0;

    char* cp = (char*)ptr;
    if (!ftCharNEq(cp, ftIdNames::ftSDNA, 4))
    {
        printf("Bin table is missing the start id!\n");
        return false;
    }

    cp += 4;
    if (!ftCharNEq(cp, ftIdNames::ftNAME, 4))
    {
        printf("Bin table is missing the name id!\n");
        return false;
    }

    cp += 4;
    FBTintPtr opad;

    ip = (FBTuint32*)cp;
    nl = *ip++;
    cp = (char*)ip;

    if (swap)
        nl = ftSwap32(nl);


    if (nl > FT_MAX_TABLE)
    {
        printf("Max name table size exceeded!\n");
        return false;
    }
    else
        m_name = (Names)::malloc((nl * sizeof(ftName)) + 1);



    i = 0;
    while (i < nl && i < FT_MAX_TABLE)
    {
        ftName name = {cp, (int)i, ftCharHashKey(cp).hash(), 0, 0, 0, 1};

        ftFixedString<64> bn;

        // re-lex
        while (*cp)
        {
            switch (*cp)
            {
            default:
            {
                bn.push_back(*cp);
                ++cp;
                break;
            }
            case ')':
            case ']':
                ++cp;
                break;
            case '(':
            {
                ++cp;
                name.m_isFptr = 1;
                break;
            }
            case '*':
            {
                ++cp;
                name.m_ptrCount++;
                break;
            }
            case '[':
            {
                while ((*++cp) != ']')
                    name.m_slots[name.m_numSlots] = (name.m_slots[name.m_numSlots] * 10) + ((*cp) - '0');
                name.m_arraySize *= name.m_slots[name.m_numSlots++];
            }
            break;
            }
        }
        ++cp;
        m_name[m_nameNr++] = name;
        m_base.push_back(bn.hash());
        ++i;
    }


    opad = (FBTintPtr)cp;
    opad = ((opad + 3) & ~3) - opad;
    while (opad--)
        cp++;


    if (!ftCharNEq(cp, ftIdNames::ftTYPE, 4))
    {
        printf("Bin table is missing the type id!\n");
        return false;
    }

    cp += 4;

    ip = (FBTuint32*)cp;
    nl = *ip++;
    cp = (char*)ip;

    if (swap)
        nl = ftSwap32(nl);

    if (nl > FT_MAX_TABLE)
    {
        printf("Max name table size exceeded!\n");
        return false;
    }
    else
    {
        m_type = (Types)::malloc((nl * sizeof(ftType) + 1));
        m_tlen = (TypeL)::malloc((nl * sizeof(FBTtype) + 1));
    }

    i = 0;
    while (i < nl)
    {
        ftType typeData    = {cp, ftCharHashKey(cp).hash(), SK_NPOS32};
        m_type[m_typeNr++] = typeData;
        while (*cp)
            ++cp;
        ++cp;
        ++i;
    }

    opad = (FBTintPtr)cp;
    opad = ((opad + 3) & ~3) - opad;
    while (opad--)
        cp++;
    if (!ftCharNEq(cp, ftIdNames::ftTLEN, 4))
    {
        printf("Bin table is missing the tlen id!\n");
        return false;
    }

    cp += 4;
    tp = (FBTtype*)cp;

    i = 0;
    while (i < m_typeNr)
    {
        m_tlen[i] = *tp++;
        if (swap)
            m_tlen[i] = ftSwap16(m_tlen[i]);
        ++i;
    }

    if (m_typeNr & 1)
        ++tp;
    cp = (char*)tp;

    if (!ftCharNEq(cp, ftIdNames::ftSTRC, 4))
    {
        printf("Bin table is missing the tlen id!\n");
        return false;
    }

    cp += 4;

    ip = (FBTuint32*)cp;
    nl = *ip++;
    tp = (FBTtype*)ip;

    if (swap)
        nl = ftSwap32(nl);

    if (nl > FT_MAX_TABLE)
    {
        printf("Max name table size exceeded!\n");
        return false;
    }
    else
        m_strc = (Strcs)::malloc(nl * FT_MAX_MEMBERS * sizeof(FBTtype) + 1);


    m_typeFinder.reserve(m_typeNr);

    i = 0;
    while (i < nl)
    {
        m_strc[m_strcNr++] = tp;
        if (swap)
        {
            tp[0] = ftSwap16(tp[0]);
            tp[1] = ftSwap16(tp[1]);

            m_type[tp[0]].m_strcId = m_strcNr - 1;
            m_typeFinder.insert(m_type[tp[0]].m_name, m_type[tp[0]]);

            k = tp[1];
            SK_ASSERT(k < FT_MAX_MEMBERS);

            j = 0;
            tp += 2;
            while (j < k)
            {
                tp[0] = ftSwap16(tp[0]);
                tp[1] = ftSwap16(tp[1]);

                ++j;
                tp += 2;
            }
        }
        else
        {
            SK_ASSERT(tp[1] < FT_MAX_MEMBERS);
            m_type[tp[0]].m_strcId = m_strcNr - 1;
            m_typeFinder.insert(m_type[tp[0]].m_name, m_type[tp[0]]);

            tp += (2 * tp[1]) + 2;
        }
        ++i;
    }

    if (m_strcNr == 0)
    {
        ::free(m_name);
        ::free(m_type);
        ::free(m_tlen);
        ::free(m_strc);

        m_name = 0;
        m_type = 0;
        m_tlen = 0;
        m_strc = 0;

        return false;
    }

    compile();
    return true;
}


void ftBinTables::compile(FBTtype i, FBTtype nr, ftStruct* off, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys)
{
    FBTuint32 e, l, a, oof, ol;
    FBTuint16 f = m_strc[0][0];

    if (i > m_strcNr)
    {
        printf("Missing recursive type\n");
        return;
    }


    for (a = 0; a < nr; ++a)
    {
        // Only calculate offsets on recursive structures
        // This saves unneeded buffers
        FBTtype* strc = m_strc[i];

        oof = cof;
        ol  = m_tlen[strc[0]];

        l = strc[1];
        strc += 2;

        for (e = 0; e < l; e++, strc += 2)
        {
            if (strc[0] >= f && m_name[strc[1]].m_ptrCount == 0)
            {
                ftKey64 k = {m_type[strc[0]].m_typeId, m_name[strc[1]].m_nameId};
                keys.push_back(k);

                compile(m_type[strc[0]].m_strcId, m_name[strc[1]].m_arraySize, off, cof, depth + 1, keys);

                keys.pop_back();
            }
            else
                putMember(strc, off, a, cof, depth, keys);
        }

        if ((cof - oof) != ol)
            printf("Build ==> invalid offset (%i)(%i:%i)\n", a, (cof - oof), ol);
    }
}

void ftBinTables::compile(void)
{
    m_offs.reserve(FT_MAX_TABLE);

    if (!m_strc || m_strcNr <= 0)
    {
        printf("Build ==> No structures.");
        return;
    }

    FBTuint32 i, cof = 0, depth;
    FBTuint16 f = m_strc[0][0], e, memberCount;

    ftStruct::Keys emptyKeys;
    for (i = 0; i < m_strcNr; i++)
    {
        FBTtype* strc = m_strc[i];

        FBTtype strcType = strc[0];

        depth             = 0;
        cof               = 0;
        ftStruct* off     = new ftStruct;
        off->m_key.k16[0] = strcType;
        off->m_key.k16[1] = 0;
        off->m_val.k32[0] = m_type[strcType].m_typeId;
        off->m_val.k32[1] = 0;  // no name
        off->m_nr         = 0;
        off->m_dp         = 0;
        off->m_off        = cof;
        off->m_len        = m_tlen[strcType];
        off->m_strcId     = i;
        off->m_link       = 0;
        off->m_flag       = ftStruct::CAN_LINK;

        m_offs.push_back(off);

        memberCount = strc[1];

        strc += 2;
        off->m_members.reserve(FT_MAX_MEMBERS);

        for (e = 0; e < memberCount; ++e, strc += 2)
        {
            if (strc[0] >= f && m_name[strc[1]].m_ptrCount == 0)
            {
                ftStruct::Keys keys;
                ftKey64        k = {m_type[strc[0]].m_typeId, m_name[strc[1]].m_nameId};
                keys.push_back(k);

                compile(m_type[strc[0]].m_strcId, m_name[strc[1]].m_arraySize, off, cof, depth + 1, keys);
            }
            else
                putMember(strc, off, 0, cof, 0, emptyKeys);
        }

        if (cof != off->m_len)
        {
            off->m_flag |= ftStruct::MISALIGNED;
            printf("Build ==> invalid offset %s:%i:%i:%i\n", m_type[off->m_key.k16[0]].m_name, i, cof, off->m_len);
        }
    }
}

void ftBinTables::putMember(FBTtype* cp, ftStruct* off, FBTtype nr, FBTuint32& cof, FBTuint32 depth, ftStruct::Keys& keys)
{
    ftStruct nof;
    nof.m_key.k16[0] = cp[0];
    nof.m_key.k16[1] = cp[1];
    nof.m_val.k32[0] = m_type[cp[0]].m_typeId;
    nof.m_val.k32[1] = m_base[cp[1]];
    nof.m_off        = cof;
    nof.m_strcId     = off->m_strcId;
    nof.m_nr         = nr;
    nof.m_dp         = depth;
    nof.m_link       = 0;
    nof.m_flag       = ftStruct::CAN_LINK;
    nof.m_len        = (m_name[cp[1]].m_ptrCount ? m_ptrLength : m_tlen[cp[0]]) * m_name[cp[1]].m_arraySize;
    nof.m_keyChain   = keys;
    off->m_members.push_back(nof);
    cof += nof.m_len;
}


FBTtype ftBinTables::findTypeId(const ftCharHashKey& cp)
{
    FBTsizeType pos = m_typeFinder.find(cp);
    if (pos != m_typeFinder.npos)
        return m_typeFinder.at(pos).m_strcId;

    return SK_NPOS16;
}

const char* ftBinTables::getStructType(const ftStruct* strc)
{
    FBTuint32 k = strc ? strc->m_key.k16[0] : (FBTuint32)-1;
    return (k >= m_typeNr) ? "" : m_type[k].m_name;
}

const char* ftBinTables::getStructName(const ftStruct* strc)
{
    FBTuint32 k = strc ? strc->m_key.k16[1] : (FBTuint32)-1;
    return (k >= m_nameNr) ? "" : m_name[k].m_name;
}

const char* ftBinTables::getOwnerStructName(const ftStruct* strc)
{
    FBTuint32 k = strc ? strc->m_strcId : (FBTuint32)-1;
    return (k >= m_strcNr || *m_strc[k] >= m_typeNr) ? "" : m_type[*m_strc[k]].m_name;
}

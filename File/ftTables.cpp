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
#include "ftHashTypes.h"
#include "ftLogger.h"
#include "ftMember.h"
#include "ftPlatformHeaders.h"



const ftName ftBinTables::INVALID_NAME = {
    nullptr,  // m_name
    SK_NPOS,  // m_hashedName
    0,        // m_ptrCount
    0,        // m_numDimensions
    0,        // m_isFunctionPointer;
    0,        // m_arraySize
    {0, 0}    // m_array
};

const ftType ftBinTables::INVALID_TYPE = {
    nullptr,  // m_name
    SK_NPOS,  // m_typeId
    0,        // m_strcId
};



ftBinTables::ftBinTables() :
    m_name(0),
    m_type(0),
    m_tlen(0),
    m_strc(0),
    m_nameNr(0),
    m_typeNr(0),
    m_strcNr(0),
    m_ptrLength(FT_VOIDP),
    m_block(0),
    m_blockLen(0),
    m_firstStruct(0)
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
    m_block(ptr),
    m_blockLen(len),
    m_firstStruct(0)
{
}



ftBinTables::~ftBinTables()
{
    OffsM::Iterator it = m_structures.iterator();
    while (it.hasMoreElements())
        delete it.getNext();

    ::free(m_name);
    ::free(m_type);
    ::free(m_tlen);
    ::free(m_strc);
}


ftBinTables::OffsM::PointerType ftBinTables::getOffsetPtr()
{
    return m_structures.ptr();
}


ftBinTables::OffsM::SizeType ftBinTables::getOffsetCount()
{
    return m_structures.size();
}

bool ftBinTables::isPointer(const FBTuint16& name) const
{
    return getNameAt(name).m_ptrCount > 0;
}


ftCharHashKey ftBinTables::getStructHashByType(const FBTuint16& type)
{
    if (type < m_typeNr)
        return ftCharHashKey(m_type[type].m_name);
    return ftCharHashKey();
}


ftStruct* ftBinTables::findStructByName(const ftCharHashKey& kvp)
{
    FBTuint32 i;
    i = findTypeId(kvp);
    if (i != SK_NPOS32)
        return m_structures.at(i);
    return nullptr;
}


bool ftBinTables::read(bool swap)
{
    if (m_block && m_blockLen > 0)
        return read(m_block, m_blockLen, swap);
    return false;
}


bool ftBinTables::read(const void* ptr, const FBTsize& len, bool swap)
{
    FBTuint32 *ip = 0, i, j, k, nl;
    FBTtype*   tp = 0;

    char* cp = (char*)ptr;
    if (!ftCharNEq(cp, ftIdNames::FT_SDNA, 4))
    {
        ftLogger::logF("Table is missing the SDNA code.");
        return false;
    }

    cp += 4;
    if (!ftCharNEq(cp, ftIdNames::FT_NAME, 4))
    {
        ftLogger::logF("Table is missing the NAME code.");
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
        ftLogger::logF("Table max names exceeded(%d)", FT_MAX_TABLE);
        return false;
    }
    else
        m_name = (Names)::malloc((nl * sizeof(ftName)) + 1);



    i = 0;
    while (i < nl && i < FT_MAX_TABLE)
    {
        ftName name = {cp, ftCharHashKey(cp).hash(), 0, 0, 0, 1};

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
                name.m_isFunctionPointer = 1;
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
                    name.m_dimensions[name.m_numDimensions] = (name.m_dimensions[name.m_numDimensions] * 10) + ((*cp) - '0');
                name.m_arraySize *= name.m_dimensions[name.m_numDimensions++];
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


    if (!ftCharNEq(cp, ftIdNames::FT_TYPE, 4))
    {
        ftLogger::logF("Table is missing the TYPE code.");
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
        ftLogger::logF("Table max names exceeded(%d)", FT_MAX_TABLE);
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
    if (!ftCharNEq(cp, ftIdNames::FT_TLEN, 4))
    {
        ftLogger::logF("Table is missing the TLEN code.");
        return false;
    }

    cp += 4;
    tp = (FBTtype*)cp;

    i = 0;
    while (i < m_typeNr && i < SK_NPOS16)
    {
        FBTtype& type = m_tlen[i];
        type          = (*tp++);

        if (swap)
            m_tlen[i] = ftSwap16(m_tlen[i]);
        ++i;
    }

    if (m_typeNr & 1)
        ++tp;

    cp = (char*)tp;
    if (!ftCharNEq(cp, ftIdNames::FT_STRC, 4))
    {
        ftLogger::logF("Table is missing the STRC code.");
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
        ftLogger::logF("Max name table size exceeded(%d).", FT_MAX_TABLE);
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
            if (k < FT_MAX_MEMBERS)
            {
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
                ftLogger::logF("Max members exceeded(%d).", FT_MAX_MEMBERS);
        }
        else
        {
            if (tp[1] < FT_MAX_MEMBERS)
            {
                m_type[tp[0]].m_strcId = m_strcNr - 1;
                m_typeFinder.insert(m_type[tp[0]].m_name, m_type[tp[0]]);

                tp += (2 * tp[1]) + 2;
            }
            else
                ftLogger::logF("Max members exceeded(%d).", FT_MAX_MEMBERS);
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


void ftBinTables::compile(FBTtype    i,
                          FBTtype    nr,
                          ftStruct*  off,
                          FBTuint32& cof,
                          FBTuint32  depth)
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
        FBTtype* strc = m_strc[i];

        oof = cof;
        ol  = m_tlen[strc[0]];

        l = strc[1];
        strc += 2;

        for (e = 0; e < l; e++, strc += 2)
        {
            if (strc[0] >= f && m_name[strc[1]].m_ptrCount == 0)
            {
                compile(m_type[strc[0]].m_strcId,
                        m_name[strc[1]].m_arraySize,
                        off,
                        cof,
                        depth + 1);
            }
            else
                putMember(strc, off, a, cof, depth);
        }

        if ((cof - oof) != ol)
            printf("Build ==> invalid offset (%i)(%i:%i)\n", a, (cof - oof), ol);
    }
}

void ftBinTables::compile(void)
{
    m_structures.reserve(FT_MAX_TABLE);

    if (!m_strc || m_strcNr <= 0)
    {
        printf("No structures to compile.");
        return;
    }

    FBTuint32 i, cof = 0, depth;
    FBTuint16 e, memberCount;

    m_firstStruct = m_strc[0][0];

    for (i = 0; i < m_strcNr; i++)
    {
        FBTtype* strc = m_strc[i];
        FBTtype  strcType = strc[0];

        depth = 0;
        cof   = 0;

        ftStruct* nstrc;
        nstrc = new ftStruct(this);
        nstrc->m_type       = strcType;
        nstrc->m_hashedType = m_type[strcType].m_typeId;
        nstrc->m_strcId     = i;
        nstrc->m_sizeInBytes = m_tlen[strcType];
        nstrc->m_link        = 0;
        nstrc->m_flag        = ftStruct::CAN_LINK;
        m_structures.push_back(nstrc);

        memberCount = strc[1];

        strc += 2;
        nstrc->m_members.reserve(FT_MEMBERS_RESERVE);

        for (e = 0; e < memberCount; ++e, strc += 2)
        {
            const short& type = strc[0];
            const short& name = strc[1];

            if (type >= m_firstStruct && m_name[name].m_ptrCount == 0)
            {
                compile(m_type[type].m_strcId,
                        m_name[name].m_arraySize,
                        nstrc,
                        cof,
                        depth + 1);
            }
            else
                putMember(strc, nstrc, 0, cof, 0);
        }

        if (cof != nstrc->m_sizeInBytes)
        {
            nstrc->m_flag |= ftStruct::MISALIGNED;
            ftLogger::logF("Misaligned struct %s:%i:%i:%i\n",
                           m_type[nstrc->m_type].m_name,
                           i,
                           cof,
                           nstrc->m_sizeInBytes);
        }
    }
}


void ftBinTables::putMember(FBTtype*   cp,
                            ftStruct*  parent,
                            FBTtype    nr,
                            FBTuint32& cof,
                            FBTuint32  depth)
{
    const FBTuint16& type = cp[0];
    const FBTuint16& name = cp[1];

    if (type < 0 || type >= m_typeNr)
    {
        ftLogger::logF("Invalid type index");
        return;
    }

    if (name < 0 || name >= m_nameNr)
    {
        ftLogger::logF("Invalid name index");
        return;
    }


    ftMember* member = parent->createMember();
    member->setTypeIndex(type);
    member->setNameIndex(name);

    member->m_offset         = cof;
    member->m_location       = nr;
    member->m_recursiveDepth = depth;
    member->m_link           = nullptr;

    if (m_name[name].m_ptrCount > 0)
        member->m_sizeInBytes = m_ptrLength * m_name[name].m_arraySize;
    else
        member->m_sizeInBytes = m_tlen[type] * m_name[name].m_arraySize;

    cof += member->m_sizeInBytes;
}


ftStruct* ftBinTables::findStructByType(const FBTuint16& type)
{
    if (type < m_structures.size())
        return m_structures.at(type);
    return nullptr;
}


bool ftBinTables::isLinkedToMemory(const FBTuint16& type)
{
    if (type < m_structures.size())
        return m_structures.at(type)->m_link != 0;
    return false;
}



FBTuint32 ftBinTables::findTypeId(const ftCharHashKey& cp)
{
    FBTsize pos = m_typeFinder.find(cp);
    if (pos != m_typeFinder.npos)
        return m_typeFinder.at(pos).m_strcId;
    return SK_NPOS32;
}

FBThash ftBinTables::getTypeHash(const FBTuint16& type) const
{
    if (type < m_typeNr)
        return m_type[type].m_typeId;
    return SK_NPOS;
}

const ftName& ftBinTables::getStructNameByIdx(const FBTuint16& idx) const
{
    if (idx < m_nameNr)
        return m_name[idx];
    return INVALID_NAME;
}


ftFixedString<4> ftByteToString(FBTuint32 i)
{
    union {
        char      ids[4];
        FBTuint32 idi;
    } IDU;
    IDU.idi = i;

    ftFixedString<4> cp;
    cp.push_back(IDU.ids[0]);
    cp.push_back(IDU.ids[1]);
    cp.push_back(IDU.ids[2]);
    cp.push_back(IDU.ids[3]);
    return cp;
}

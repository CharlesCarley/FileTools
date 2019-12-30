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
#include "ftEndianUtils.h"
#include "ftHashTypes.h"
#include "ftLogger.h"
#include "ftMember.h"
#include "ftPlatformHeaders.h"
#include "ftStreams.h"

using namespace ftEndianUtils;
using namespace ftFlags;


const ftName ftTables::INVALID_NAME = {
    nullptr,  // m_names
    SK_NPOS,  // m_hash
    0,        // m_ptrCount
    0,        // m_numDimensions
    0,        // m_isFunctionPointer;
    0,        // m_arraySize
    {0, 0}    // m_array
};

const ftType ftTables::INVALID_TYPE = {
    nullptr,  // m_names
    SK_NPOS,  // m_hash
    0,        // m_strcId
};



ftTables::ftTables(int pointerLength) :
    m_names(0),
    m_types(0),
    m_tlens(0),
    m_strcs(0),
    m_nameCount(0),
    m_typeCount(0),
    m_strcCount(0),
    m_ptrLength(0),
    m_firstStruct(0),
    m_typeFinder()
{
    // This needs to be calculated from the table.
    m_ptrLength = pointerLength;
}


ftTables::~ftTables()
{
    clearTables();
}


bool ftTables::isPointer(const FBTuint16& name) const
{
    return getNameAt(name).m_ptrCount > 0;
}

ftCharHashKey ftTables::getStructHashByType(const FBTuint16& type)
{
    if (type < m_typeCount)
        return ftCharHashKey(m_types[type].m_name);
    return ftCharHashKey();
}


ftStruct* ftTables::findStructByName(const ftCharHashKey& kvp)
{
    FBTuint32 i;
    i = findTypeId(kvp);
    if (i != SK_NPOS32)
        return m_structures.at(i);

    return nullptr;
}


void ftTables::clearTables(void)
{
    if (!m_structures.empty())
    {
        Structures::Iterator it = m_structures.iterator();
        while (it.hasMoreElements())
            delete it.getNext();
    }

    m_typeFinder.clear();
    m_hashedNames.clear();

    if (m_names)
    {
        ::free(m_names);
        m_names = 0;
    }

    if (m_types)
    {
        ::free(m_types);
        m_types = 0;
    }

    if (m_tlens)
    {
        ::free(m_tlens);
        m_tlens = 0;
    }
    if (m_strcs)
    {
        ::free(m_strcs);
        m_strcs = 0;
    }
    m_nameCount = 0;
    m_typeCount = 0;
    m_strcCount = 0;
}


int ftTables::allocateTable(void**  destination,
                            FBTsize numberOfEntries,
                            FBTsize sizeOfEntry,
                            int     fileFlags)
{
    if (!destination)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Invalid storage pointer.");
        return RS_LIMIT_REACHED;
    }

    if (numberOfEntries > FT_MAX_TABLE || numberOfEntries <= 0)
    {
        if (fileFlags != LF_NONE)
        {
            ftLogger::logF(
                "Invalid table entry count(%d): "
                "valid count should be [1, %d]",
                numberOfEntries,
                FT_MAX_TABLE);
        }
        return RS_LIMIT_REACHED;
    }
    else
    {
        FBTsize allocLen = numberOfEntries * sizeOfEntry;
        if (allocLen > 0 && allocLen < SK_NPOS32)
        {
            *destination = (Names)::malloc(allocLen);
            if (!(*destination))
            {
                if (fileFlags != LF_NONE)
                    ftLogger::logF("Failed to allocate table.");
                return RS_BAD_ALLOC;
            }
            else
            {
                // This doesn't explicitly need to be zeroed.
                // The memory will be initialized after this
                // call. This should only used be to
                // weed out any possible bugs.
                if (fileFlags & LF_DO_CHECKS)
                    memset(*destination, 0, allocLen);
            }
        }
        else
        {
            if (fileFlags != LF_NONE)
            {
                ftLogger::logF(
                    "The sizeOfEntry(%d) parameter caused the "
                    "allocation limit to be exceeded (%d)",
                    sizeOfEntry,
                    allocLen);
            }
            return RS_BAD_ALLOC;
        }
    }
    return FS_OK;
}


int ftTables::read(const void*    tableSource,
                   const FBTsize& tableLength,
                   int            headerFlags,
                   int            fileFlags)
{
    int status = FS_OK;
    if (!tableSource)
        return RS_INVALID_PTR;

    ftMemoryStream stream;
    stream.open((FBTbyte*)tableSource, tableLength, 0, true);

    // FIXME: there should be no guarantee on the order that the NAME codes come in. 

    status = readTableHeader(stream, ftIdNames::FT_SDNA, fileFlags);
    if (status != FS_OK)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Failed to read the table header.");
        return status;
    }

    status = readNameTable(stream, headerFlags, fileFlags);
    if (status != FS_OK)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Failed to read the name table.");
        clearTables();
        return status;
    }

    status = readTypeTable(stream, headerFlags, fileFlags);
    if (status != FS_OK)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Failed to read the type table.");
        clearTables();
        return status;
    }

    status = readSizeTable(stream, headerFlags, fileFlags);
    if (status != FS_OK)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Failed to read the size table.");
        clearTables();
        return status;
    }

    status = readStructureTable(stream, headerFlags, fileFlags);
    if (status != FS_OK)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Failed to read the structure table.");
        clearTables();
        return status;
    }


    status = compile(fileFlags);
    if (status == FS_OK)
    {
        if (fileFlags & LF_DIAGNOSTICS)
        {
            if (fileFlags & LF_DUMP_NAME_TABLE)
            {
                FBTuint32 i;
                for (i = 0; i < m_nameCount; ++i)
                    ftLogger::log(m_names[i]);
            }

            if (fileFlags & LF_DUMP_TYPE_TABLE)
            {
                FBTuint32 i;
                for (i = 0; i < m_typeCount; ++i)
                    ftLogger::log(m_types[i]);
            }

            if (fileFlags & LF_DUMP_SIZE_TABLE)
            {
                FBTuint32 i;
                for (i = 0; i < m_typeCount; ++i)
                    ftLogger::log(m_types[i], m_tlens[i]);
            }
        }
    }
    return status;
}


int ftTables::readTableHeader(ftMemoryStream& stream, const char* headerName, int fileFlags)
{
    char cp[5] = {};
    stream.read(cp, 4);

    if (!ftCharNEq(cp, headerName, 4))
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Table is missing the %s code.", headerName);
        return RS_INVALID_CODE;
    }
    return FS_OK;
}


int ftTables::readNameTable(ftMemoryStream& stream, int headerFlags, int fileFlags)
{
    ftName    name;
    FBTuint32 status, count = 0;

    status = readTableHeader(stream, ftIdNames::FT_NAME, fileFlags);
    if (status == FS_OK)
    {
        stream.readInt32(count);
        if (headerFlags & FH_ENDIAN_SWAP)
            count = swap32(count);


        status = allocateTable((void**)&m_names, count, sizeof(ftName), fileFlags);
        if (status == FS_OK)
        {
            m_hashedNames.reserve(count);


            for (m_nameCount = 0; m_nameCount < count; ++m_nameCount)
            {
                convertName(name, stream.addressAtPosition());

                m_names[m_nameCount] = name;
                m_hashedNames.push_back(name.m_hash);

                stream.seekString();
            }

            count = (SKuint32)stream.getVaryingInt();
            count = ((count + 3) & ~3) - count;
            if (count)
                stream.seek(count, SEEK_CUR);
        }
    }
    return status;
}

int ftTables::readTypeTable(ftMemoryStream& stream, int headerFlags, int fileFlags)
{
    FBTuint32 status, count = 0;
    status = readTableHeader(stream, ftIdNames::FT_TYPE, fileFlags);
    if (status == FS_OK)
    {
        stream.readInt32(count);
        if (headerFlags & FH_ENDIAN_SWAP)
            count = swap32(count);


        status = allocateTable((void**)&m_types, count, sizeof(ftType), fileFlags);
        if (status == FS_OK)
        {
            FBTbyte* cp;
            for (m_typeCount = 0; m_typeCount < count; ++m_typeCount)
            {
                cp = stream.addressAtPosition();

                m_types[m_typeCount].m_name   = cp;
                m_types[m_typeCount].m_hash   = skHash(cp);
                m_types[m_typeCount].m_strcId = SK_NPOS32;

                stream.seekString();
            }

            count = (SKuint32)stream.getVaryingInt();
            count = ((count + 3) & ~3) - count;
            if (count)
                stream.seek(count, SEEK_CUR);
        }
    }
    return status;
}



int ftTables::readSizeTable(ftMemoryStream& stream, int headerFlags, int fileFlags)
{
    FBTuint32 status;
    status = readTableHeader(stream, ftIdNames::FT_TLEN, fileFlags);
    if (status == FS_OK)
    {
        status = allocateTable((void**)&m_tlens, m_typeCount, sizeof(FBTtype), fileFlags);
        if (status == FS_OK)
        {
            FBTuint16 type;
            FBTuint32 i;

            for (i = 0; i < m_typeCount; ++i)
            {
                stream.readInt16(type);
                if (headerFlags & FH_ENDIAN_SWAP)
                    type = swap16(type);
                m_tlens[i] = type;
            }

            if (m_typeCount & 1)
                stream.seek(sizeof(FBTuint16), SEEK_CUR);
        }
    }
    return status;
}


int ftTables::readStructureTable(ftMemoryStream& stream, int headerFlags, int fileFlags)
{
    FBTuint32 status, count = 0;

    status = readTableHeader(stream, ftIdNames::FT_STRC, fileFlags);
    if (status == FS_OK)
    {
        stream.readInt32(count);
        if (headerFlags & FH_ENDIAN_SWAP)
            count = swap32(count);

        status = allocateTable((void**)&m_strcs,
                               count,
                               sizeof(FBTtype) * FT_MAX_MEMBERS,
                               fileFlags);

        if (status == FS_OK)
        {
            m_typeFinder.reserve(m_typeCount);
            FBTuint16* tp = (FBTuint16*)stream.addressAtPosition();

            m_strcCount = 0;
            while (m_strcCount < count && status == FS_OK)
            {
                status = buildStruct(tp, m_strcCount, headerFlags, fileFlags);
                m_strcCount++;
            }
        }
    }
    return status;
}



void ftTables::convertName(ftName& dest, char* cp)
{
    dest = INVALID_NAME;

    // All of the names are a reference to the block of data
    // that houses the tables. This is storing the address
    // of the current name's location in the buffer
    // of null terminated strings.
    dest.m_name      = cp;
    dest.m_hash      = skHash(dest.m_name);
    dest.m_arraySize = 1;

    int i = 0;
    while (*cp)
    {
        int ival = 0;
        switch (*cp)
        {
        default:
            ++cp;
            break;
        case ')':
        case ']':
            ++cp;
            break;
        case '(':
            ++cp;
            dest.m_isFunctionPointer = 1;
            break;
        case '*':
            ++cp;
            dest.m_ptrCount++;
            break;
        case '[':
            while ((*++cp) != ']')
            {
                if ((*cp) >= '0' && (*cp) <= '9')
                    ival = (ival * 10) + ((*cp) - '0');
            }
            dest.m_dimensions[i] = ival;
            dest.m_arraySize *= dest.m_dimensions[i++];
            break;
        }
    }
    dest.m_numDimensions = i;
}



int ftTables::buildStruct(FBTuint16*& strc, FBTuint16 current, int headerFlags, int fileFlags)
{
    int j, k, status = FS_OK;

    if (!m_strcs || !strc)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("Invalid structure table");
        return RS_BAD_ALLOC;
    }

    m_strcs[current] = strc;

    if (headerFlags & FH_ENDIAN_SWAP)
    {
        strc[0] = swap16(strc[0]);
        strc[1] = swap16(strc[1]);

        status = isValidTypeName(strc[0], strc[1], fileFlags);
        if (status == FS_OK)
        {
            // Reassign the current id to the type
            // so it can be referenced later.
            m_types[strc[0]].m_strcId = current;

            if (!m_typeFinder.insert(m_types[strc[0]].m_name, m_types[strc[0]]))
            {
                if (fileFlags != LF_NONE)
                    ftLogger::logF("Failed to insert the type name for structure(%d).", strc[0]);
                status = RS_LIMIT_REACHED;
            }
            else
            {
                k = strc[1];
                if (k < FT_MAX_MEMBERS)
                {
                    j = 0;
                    strc += 2;
                    while (j < k && status == FS_OK)
                    {
                        strc[0] = swap16(strc[0]);
                        strc[1] = swap16(strc[1]);

                        status = isValidTypeName(strc[0], strc[1], fileFlags);
                        if (status == FS_OK)
                        {
                            ++j;
                            strc += 2;
                        }
                    }
                }
                else
                {
                    if (fileFlags != LF_NONE)
                        ftLogger::logF("Max members exceeded(%d).", FT_MAX_MEMBERS);
                    status = RS_LIMIT_REACHED;
                }
            }
        }
    }
    else
    {
        if (strc[1] < FT_MAX_MEMBERS)
        {
            // Reassign the current id to the type
            // so it can be referenced later.
            m_types[strc[0]].m_strcId = current;

            if (!m_typeFinder.insert(m_types[strc[0]].m_name, m_types[strc[0]]))
            {
                if (fileFlags != LF_NONE)
                    ftLogger::logF("Failed to insert the type name for structure(%d) '%s'.",
                                   strc[0],
                                   m_types[strc[0]].m_name);
                status = RS_LIMIT_REACHED;
            }
            else if (fileFlags & LF_DO_CHECKS)
            {
                // Check all members for consistent values.

                k = strc[1];
                j = 0;
                strc += 2;
                while (j < k && status == FS_OK)
                {
                    status = isValidTypeName(strc[0], strc[1], fileFlags);
                    if (status == FS_OK)
                    {
                        ++j;
                        strc += 2;
                    }
                }
            }
            else
            {
                // skip past the members
                strc += (2 * strc[1]) + 2;
            }
        }
        else
        {
            if (fileFlags != LF_NONE)
                ftLogger::logF("Max members exceeded(%d).", FT_MAX_MEMBERS);
            status = RS_LIMIT_REACHED;
        }
    }
    return status;
}

int ftTables::isValidTypeName(const FBTuint16& type, const FBTuint16& name, int flags)
{
    if (type > m_typeCount)
    {
        if (flags != LF_NONE)
        {
            ftLogger::logF(
                "The parsed structure type (%d) exceeds the number of types(%d)",
                type,
                m_typeCount);
        }
        return RS_LIMIT_REACHED;
    }
    else if (name > m_nameCount)
    {
        if (flags != LF_NONE)
        {
            ftLogger::logF(
                "The parsed structure name (%d) exceeds the number of names(%d)",
                name,
                m_nameCount);
        }
        return RS_LIMIT_REACHED;
    }
    return FS_OK;
}


void ftTables::compile(FBTtype    i,
                       FBTtype    nr,
                       ftStruct*  off,
                       FBTuint32& cof,
                       FBTuint32  depth,
                       int&       status)
{
    FBTuint32 e, l, a, oof, ol;
    FBTuint16 f = m_strcs[0][0];

    if (i > m_strcCount)
    {
        printf("Missing recursive type\n");
        return;
    }

    for (a = 0; a < nr; ++a)
    {
        FBTtype* strc = m_strcs[i];
        oof           = cof;
        ol            = m_tlens[strc[0]];

        l = strc[1];
        strc += 2;

        for (e = 0; e < l; e++, strc += 2)
        {
            if (strc[0] >= f && m_names[strc[1]].m_ptrCount == 0)
            {
                compile(m_types[strc[0]].m_strcId,
                        m_names[strc[1]].m_arraySize,
                        off,
                        cof,
                        depth + 1,
                        status);
            }
            else
                putMember(strc, off, a, cof, depth, status);
        }

        if ((cof - oof) != ol)
            printf("Build ==> invalid offset (%i)(%i:%i)\n", a, (cof - oof), ol);
    }
}


bool ftTables::testDuplicateKeys()
{
    bool testResult = true;

    FBTuint32 i, j;
    for (i = 0; i < m_typeCount && testResult; ++i)
    {
        for (j = 0; j < m_typeCount && testResult; ++j)
            if (i != j)
                testResult = m_types[i].m_hash != m_types[j].m_hash;
    }

    if (testResult)
    {
        for (i = 0; i < m_nameCount && testResult; ++i)
        {
            for (j = 0; j < m_nameCount && testResult; ++j)
                if (i != j)
                    testResult = m_names[i].m_hash != m_names[j].m_hash;
        }
    }
    return testResult;
}



int ftTables::compile(int fileFlags)
{
    if (!m_strcs || m_strcCount <= 0)
    {
        if (fileFlags != LF_NONE)
            ftLogger::logF("No structures to compile.");
        return FS_TABLE_INIT_FAILED;
    }

    m_structures.reserve(m_strcCount);
    int status = FS_OK;

    FBTuint32 i, cof, depth;
    FBTuint16 e, memberCount;

    // Save the first structure type index
    // So the isBuiltin test can determine
    // from a type index whether or not
    // the supplied type represents a struct.
    // Types in the type table are stored with
    // atomic types first.
    m_firstStruct = m_strcs[0][0];


    for (i = 0; i < m_strcCount && status == FS_OK; i++)
    {
        FBTtype* strc = m_strcs[i];
        FBTtype  type = strc[0];

        if (type > m_typeCount)
        {
            status = RS_LIMIT_REACHED;
            if (fileFlags != LF_NONE)
            {
                ftLogger::logF(
                    "The parsed structure type (%d) exceeds the number of types(%d)",
                    type,
                    m_typeCount);
            }
        }
        else
        {
            if (m_types[type].m_hash == SK_NPOS)
            {
                status = RS_MIS_ALIGNED;
                if (fileFlags != LF_NONE)
                {
                    ftLogger::logF(
                        "The current structure (%d) is out of alignment with the type table.",
                        type);
                }
            }
            else
            {
                depth = 0;
                cof   = 0;

                ftStruct* nstrc;
                nstrc                = new ftStruct(this);
                nstrc->m_type        = type;
                nstrc->m_hashedType  = m_types[type].m_hash;
                nstrc->m_strcId      = i;
                nstrc->m_sizeInBytes = m_tlens[type];
                nstrc->m_link        = 0;
                nstrc->m_flag        = ftStruct::CAN_LINK;
                m_structures.push_back(nstrc);

                memberCount = strc[1];
                strc += 2;
                nstrc->m_members.reserve(memberCount);

                for (e = 0; e < memberCount && status == FS_OK;
                     ++e, strc += 2)
                {
                    const short& type = strc[0];
                    const short& name = strc[1];

                    if (type >= m_firstStruct && m_names[name].m_ptrCount == 0)
                    {
                        compile(m_types[type].m_strcId,
                                m_names[name].m_arraySize,
                                nstrc,
                                cof,
                                depth + 1,
                                status);

                        nstrc->m_flag |= ftStruct::HAS_DEPENDANT;
                    }
                    else
                        putMember(strc, nstrc, 0, cof, 0, status);
                }

                if (cof != nstrc->m_sizeInBytes)
                {
                    nstrc->m_flag |= ftStruct::MISALIGNED;

                    ftLogger::logF("Misaligned struct %s:%i:%i:%i\n",
                                   m_types[nstrc->m_type].m_name,
                                   i,
                                   cof,
                                   nstrc->m_sizeInBytes);
                    status = RS_MIS_ALIGNED;
                }
            }
        }
    }
    return status;
}


void ftTables::putMember(FBTtype*   cp,
                         ftStruct*  parent,
                         FBTtype    nr,
                         FBTuint32& cof,
                         FBTuint32  depth,
                         int&       status)
{
    const FBTuint16& type = cp[0];
    const FBTuint16& name = cp[1];

    if (type < 0 || type >= m_typeCount)
        status = RS_LIMIT_REACHED;
    else if (name < 0 || name >= m_nameCount)
        status = RS_LIMIT_REACHED;
    else
    {
        if (type >= m_firstStruct)
            parent->m_flag |= ftStruct::HAS_DEPENDANT;
 
        ftMember* member = parent->createMember();
        member->setTypeIndex(type);
        member->setNameIndex(name);

        member->m_offset         = cof;
        member->m_location       = nr;
        member->m_recursiveDepth = depth;
        member->m_link           = nullptr;

        if (m_names[name].m_ptrCount > 0)
            member->m_sizeInBytes = m_ptrLength * m_names[name].m_arraySize;
        else
            member->m_sizeInBytes = m_tlens[type] * m_names[name].m_arraySize;

        cof += member->m_sizeInBytes;
    }
}


ftStruct* ftTables::findStructByType(const FBTuint16& type)
{
    if (type < m_structures.size())
    {
        ftStruct* strc = m_structures.at(type);

        if (type != strc->m_strcId)
            ftLogger::logF("Type mismatch!");
        return strc;
    }
    return nullptr;
}


bool ftTables::isLinkedToMemory(const FBTuint16& type)
{
    if (type < m_structures.size())
        return m_structures.at(type)->m_link != 0;
    return false;
}



FBTuint32 ftTables::findTypeId(const ftCharHashKey& cp)
{
    FBTsize pos = m_typeFinder.find(cp);
    if (pos != m_typeFinder.npos)
        return m_typeFinder.at(pos).m_strcId;
    return SK_NPOS32;
}

FBThash ftTables::getTypeHash(const FBTuint16& type) const
{
    if (type < m_typeCount)
        return m_types[type].m_hash;
    return SK_NPOS;
}

FBTuint32 ftTables::findStructIdByType(const FBTuint16& type)
{
    if (type < m_typeCount)
        return m_types[type].m_strcId;
    return SK_NPOS32;
}



const ftName& ftTables::getStructNameByIdx(const FBTuint16& idx) const
{
    if (idx < m_nameCount)
        return m_names[idx];
    return INVALID_NAME;
}

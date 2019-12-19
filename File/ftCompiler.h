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
#ifndef _ftBuilder_h_
#define _ftBuilder_h_

#include "Utils/skArray.h"
#include "Utils/skFixedString.h"
#include "Utils/skMap.h"
#include "ftTables.h"
#include "ftTypes.h"



typedef skFixedString<272>       ftPath;
typedef skFixedString<FT_MAX_ID> ftId;
typedef int                      ftArraySlots[FT_ARR_DIM_MAX];
typedef void*                    ftParser;
typedef skArray<ftId>            ftStringPtrArray;
typedef skHashTable<ftId, ftId>  ftStringPtrTable;
typedef skArray<ftPath>          ftPathArray;

class ftBuildInfo;
class ftScanner;
class ftToken;



class ftVariable
{
public:
    ftVariable() :
        m_type(),
        m_name(),
        m_typeId(-1),
        m_hashedName(-1),
        m_ptrCount(0),
        m_numDimensions(0),
        m_isFunctionPointer(0),
        m_lstat(0),
        m_undefined(0),
        m_isDependentType(false),
        m_arraySize(1),
        m_path(),
        m_line(-1),
        m_arrays()
    {
    }

    ftId         m_type;
    ftId         m_name;
    int          m_typeId;
    int          m_hashedName;
    int          m_ptrCount;
    int          m_numDimensions;
    int          m_isFunctionPointer;
    int          m_lstat;
    int          m_undefined;
    bool         m_isDependentType;
    ftArraySlots m_arrays;
    FBTsize      m_arraySize;
    ftPath       m_path;
    FBTsize      m_line;
};

typedef skArray<ftVariable> ftVariables;



class ftCompileStruct
{
public:
    typedef skArray<ftCompileStruct> Array;

public:
    ftCompileStruct() :
        m_structId(SK_NPOS),
        m_line(-1),
        m_nrDependentTypes(0)
    {
    }

    FBTsize     m_structId;
    ftId        m_name;
    ftVariables m_data;
    FBTsize     m_nrDependentTypes;
    ftPath      m_path;
    FBTsize     m_line;
};


enum ftLinkerIssues
{
    LNK_OK              = 0,
    LNK_ASSERT          = (1 << 0),
    LNK_ALIGNEMENT2     = (1 << 1),
    LNK_ALIGNEMENT4     = (1 << 2),
    LNK_ALIGNEMENT8     = (1 << 3),
    LNK_ALIGNEMENTS     = (1 << 4),
    LNK_ALIGNEMENTP     = (1 << 5),
    LNK_UNKNOWN         = (1 << 6),
    LNK_UNDEFINED_TYPES = (1 << 7),
    LNK_DUPLICATE_TYPES = (1 << 8)
};


class ftCompiler
{
public:
    enum WriteMode
    {
        WRITE_ARRAY = 0,  // writes table as a c/c++ array (default)
        WRITE_STREAM,     // writes table to the specified stream (data only no text)
    };


public:
    ftCompiler();
    ~ftCompiler();

    int parseFile(const ftPath& id);
    int parseBuffer(const ftId& name, const char* ms, size_t len);
    int buildTypes(void);
    
    FBTuint32 getNumberOfBuiltinTypes(void);

    
    inline void setWriteMode(int mode)
    {
        m_writeMode = mode;
    }

    void writeFile(const ftId& id, class skStream* fp);
    void writeFile(const ftId& id, const ftPath& path);
    void writeStream(class skStream* fp);

    // ftBinTables* write(void);

private:
    int  parse(void);
    void parseClass(int& tok, ftToken& tp);


    void writeBinPtr(skStream* fp, void* ptr, int len);
    void writeCharPtr(skStream* fp, const ftStringPtrArray& ptrs);
    void writeValidationProgram(const ftPath& path);
    void makeName(ftVariable&, bool);

    char*                  m_buffer;
    FBTsize                m_pos;
    ftBuildInfo*           m_build;
    FBTsize                m_start;
    ftPathArray            m_includes;
    ftStringPtrArray       m_namespaces, m_skip;
    ftCompileStruct::Array m_builders;
    int                    m_curBuf, m_writeMode;
    ftScanner*             m_scanner;
};

#endif  //_ftBuilder_h_

/*
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

#include "ftTypes.h"
#include "ftTables.h"

typedef ftFixedString<272>          ftPath;
typedef ftFixedString<ftMaxID>      ftId;
typedef int                         ftArraySlots[FileTools_ARRAY_SLOTS];
typedef void*                       ftParser;
typedef ftArray<ftId>               ftStringPtrArray;
typedef ftArray<ftPath>             ftPathArray;





class ftVariable
{
public:

	ftVariable()
		:   m_type(), 
			m_name(), 
			m_typeId(-1),
		    m_nameId(-1), 
			m_ptrCount(0), 
			m_numSlots(0),
		    m_isFptr(0), 
			m_lstat(0), 
			m_undefined(0),
		    m_isDependentType(false), 
			m_arraySize(1),
			m_path(),
			m_line(-1)
	{
	}

    ftId            m_type;
	ftId            m_name;
	int             m_typeId;
	int             m_nameId;
	int             m_ptrCount;
	int             m_numSlots;
	int             m_isFptr;
	int             m_lstat;
	int             m_undefined;
	bool            m_isDependentType;
	ftArraySlots    m_arrays;
	FBTsize         m_arraySize;
	ftPath          m_path;
	FBTsize         m_line;
};

typedef ftArray<ftVariable> ftVariables;



class ftCompileStruct
{
public:
    typedef ftArray<ftCompileStruct> Array;

public:

    ftCompileStruct()
		:   m_structId(-1),
		    m_line(-1),
		    m_nrDependentTypes(0)
	{
	}

	FBTsize         m_structId;
	ftId            m_name;
	ftVariables     m_data;
	FBTsize         m_nrDependentTypes;

    ftPath          m_path;
	FBTsize         m_line;
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

	ftCompiler();
	~ftCompiler();

	int parseFile(const ftPath& id);
	int parseBuffer(const ftId& name, const char* ms);

	int buildTypes(void);

	void writeFile(const ftId& id, class ftStream* fp);
	void writeFile(const ftId& id, const ftPath& path);
	void writeStream(class ftStream* fp);

	ftBinTables* write(void);

private:

	int doParse(void);
	class ftBuildInfo* m_build;

	void writeBinPtr(ftStream* fp, void* ptr, int len);
	void writeCharPtr(ftStream* fp, const ftStringPtrArray& ptrs);

	void writeValidationProgram(const ftPath& path);


	void makeName(ftVariable&, bool);

	FBTsize             ft_start;
	ftPathArray         ft_includes;
	ftStringPtrArray    ft_namespaces, ft_skip;
    ftCompileStruct::Array ft_struct_builders;
	int m_curBuf, m_writeMode;
};

#endif//_ftBuilder_h_

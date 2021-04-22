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
#ifndef _ftCompiler_h_
#define _ftCompiler_h_

#include "ftBuildInfo.h"

class ftCompiler
{
public:
    ftCompiler();
    ~ftCompiler();

    int parse(const ftPath& id);
    int parse(const ftPath& id, const char* data, size_t len);

    int buildTypes(void);
    
    FBTuint32 getNumberOfBuiltinTypes(void) const;

    
    inline void setWriteMode(int mode)
    {
        m_writeMode = mode;
    }

    void writeFile(const ftId& id, class skStream* fp);
    void writeFile(const ftId& id, const ftPath& path);
    void writeStream(class skStream* fp);

    // ftTables* write(void);

private:
    int  parse(void);
    void parseClass(int& token, ftToken& tokenPtr);
    void parseIdentifier(int& token, ftToken& tokenPtr, ftBuildStruct& buildStruct);

    static void handleConstant(int&           token,
                               ftToken&       tokenPtr,
                               ftBuildMember& member);

    static void handleStatementClosure(int&           token,
                                       ftBuildStruct& buildStruct,
                                       ftBuildMember& member,
                                       bool           forceArray,
                                       bool           isId);

    void errorUnknown(int& token, ftToken& tokenPtr);


    void        writeBinPtr(skStream* fp, void* ptr, int len);
    void        writeCharPtr(skStream* fp, const ftStringPtrArray& pointers);
    void        writeValidationProgram(const ftPath& path);
    static void makeName(ftBuildMember&, bool);

    char*                  m_buffer;
    FBTsize                m_pos;
    ftBuildInfo*           m_build;
    FBTsize                m_start;
    ftPathArray            m_includes;
    ftStringPtrArray       m_namespaces, m_skip;
    ftBuildStruct::Array m_builders;
    int                    m_curBuf, m_writeMode;
    ftScanner*             m_scanner;
};

#endif  //_ftCompiler_h_

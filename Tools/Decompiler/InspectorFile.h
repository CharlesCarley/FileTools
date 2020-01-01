/*

    This file was automatically generated.
    https://github.com/CharlesCarley/FileTools

    By    : TableDecompiler.exe
    From  : API/INSP.insp(INSPECT_v100)
    On    : Wed Jan 01 03:05:55 PM

*/
#ifndef _InspectorFile_h_
#define _InspectorFile_h_

namespace InspectorFile
{
#pragma region Forward
    struct Link;
    struct fbtText;
    struct fbtFileList;
    struct fbtProjectFile;
#pragma endregion

// Pointers that have references to no known
// struct need to be declared as some type of handle.
// This should be a struct handle class so that it can be
// recompiled. struct XXX {int unused; }
#pragma region MissingStructures

#pragma endregion

// Independent structures:
// The member declarations only contain references to other
// structures via a pointer (or only atomic types); Therefore,
// declaration order does not matter as long as any pointer
// reference is forwardly declared.
#pragma region Independent
    struct Link
    {
        void *m_data;
    };

    struct fbtText
    {
        fbtText *m_next;
        fbtText *m_prev;
        char     m_name[32];
        char *   m_data;
        int      m_size;
        int      m_flag;
        void *   m_textFile;
    };

    struct fbtFileList
    {
        fbtText *m_first;
        fbtText *m_last;
    };

#pragma endregion

// Dependent structures:
// The member declarations have references to other
// structures without a pointer; Therefore, declaration order DOES matter.
// If a structure has a non pointer member structure, then that structure
// must be visible before defining the structure that uses it.
#pragma region Dependent
    struct fbtProjectFile
    {
        char *      m_windowLayout;
        int         m_version;
        int         m_layoutSize;
        int         m_flags;
        int         m_pad;
        fbtFileList m_projectFiles;
    };

#pragma endregion

}  // namespace InspectorFile
#endif  //_InspectorFile_h_

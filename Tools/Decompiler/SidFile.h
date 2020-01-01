/*

    This file was automatically generated.
    https://github.com/CharlesCarley/FileTools

    By    : TableDecompiler.exe
    From  : API/untitled.sid(SidFile-v100)
    On    : Wed Jan 01 03:02:09 PM

*/
#ifndef _SidFile_h_
#define _SidFile_h_

namespace SidFile
{
#pragma region Forward
    struct Object;
    struct Global;
    struct LinkNode;
    struct LinkedList;
    struct Camera;
    struct Scene;
    struct Vertex;
    struct IndexBuffer;
    struct Vertex3Bufferf;
    struct Model;
    struct Vector3;
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
    struct Global
    {
        int version;
        int test;
    };

    struct LinkNode
    {
        void *m_next;
    };

    struct LinkedList
    {
        LinkNode *m_head;
        LinkNode *m_tail;
    };

    struct Camera
    {
        char   m_name[64];
        double m_fov;
        double m_near;
        double m_far;
    };

    struct Scene
    {
        char     m_name[64];
        Camera * m_camera;
        void *   m_private;
        int      m_objectCount;
        int      m_pad;
        Object **m_objects;
    };

    struct Vertex
    {
        float x;
        float y;
        float z;
    };

    struct IndexBuffer
    {
        int  m_total;
        int  pad;
        int *indices;
    };

    struct Vertex3Bufferf
    {
        int     total;
        int     pad;
        Vertex *verticies;
    };

    struct Vector3
    {
        double x;
        double y;
        double z;
    };

#pragma endregion

// Dependent structures:
// The member declarations have references to other
// structures without a pointer; Therefore, declaration order DOES matter.
// If a structure has a non pointer member structure, then that structure
// must be visible before defining the structure that uses it.
#pragma region Dependent
    struct Model
    {
        char           m_name[64];
        Vertex3Bufferf m_verticies;
    };

    struct Object
    {
        char    m_name[64];
        Vector3 m_loc;
        Vector3 m_rot;
        Vector3 m_scl;
        int     m_type;
        int     m_pad;
        void *  m_data;
    };

#pragma endregion

}  // namespace SidFile
#endif  //_SidFile_h_

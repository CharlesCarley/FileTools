/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

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
#ifndef _ftChunk_h_
#define _ftChunk_h_

#include "ftTypes.h"

/// <summary>
/// Is a structure that contains the minimum amount of
/// information to jump around the file.
/// </summary>
/// <remarks>Used when scanning for a specific data block.</remarks>
struct ftChunkScan
{
    SKuint32 code;
    SKuint32 length;
};

/// <summary>
/// Structure to represent a chunk saved on a 32bit platform.
/// </summary>
struct ftChunk32
{
    SKuint32 code;
    SKuint32 length;
    SKuint32 address;
    SKuint32 structId;
    SKuint32 count;
};
SK_ASSERTCOMP(ChunkLen32, sizeof(ftChunk32) == 20);

/// <summary>
/// Structure to represent a chunk saved on a 64bit platform.
/// </summary>
struct ftChunk64
{
    SKuint32 code;
    SKuint32 length;
    SKuint64 address;
    SKuint32 structId;
    SKuint32 count;
};
SK_ASSERTCOMP(ChunkLen64, sizeof(ftChunk64) == 24);

/// <summary>
/// Structure to represent a chunk saved on a
/// 32 bit platform or a 64bit platform.
/// </summary>
struct ftChunk
{
    ftChunk() :
        code(0),
        length(0),
        address(0),
        structId(0),
        count(0)
    {
    }

    SKuint32 code;
    SKuint32 length;
    SKsize   address;
    SKuint32 structId;
    SKuint32 count;
};

#if SK_ARCH == SK_ARCH_32
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftChunk32) == sizeof(ftChunk));
#else
SK_ASSERTCOMP(ChunkLenNative, sizeof(ftChunk64) == sizeof(ftChunk));
#endif

/// <summary>
/// Structure to hold a chunk and it's subsequent data
/// </summary>
struct ftMemoryChunk
{
    enum Flag
    {
        BLK_MODIFIED = 1 << 0,
        BLK_LINKED   = 1 << 1,
    };

    ftMemoryChunk() :
        next(nullptr),
        prev(nullptr),
        fileBlock(nullptr),
        memoryBlock(nullptr),
        pointerBlock(nullptr),
        pointerBlockLen(0),
        flag(0),
        newTypeId(0),
        fileStruct(nullptr),
        memoryStruct(nullptr)
    {
    }

    ftMemoryChunk *next, *prev;


    ftChunk        chunk;

    /// <summary>
    /// Is the block of memory that was allocated and read
    /// from the file. It contains the data of the structure at
    /// the time of saving.
    /// </summary>
    void* fileBlock;

    /// <summary>
    /// Is the block of memory allocated for conversion.
    /// Its length is the size of the corresponding structure in its
    /// current state. If the file an memory structures are different,
    /// the data from fileBlock gets cast into memoryBlock one member at a time.
    /// </summary>
    void* memoryBlock;

    /// <summary>
    /// Is the storage location for pointers to pointers.
    /// When casting fileBlock into memoryBlock, if the current member is
    /// a pointer to a pointer, this block provides the storage location
    /// for each of the pointers. The address of pointerBlock is assigned to
    /// memoryBlock at the offset for the pointer to pointer member.
    /// </summary>
    void* pointerBlock;

    /// <summary>
    /// Contains the total length of allocated memory for pointerBlock
    /// </summary>
    SKuint32 pointerBlockLen;

    SKuint8 flag;
    FTtype  newTypeId;

    ftStruct* fileStruct;
    ftStruct* memoryStruct;
};


/// <summary>
/// Utility class to read write and scan chunks.
/// </summary>
struct ftChunkUtils
{
    enum Size
    {
        BlockSize = sizeof(ftChunk),
        Block32   = sizeof(ftChunk32),
        Block64   = sizeof(ftChunk64),
        BlockScan = sizeof(ftChunkScan),
    };

    static SKsize read(ftChunk* dest, skStream* stream, int flags);
    static SKsize write(ftChunk* src, skStream* stream);
    static SKsize scan(ftChunkScan* dest, skStream* stream, int flags);

    static const ftChunk BlankChunk;
};

#endif  //_ftChunk_h_

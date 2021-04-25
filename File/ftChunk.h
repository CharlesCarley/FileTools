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
        /// <summary>
        /// Flag to indicate that this chunk was modified
        /// </summary>
        BLK_MODIFIED = 0x01,

        /// <summary>
        /// Flag to indicate that this chunk has been reconstructed.
        /// </summary>
        BLK_LINKED = 0x02,
    };

    /// <summary>
    /// Default constructor to initialize fields.
    /// </summary>
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

    /// <summary>
    /// Pointer to the next chunk in the list.
    /// </summary>
    ftMemoryChunk* next;

    /// <summary>
    /// Pointer to the previous chunk in the list.
    /// </summary>
    ftMemoryChunk* prev;

    /// <summary>
    /// Information about this chunk.
    /// </summary>
    ftChunk chunk;

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

    /// <summary>
    /// Contains the Flags for this chunk.
    /// </summary>
    SKuint8 flag;

    /// <summary>
    /// Contains the chunk's type index into the memory table's type array.
    /// </summary>
    FTtype newTypeId;

    /// <summary>
    /// A reference to the current structure from the file table.
    /// </summary>
    ftStruct* fileStruct;

    /// <summary>
    /// A reference to the current structure from the memory.
    /// </summary>
    ftStruct* memoryStruct;
};

/// <summary>
/// Utility class to read, write, and scan chunks.
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

    /// <summary>
    /// Utility to read a chunk from the supplied stream.
    /// </summary>
    /// <param name="dest">The destination memory.</param>
    /// <param name="stream">The stream to read from.</param>
    /// <param name="headerFlags">A copy of the file's header flags.</param>
    /// <returns>The total number of bytes that were read from the stream. </returns>
    static SKsize read(ftChunk* dest, skStream* stream, int headerFlags);


    /// <summary>
    /// Utility to write a chunk to the supplied stream.
    /// </summary>
    /// <param name="source">The source memory to write</param>
    /// <param name="stream">The stream to write to.</param>
    /// <returns>The total number of bytes that were read from the stream. </returns>
    static SKsize write(ftChunk* source, skStream* stream);

    /// <summary>
    /// Utility to scan over chunks from the supplied stream.
    /// </summary>
    /// <param name="dest">The destination memory.</param>
    /// <param name="stream">The stream to read from.</param>
    /// <param name="headerFlags">A copy of the file's header flags.</param>
    /// <returns>The total number of bytes that were read from the stream. </returns>
    static SKsize scan(ftChunkScan* dest, skStream* stream, int headerFlags);

    static const ftChunk BlankChunk;
};

#endif  //_ftChunk_h_

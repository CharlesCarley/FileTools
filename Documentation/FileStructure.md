# File Structure


1. [File Structure](#file-structure)
   1. [Header](#header)
   2. [Chunks](#chunks)
   3. [Reserved Codes](#reserved-codes)

## Header

The file header is the first 12 bytes of the file.

| Bytes  | Data Type | Description                                                                 |
| :----: | --------- | :-------------------------------------------------------------------------- |
| [0,6]  | char[7]   | Are a unique name to identify the file type.                                |
|   7    | char      | Identifies the machine architecture that the file was was saved in.         |
|   8    | char      | Identifies the byte order that the file was was saved in.                   |
| [9,12] | int       | A integer that fills up to 3 ASCII characters; IE, version 1.5.0 equals 150 |

Codes for bytes 7,8

| ASCII | Hex  | Description                                                    |
| ----- | ---- | -------------------------------------------------------------- |
| '-'   | 0x2D | '-' indicates that the file was saved with 64 bit chunks.      |
| '_'   | 0x5F | '_' indicates that the file was saved with 32 bit chunks.      |
| 'V'   | 0x56 | 'V' indicates that the file was saved on a big endian machine. |
| 'v'   | 0x76 | 'V' indicates that the file was saved on a big endian machine. |

## Chunks


```c++
struct ChunkNative
{
    unsigned int code;    // 4 bytes
    unsigned int length;  // 4 bytes
    size_t       address; // 4|8 bytes
    unsigned int typeid;  // 4 bytes
    unsigned int count;   // 4 bytes
}; // 20 or 24 bytes total
```

The address member needs to be large enough to hold the base address of the pointer that this chunk represents.

```c++
struct Chunk32
{
    unsigned int code;    // 4 bytes
    unsigned int length;  // 4 bytes
    unsigned int address; // 4 bytes
    unsigned int typeid;  // 4 bytes
    unsigned int count;   // 4 bytes
}; // 20 bytes total
```

```c++
struct Chunk64
{
    unsigned int      code;    // 4 bytes
    unsigned int      length;  // 4 bytes
    unsigned int64_t  address; // 8 bytes
    unsigned int      typeid;  // 4 bytes
    unsigned int      count;   // 4 bytes
}; // 24 bytes total
```


| Member  | Description                                                                          |
| ------- | ------------------------------------------------------------------------------------ |
| code    | Is a unique IFF type identifier for identifying how this block should be read.       |
| length  | Is the size in bytes of the chunks data.                                             |
| address | Is the base address of the chunk data at the time of saving.                         |
| typeid  | Is the structure type index identifier found in the DNA1 block.                      |
| count   | Is the number of subsequent blocks being saved in this chunk starting with 'address' |

## Reserved Codes

The following are reserved chunk identifier codes that are used to save the file I/O API.  
See the [TableStructure](TableStructure.md) document for a description of their use.

+ DNA1
+ SDNA
+ NAME
+ TYPE
+ TLEN
+ STRC
+ DATA

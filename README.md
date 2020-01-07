# FileTools

The FileTools project is a small collection of tools that are centered around loading file formats which are similar in structure to the [Blender](https://blender.org) .blend file. The main idea of the file format is to take a C or C++ API and compile it into a set of tables that can be used to rebuild the API on load.

## Contents

1. [FileTools](#filetools)
   1. [Contents](#contents)
   2. [File Structure](#file-structure)
      1. [File Header](#file-header)
      2. [Chunk Header](#chunk-header)
      3. [Reserved Codes](#reserved-codes)
   3. [Table structure](#table-structure)
      1. [Type table(TYPE)](#type-tabletype)
      2. [Name table(NAME)](#name-tablename)
      3. [Type size table(TLEN)](#type-size-tabletlen)
      4. [Structure table(STRC)](#structure-tablestrc)
      5. [Building the tables](#building-the-tables)
         1. [Scanner and Compiler](#scanner-and-compiler)
         2. [Usage](#usage)
         3. [Output](#output)
         4. [CMake Utility](#cmake-utility)
      6. [Reversing a file's tables](#reversing-a-files-tables)

## File Structure

The file format is a simple chunk based format.  It consists of a 12-byte file header followed by any number of chunks. The last header should be an empty chunk with the ENDB code.

### File Header

The file header determines the file type, the architecture of the saving platform, and the API version.

| Byte(s) | Data Type | Description                                            |
| :-----: | --------- | :----------------------------------------------------- |
|  [0,6]  | char[7]   | Identify the file type.                                |
|    7    | char      | Is used to determine whether to load 32/64 bit chunks. |
|    8    | char      | Identifies the byte-order of the file.                 |
| [9,11]  | int       | Is a three-digit version code. (EG; 1.5.0 equals 150)  |

The following ASCII codes are reserved for bytes 7 and 8:

| ASCII | Hex  | Description                                                       |
| ----- | ---- | ----------------------------------------------------------------- |
| '-'   | 0x2D | '-' indicates that the file was saved with 64-bit chunks.         |
| '_'   | 0x5F | '_' indicates that the file was saved with 32-bit chunks.         |
| 'V'   | 0x56 | 'V' indicates that the file was saved on a big-endian machine.    |
| 'v'   | 0x76 | 'v' indicates that the file was saved on a little-endian machine. |

For example the .blend header:

```blend
BLENDER-v279
```

### Chunk Header

The chunk header is a varying sized structure that is 20 or 24 bytes. The size is dependent on the platform architecture at the time of saving because it stores the heap address of its data block.

```c++
struct ChunkNative
{
    unsigned int code;      // 4 bytes
    unsigned int length;    // 4 bytes
    size_t       address;   // 4|8 bytes
    unsigned int structId;  // 4 bytes
    unsigned int count;     // 4 bytes
}; // 20 or 24 bytes total

struct Chunk32
{
    unsigned int code;      // 4 bytes
    unsigned int length;    // 4 bytes
    unsigned int address;   // 4 bytes
    unsigned int structId;  // 4 bytes
    unsigned int count;     // 4 bytes
}; // 20 bytes total

struct Chunk64
{
    unsigned int      code;      // 4 bytes
    unsigned int      length;    // 4 bytes
    unsigned int64_t  address;   // 8 bytes
    unsigned int      structId;  // 4 bytes
    unsigned int      count;     // 4 bytes
}; // 24 bytes total
```

| Member   | Description                                                                          |
| -------- | :----------------------------------------------------------------------------------- |
| code     | Is a unique IFF type identifier for identifying how this block should be handled.    |
| length   | Is the size in bytes of the data block.                                              |
| address  | Is the base address of the chunk data at the time of saving.                         |
| structId | Is the structure type index identifier found in the DNA1->STRC block.                |
| count    | Is the number of subsequent blocks being saved in this chunk starting with 'address' |

A whole chunk includes a header and block of memory directly following the header. The block of memory should be the same length as described in the header.

```hex
Chunk32
Code   : OLIB
Len    : 152
Old    : 0x15170d0
TypeId : 0
Count  : 1
-------------------------------------------------------------------------------
00000000   43 75 62 65 00 00 00 00  00 00 00 00 00 00 00 00  |Cube............|
00000010   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000030   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000040   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000050   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000060   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000070   00 00 00 00 63 58 E2 3F  00 00 00 00 63 58 E2 3F  |....cX.?....cX.?|
00000080   00 00 00 00 63 58 E2 3F  02 00 00 00 50 41 44 42  |....cX.?....PADB|
00000090   68 09 B9 01 CD CD CD CD                           |h.......        |
-------------------------------------------------------------------------------

```

### Reserved Codes

The following are reserved chunk identifiers that the loader internally uses when reading a file.  

| CODE | Description                                                                                                                      |
| ---- | :------------------------------------------------------------------------------------------------------------------------------- |
| DNA1 | Chunk identifier that lets the loader separate and load the API.                                                                 |
| SDNA | Table header.                                                                                                                    |
| NAME | Specifies the member name table.                                                                                                 |
| TYPE | Specifies the type name table.                                                                                                   |
| TLEN | Specifies the type length table.                                                                                                 |
| STRC | Specifies the structure table.                                                                                                   |
| DATA | Indicates a block of data that may or may not have a structure associated with it but still needs to be relinked by its address. |
| ENDB | Indicates that there are no more chunks to read                                                                                  |

## Table structure

The tables consist of the type names,  member names, type sizes, and the structures that contain the types.

### Type table(TYPE)

This table starts with a 4-byte IFF code. Directly following the type code is a 4-byte integer holding the number of strings in the type table. The table itself is a NULL-terminated string array. Built-in or atomic types are declared first, which is followed by user-defined class/struct names.

### Name table(NAME)

The name table is identical in structure as the type table. The string array contains the member names of all user-defined types. Any extra information about a specific member other than its atomic type comes from the name. Which would be, for instance, determining whether or not it is a pointer, a pointer to a pointer, an array or multidimensional array, etc.. By default, FileTools supports only three-dimensional arrays. The preprocessor definition FT_ARR_DIM_MAX 3, may be changed to support larger n-dimensional arrays if needed.

### Type size table(TLEN)

The TLEN table is an array of 2-byte integers. It contains the same number of elements as the type table. The type index for the type table directly corresponds to the same element in the size table. The values in this table contain the computed size for the atomic types as well as all user-defined types.

For example:

```c
struct vec3f
{
    float x; // size[type[float]] * name[x].array_len  = 4 bytes  
    float y; // size[type[float]] * name[y].array_len  = 4 bytes
    float z; // size[type[float]] * name[z].array_len  = 4 bytes
}; // size[type[vec3f]] = 12 bytes  
```

### Structure table(STRC)

...

### Building the tables

The TableCompiler program compiles structures and classes into the tables. It reads the files via the command line.

#### Scanner and Compiler

The goal of the scanner is to extract only member variables from classes and structures. It will parse namespaces but only in as much to store it's name. Every time the scanner reads an 'n', 'c', or an 's', it will test for a namespace class or struct keyword. If any of the keywords match it will attempt to store the keyword identifier. If it matches a struct or a class keyword, the scanner will change states and attempt to extract and store member variables. The scanner will not parse c++ outside the scope of member variable declaration, and does not calculate the size of an object with base types. If any API specific methods are needed, the scanner tests for the comment keyword:
```txt 
// @makeft_ignore
```
If the scanner finds a match it will switch state to ignore everything  to the end of the file or until the next occurrence of the keyword. One thing to note is that any member variable declarations must be visible to the scanner in order for it to calculate the correct size of the class or structure. Members can be public or private from the perspective of C++, but no member variables should be declared in an ignore block.


#### Usage

```txt
Uasge: TableCompiler.exe <tablename> <ofilename> <ifile[0]> ... <ifile[n]>
```

| Argument    | Description                                       |
| :---------- | :------------------------------------------------ |
| tablename   | A unique name for the output table array.         |
| ofilename   | Output file and path name for the generated code. |
| ifile[0..n] | Input API declaration file(s) to parse.           |

#### Output

The output should be compiled back into into to the library that contains the rest of the file implementation.

```c
unsigned char DocsExampleTables[]={
    0x53,0x44,0x4E,0x41,0x4E,0x41,0x4D,0x45,0x03,0x00,0x00,0x00,
    0x78,0x00,0x79,0x00,0x7A,0x00,0x62,0x79,0x54,0x59,0x50,0x45,
    0x0B,0x00,0x00,0x00,0x63,0x68,0x61,0x72,0x00,0x75,0x63,0x68,
    0x61,0x72,0x00,0x73,0x68,0x6F,0x72,0x74,0x00,0x75,0x73,0x68,
    0x6F,0x72,0x74,0x00,0x69,0x6E,0x74,0x00,0x6C,0x6F,0x6E,0x67,
    0x00,0x75,0x6C,0x6F,0x6E,0x67,0x00,0x66,0x6C,0x6F,0x61,0x74,
    0x00,0x64,0x6F,0x75,0x62,0x6C,0x65,0x00,0x76,0x6F,0x69,0x64,
    0x00,0x76,0x65,0x63,0x33,0x66,0x00,0x62,0x54,0x4C,0x45,0x4E,
    0x01,0x00,0x01,0x00,0x02,0x00,0x02,0x00,0x04,0x00,0x04,0x00,
    0x04,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x0C,0x00,0x40,0x40,
    0x53,0x54,0x52,0x43,0x01,0x00,0x00,0x00,0x0A,0x00,0x03,0x00,
    0x07,0x00,0x00,0x00,0x07,0x00,0x01,0x00,0x07,0x00,0x02,0x00,
};
int DocsExampleLen=sizeof(DocsExampleTables);
```


The pure virtual methods in [ftFile](https://github.com/CharlesCarley/FileTools/blob/master/File/ftFile.h) provide a way for the loader to access the compiler output.

```c
virtual void*       getTables(void) = 0;
virtual FBTsize     getTableSize(void) = 0;
```

```c
#include "DocsExample.inl"

void* DocsExample::getTables(void)
{
    return (void*)DocsExampleTables;
}

FBTsize DocsExample::getTableSize(void)
{
    return DocsExampleLen;
}
```

#### CMake Utility

The [TableCompiler](https://github.com/snailrose/FileTools/blob/master/CMake/Readme.md) CMake utility can be used to attach table generation to a build. This macro outputs the include file to the current build directory then adds the build directory to the list of include paths.


### Reversing a file's tables

...


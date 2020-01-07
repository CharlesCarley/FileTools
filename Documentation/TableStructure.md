# Table structure

1. [Table structure](#table-structure)
   1. [Type table](#type-table)
   2. [Name table](#name-table)
   3. [Type size table](#type-size-table)
   4. [Structure table](#structure-table)
   5. [Name table](#name-table-1)
   6. [Structure table](#structure-table-1)
   7. [Building the memory tables](#building-the-memory-tables)
   8. [Reversing the file tables](#reversing-the-file-tables)

The tables consist of the type names,  member names, type sizes, and the structures that contain the types.

## Type table

The type table starts with the 4 byte IFF code TYPE. Directly following the type code is a 4-byte integer containing the number of names in the type table. The type table is a NULL-terminated string array.

The name is found by its 'ID' which is the index as it resides in this table.

Built-in or atomic types are declared first, followed by user-defined class/struct type names.

|   ID | Name          | Structure ID |                     Total size |
| ---: | :------------ | -----------: | -----------------------------: |
|    0 | char          |           -1 |                              1 |
|    1 | uchar         |           -1 |                              1 |
|    2 | short         |           -1 |                              2 |
|    3 | ushort        |           -1 |                              2 |
|    4 | int           |           -1 |                              4 |
|    5 | long          |           -1 |                              4 |
|    6 | ulong         |           -1 |                              4 |
|    7 | float         |           -1 |                              4 |
|    8 | double        |           -1 |                              8 |
|    9 | int64_t       |           -1 |                              8 |
|   10 | uint64_t      |           -1 |                              8 |
|   11 | scalar_t      |           -1 | 4 or 8 typedef double or float |
|   12 | void          |           -1 |                              0 |
|   13 | compiled type | unique index |          sizeof(compiled type) |

## Name table

The name table is identical in structure as the type table. It begins with the IFF type identifier code NAME.

The string array contains the member names of all user-defined types. Extra information about a specific member other than its atomic type needs to be computed from the name.  That would be, for instance, determining whether or not it is a pointer, a pointer to a pointer, an array or multidimensional array, etc..

By default, FileTools supports only three-dimensional. The macro definition FT_ARR_DIM_MAX 3, may be changed to support larger n-dimensional arrays.

## Type size table

The TLEN table is an array of 2-byte integers. It contains the same number of elements as the type table. The type index for the type table corresponds to the corresponding element in the size table.  The values in this table contain the computed size for the atomic types as well as user-defined types.

For example:

```c
struct vec3f
{
    float x; // size[type[float]] * name[x].array_len  = 4 bytes  
    float y; // size[type[float]] * name[y].array_len  = 4 bytes
    float z; // size[type[float]] * name[z].array_len  = 4 bytes
}; // size[type[vec3f]] = 12 bytes  
```



## Structure table

| ID  | Name  | Member count | Total size |
| --- | :---: | :----------: | :--------: |

For example:

If the compiled API is a header file consisting of a vec3f structure.

```c
#ifndef _API_H_
#define _API_H_

typedef struct vec3f {
    float x, y, z;
} vec3f;

#endif//_API_H_
```

the file table output will be

| ID  |   Name   | Structure ID | Total size |
| --- | :------: | :----------: | :--------: |
| 0   |   char   |      -1      |     1      |
| 1   |  uchar   |      -1      |     1      |
| 2   |  short   |      -1      |     2      |
| 3   |  ushort  |      -1      |     2      |
| 4   |   int    |      -1      |     4      |
| 5   |   long   |      -1      |     4      |
| 6   |  ulong   |      -1      |     4      |
| 7   |  float   |      -1      |     4      |
| 8   |  double  |      -1      |     8      |
| 9   | int64_t  |      -1      |     8      |
| 10  | uint64_t |      -1      |     8      |
| 11  | scalar_t |      -1      |   4 or 8   |
| 12  |   void   |      -1      |     0      |
| 13  |   vec3   |      0       |     12     |

## Name table

| ID  | Name  | Pointer | Array size |
| --- | :---: | :-----: | :--------: |
| 0   |   x   |    0    |     1      |
| 1   |   y   |    0    |     1      |
| 2   |   z   |    0    |     1      |

## Structure table

| ID  | Name  | Member count | Total size |
| --- | :---: | :----------: | :--------: |
| 0   | vec3  |      3       |     12     |

## Building the memory tables

The memory tables are generated from input files with the build tool TableCompiler. The file tables are written to the file as a copy of the current state of the memory tables.

See [TableCompiler](TableCompiler.md)


## Reversing the file tables

The memory tables are generated from input files with the build tool TableCompiler. The file tables are written to the file as a copy of the current state of the memory tables.

See [TableDecompiler](TableDecompiler.md)

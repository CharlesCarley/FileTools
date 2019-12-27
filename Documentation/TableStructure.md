# Table structure

1. [Table structure](#table-structure)
   1. [Type table](#type-table)
   2. [Name table](#name-table)
   3. [Structure table](#structure-table)
   4. [Name table](#name-table-1)
   5. [Structure table](#structure-table-1)
   6. [Building the memory tables](#building-the-memory-tables)

The compiled lookup tables contain the type names, type sizes, and the structures that contain the types.

## Type table

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

The first thirteen elements in the table are the standard builtin data types.

## Name table

The name table is an list of structures.
The name member is an integer that points to the array index in the name table.
Pointer represents a Boolean value indicating whether or not the variable name is a pointer. Array size is used to determine the number of elements in each specific name.

|   ID | Name | Pointer | Array size |
| ---: | :--- | ------: | ---------: |
|    0 | ...  |      -1 |          1 |

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

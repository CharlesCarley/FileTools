# Table structure

1. [Table structure](#table-structure)
   1. [Type table(TYPE)](#type-tabletype)
   2. [Name table(NAME)](#name-tablename)
   3. [Type size table(TLEN)](#type-size-tabletlen)
   4. [Structure table(STRC)](#structure-tablestrc)
   5. [Building the tables](#building-the-tables)
   6. [Reversing a file's tables](#reversing-a-files-tables)

The tables consist of the type names,  member names, type sizes, and the structures that contain the types.

## Type table(TYPE)

This table starts with a 4-byte IFF code. Directly following the type code is a 4-byte integer holding the number of strings in the type table. The table itself is a NULL-terminated string array. Built-in or atomic types are declared first, which is followed by user-defined class/struct names.

## Name table(NAME)

The name table is identical in structure as the type table. The string array contains the member names of all user-defined members. Any extra information about a specific member other than its atomic type comes from the name. Which would be, for instance, determining whether or not it is a pointer, a pointer to a pointer, an array or multidimensional array, etc.. By default, FileTools supports only three-dimensional arrays. The macro definition FT_ARR_DIM_MAX 3, may be changed to support larger n-dimensional arrays if needed.

## Type size table(TLEN)

The TLEN table is an array of 2-byte integers. It contains the same number of elements as the type table. The type index for the type table corresponds to the corresponding element in the size table. The values in this table contain the computed size for the atomic types as well as user-defined types.

For example:

```c
struct vec3f
{
    float x; // size[type[float]] * name[x].array_len  = 4 bytes  
    float y; // size[type[float]] * name[y].array_len  = 4 bytes
    float z; // size[type[float]] * name[z].array_len  = 4 bytes
}; // size[type[vec3f]] = 12 bytes  
```

## Structure table(STRC)

...


## Building the tables

See [TableCompiler](TableCompiler.md)

## Reversing a file's tables

See [TableDecompiler](TableDecompiler.md)

# FileTools

File tools is a file loader originally based off the blender .blend file. The .blend a chunk based file format based off of the EA-IFF 85 standard.

Chunks are composed of a header followed by a memory dump of the type.


Structures are compiled into a set of tables which can be reconstructed back into their current form. The loader does this by maintaining two sets of tables one for the file, and the other for memory.


File tools is a file format based off the blender .blend file. The .blend basically consists of a set of tables that are compiled from a C API. The compiled lookup tables contain type names, size of all the types, and the parent structures. One set of tables gets placed in the file during the saving process and the other table is maintained in memory. The file loader resolves discrepancies between the file and memory tables then casts the file data through the memory tables. This allows previously saved structures to be rebuilt into the newest version.  

Files are 32/64 bit safe and saved in native byte order. If the byte order varies between the file version and the loading platform. The bytes will be swapped to native endian.

Pointer addresses at the time of saving are written to the file. Then during loading, the saved addresses will be used to relink the new data being read.

Documentation efforts, refactoring, and testing for consistency is still a work in progress.

## Introduction

* [Table-structure](Table-structure.md)
* [makeft](makeft.md)


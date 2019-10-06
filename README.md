# FileTools

File tools is a file format based off the blender .blend file. The .blend basically consists of a set of tables that are compiled from a C API. The compiled lookup tables contain type names, size of all the types, and the parent structures. One set of tables gets placed in the file during the saving process and the other table is maintained in memory. The file loader resolves discrepancies between the file and memory tables then casts the file data through the memory tables.

Files are 32/64 bit safe and saved in native byte order. If the byte order varies between the file version and the loading platform. The bytes will be swapped to native endian.

All structure members saved in the tables must be four-byte aligned.

Pointer addresses at the time of saving are written to the file. Then during loading, the saved addresses will be used to relink the new data being read.

Documentation efforts, refactoring, and testing for consistency is still a work in progress.

See the wiki for an introduction to the file format.

* [Table-structure](Table-structure.md)
* [makeft](makeft.md)

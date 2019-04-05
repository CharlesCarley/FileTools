# FileTools
File tools is a file format based off the blender .blend file.  Which basically consists of a set of tables that are compiled from a C/C++ API. The compiled lookup tables contain type names, size of all the types, and the structures that contain the types. 

Files are 32/64 bit safe and saved in native byte order. If the byte order varies between the file version and the loading platform. The bytes will be swapped to native endian. 

All variables saved in the tables must be four-byte aligned.

Pointer addresses at the time of saving are written to the file. Then during loading, the saved addresses will be used to relink the new data being read.

Documentation efforts, refactoring and testing for consistency is still a work in progress. 

See the wiki for an introduction to the file format.
* [Wiki](https://github.com/snailrose/FileTools/wiki)

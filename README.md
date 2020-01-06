# FileTools

1. [FileTools](#filetools)
   1. [Introduction](#introduction)

## Introduction

The FileTools project is a small collection of tools that are centered around loading file formats which are similar in structure to the [Blender](https://blender.org) .blend file. The original library was written as part of the [OgreKit](https://github.com/gamekit-developers/gamekit) project.  

The main idea of the file format is to take a C/C++ API, compile it into a set of tables that can be used to rebuild the API on load.

+ [File Structure](Documentation/FileStructure.md) document for information on how the file is structured.
+ [Table Structure](Documentation/TableStructure.md) document for information on how the tables are structured.
+ [Table Compiler](Documentation/TableCompiler.md) is the build tool that will read classes and structures then turn them into the API Tables.
+ [Table Decompiler](Documentation/TableDecompiler.md) is a utility will reverse the file's tables back into a single useable C++ header.


# FileTools

1. [FileTools](#filetools)
   1. [Introduction](#introduction)

## Introduction

The FileTools project is a small collection of tools that are centered around loading file formats which are similar in structure to the [Blender](https://blender.org) .blend file. The library was initially written as part of the [OgreKit](https://github.com/gamekit-developers/gamekit) project.

The main idea of the file format is to take a C or C++ API and compile it into a set of tables that can be used to rebuild the API on load.

+ The [File Structure](Documentation/FileStructure.md) document contains information on how the file is structured.
+ The [Table Structure](Documentation/TableStructure.md) document contains information on how the compiled tables are structured.
+ The [Table Compiler](Documentation/TableCompiler.md) is the build tool that will read classes and structures then turn them into the API Tables.
+ The [Table Decompiler](Documentation/TableDecompiler.md) is a utility that will reverse a file's tables back into a single useable C++ header.

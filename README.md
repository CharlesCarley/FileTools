# FileTools

1. [FileTools](#filetools)
   1. [Introduction](#introduction)
      1. [File Structure](#file-structure)
      2. [Table Structure](#table-structure)
      3. [Table Compiler](#table-compiler)
      4. [Table Decompiler](#table-decompiler)
   2. [References](#references)

## Introduction

Is a file format that is based off of the blender .blend file. It was originally written in a hurry as part of the [OgreKit project](https://github.com/gamekit-developers/gamekit). This project aims to clean up, and complete that version.

The main idea of the loader is to serialize a C/C++ API that is written to both the file and the program that loads the file.
The [ftFile](File/ftFile.h) and [ftTables](File/ftTables.h) classes use the API tables to reconstruct the saved file memory into the most current version that is compiled into the program.

### File Structure

See the [File Structure](Documentation/FileStructure.md) document for information on how the file is structured.

### Table Structure

See the [Table Structure](Documentation/TableStructure.md) document for information on how the tables are structured.

### Table Compiler

The [Table Compiler](Documentation/TableCompiler.md) is the build tool that will read classes and structures then turn them into the API Tables.

### Table Decompiler

The [Table Decompiler](Documentation/TableDecompiler.md) is a utility will reverse the file's tables back into a single useable C++ header.

## References

* [Blender](https://blender.org)

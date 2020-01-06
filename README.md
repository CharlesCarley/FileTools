# FileTools

1. [FileTools](#filetools)
   1. [Introduction](#introduction)
      1. [File Structure](#file-structure)
      2. [Table Structure](#table-structure)
      3. [Table Compiler](#table-compiler)
      4. [Table Decompiler](#table-decompiler)
   2. [References](#references)

## Introduction

The FileTools project is a collection of tools that are centered around loading file formats which are based on the Blender .blend file.  The original library was written as part of the [OgreKit](https://github.com/gamekit-developers/gamekit) project. This project aims to clean up and complete that version.

The main idea of the file format is to take a C/C++ API, compile it into a set of tables that can then be used to rebuild structure or class memory which is saved in chunks.

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

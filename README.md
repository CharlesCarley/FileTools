# FileTools

1. [FileTools](#filetools)
   1. [Introduction](#introduction)
   2. [References](#references)

## Introduction

Is a file format that is based off of the blender .blend file.
The main idea is to serialize a C/C++ API that is written to both the file and, the program that loads the file.
The [ftFile](@) and [ftTable](@) classes use the API tables to reconstruct the saved file memory into the most current version that is compiled into the program.

* [File Structure](Documentation/FileStructure.md)
* [Table Structure](Documentation/TableStructure.md)
* [Table Compiler](Documentation/TableCompiler.md)

## References

* [Blender](https://blender.org)

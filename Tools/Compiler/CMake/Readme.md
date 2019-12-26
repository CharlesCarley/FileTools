# CMake Build Utilities

## MakeFT

The MakeFT.cmake file can be used to add a custom build targets.

### macro(ADD_FT TARGET ARGN)

```CMake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/DirectoryContainingMakeFT)
include(MakeFT)

set(ApiFiles
   StructDecl1.h
   StructDecl2.h
   StructDecl3.h
   StructDecl4.h
)

add_ft(OutputVar ${ApiFiles})

set(Source
   Implementation.h
   Implementation.cpp
   ${OutputVar}
   ${ApiFiles}
)
add_library(LibName ${Source})
```

### macro(ADD_FT_VALIDATOR TARGET ARGN)

ADD_FT_VALIDATOR generates two output files

1. The same tables as above.
2. A validation program that asserts the computed structure sizes. If validation fails a compile error will be generated.

```CMake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/DirectoryContainingMakeFT)
include(MakeFT)

set(ApiFiles
   StructDecl1.h
   StructDecl2.h
   StructDecl3.h
   StructDecl4.h
)

add_ft_validator(OutputVar ${ApiFiles})

set(Source
   Implementation.h
   Implementation.cpp 
   ${OutputVar}
   ${ApiFiles}
)
add_library(LibName ${Source})
```

## CMake build utilities


### MakeFT
MakeFT can be used to add a build time target inorder to call makeft on a group of source files. 

```CMake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/DirectoryContainingMakeFT)

include(MakeFT)


set(API_FILES
   StructDecl1.h
   StructDecl2.h
   StructDecl3.h
   StructDecl4.h
)

add_ft(output_var ${API_FILES})


set(Source
   Implementation.h
   Implementation.cpp   


   ${output_var}
   ${API_FILES}
)



add_library(LibName ${Source})

```




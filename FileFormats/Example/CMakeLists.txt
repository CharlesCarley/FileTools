include(MakeFT)

set(ApiFiles
    API/FileInfo.h
)

add_ft(ExampleTables ${ApiFiles})

set(SRC
   Example.h
   Example.cpp

   ${ApiFiles}
   ${ExampleTables}
)

include_directories(${ftINCLUDE})
add_library(Example ${SRC})

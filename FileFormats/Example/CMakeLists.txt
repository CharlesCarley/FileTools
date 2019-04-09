include(MakeFT)

set(ApiFiles
    API/FileInfo.h
    API/Data1.h
    API/Data2.h
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

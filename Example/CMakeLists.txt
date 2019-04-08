


set(SRC 
   Main.cpp
)



include_directories(${FileTools_SOURCE_DIR}/FileFormats/Example ${FileTools_SOURCE_DIR}/File)
add_executable(ExampleFile ${SRC})
target_link_libraries(ExampleFile Example ftFile)

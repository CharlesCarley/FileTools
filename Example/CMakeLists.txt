include(MakeFT)


include_directories(${CMAKE_SOURCE_DIR}/File)
add_ft(Tables API.h)

set(SRC
    Example.h
    Example.cpp
    Main.cpp
    API.h
    ${Tables}
)

add_executable(Example ${SRC})



target_link_libraries(Example ftFile)
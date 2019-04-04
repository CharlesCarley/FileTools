#------------------------------------------------------------------------------
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#------------------------------------------------------------------------------
#include(FlexBison)

set(File_SRC
    ftFile.cpp
    ftTables.cpp    
    ftTypes.cpp
    ftStreams.cpp
    ftCompiler.cpp
)

set(File_HDR
    ftFile.h
    ftTables.h
    ftCompiler.h
    ftTypes.h
    ftConfig.h
    ftStreams.h
    ftPlatformHeaders.h
)

# only for updating scanner 
#add_flex(ftScanner File_HDR ftScanner.l ft)
set(ftScanner Generated/ftScanner.inl)

if (ftUSE_ZLIB)
    include_directories(${ftZLIB_INCLUDE})
    link_libraries(${ftZLIB_LIBRARY})
endif()
add_library(ftFile ${File_SRC} ${File_HDR} ${ftScanner})

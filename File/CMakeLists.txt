# -----------------------------------------------------------------------------
#
#   Copyright (c) 2019 Charles Carley.
#
#   Contributor(s): none yet
#
# ------------------------------------------------------------------------------
#   This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
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
# ------------------------------------------------------------------------------
set(TargetName ${FileTools_TARGET_NAME})

set(File_SRC
    ftAtomic.cpp
    ftBuildInfo.cpp
    ftBuildMember.cpp
    ftBuildStruct.cpp
    ftChunk.cpp
    ftCompiler.cpp
    ftEndianUtils.cpp
    ftFile.cpp
    ftLogger.cpp
    ftMember.cpp
    ftScanDNA.cpp
    ftScanner.cpp
    ftStreams.cpp
    ftStruct.cpp
    ftTables.cpp
    ftToken.cpp
    ftTypes.cpp
)

set(File_HDR
    ftAtomic.h
    ftBuildInfo.h
    ftBuildMember.h
    ftBuildStruct.h
    ftChunk.h
    ftCompiler.h
    ftConfig.h
    ftEndianUtils.h
    ftFile.h
    ftHashTypes.h
    ftLogger.h
    ftMember.h
    ftPlatformHeaders.h
    ftScanDNA.h
    ftScanner.h
    ftStreams.h
    ftStruct.h
    ftTables.h
    ftToken.h
    ftTypes.h
)

if (FileTools_USE_ZLIB)
    include_directories(${FileTools_ZLIB_INCLUDE})
    add_definitions(${FileTools_ZLIB_DEFINITIONS})
    link_libraries(${FileTools_ZLIB_LIBRARY})
endif()

include_directories(${FileTools_Utils_INCLUDE})
add_library(${TargetName} ${File_SRC} ${File_HDR})

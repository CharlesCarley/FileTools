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
cmake_minimum_required(VERSION 3.0)
project(BlendFile)
set(BlendFile_TARGET BlendFile)

set(BlendFile_BUILD_TEST FALSE CACHE BOOL "Builds a test .blend program.")
set(BlendFile_TOOLS_INCLUDE ${FileTools_INCLUDE})
set(BlendFile_TOOLS_LIBRARY ${FileTools_LIBRARY})


set(BlendFile_INCLUDE ${BlendFile_CURRENT_SOURCE_DIR} ${BlendFile_TOOLS_INCLUDE})
set(BlendFile_LIBRARY ${BlendFile_TARGET} ${BlendFile_TOOLS_LIBRARY})

include(MakeFT)


add_ft(bfBlender Blender.h)

set(BlendFile_SRC
    ftBlend.h
    ftBlend.cpp
    Blender.h
)

include_directories(${BlendFile_INCLUDE})

add_library(
    ${BlendFile_TARGET} 
    ${BlendFile_SRC} 
    ${bfBlender}
)

link_libraries(${BlendFile_TOOLS_LIBRARY})


if (BlendFile_BUILD_TEST)
    subdirs(Test)
endif()



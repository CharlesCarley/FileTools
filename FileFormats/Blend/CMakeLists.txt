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
option(FileTools_BLEND_TEST "Builds a test .blend program." OFF)

if (BlendFile_ExternalTarget)
    set(TargetFolders ${BlendFile_TargetFolders})
    set(TargetName   ${BlendFile_TargetName})
    set(TargetGroup  ${BlendFile_TargetGroup})
else()
    set(TargetFolders FALSE)
    set(TargetName    BlendFile)
    unset(TargetGroup )
endif()


set(BlendFile_TOOLS_INCLUDE ${FileTools_INCLUDE})
set(BlendFile_TOOLS_LIBRARY ${FileTools_LIBRARY})

set(BlendFile_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}  ${BlendFile_TOOLS_INCLUDE})
set(BlendFile_LIBRARY ${TargetName} ${BlendFile_TOOLS_LIBRARY})

include(TableCompiler)
add_table(bfBlender Blender.h)

set(BlendFile_SRC
    ftBlend.h
    ftBlend.cpp
    Blender.h
)

include_directories(${BlendFile_INCLUDE})

add_library(
    ${TargetName} 
    ${BlendFile_SRC} 
    ${bfBlender}
)

link_libraries(${BlendFile_TOOLS_LIBRARY})

if (FileTools_BLEND_TEST)
    subdirs(Test)
endif()


if (TargetFolders)
    set_target_properties(${TargetName} PROPERTIES FOLDER "${TargetGroup}")
endif()

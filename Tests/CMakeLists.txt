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
set(TargetName Tests)
include (Data2Array)

file(GLOB LS    *.cpp)
file(GLOB LH    *.h)
file(GLOB CH    catch/*.h)
file(GLOB CS    catch/*.cpp)
file(GLOB TMPL  Templates/*.h)

add_templates(Templates.h ${TMPL})
set(SRC
    Templates.h
    ${LS} ${LH}
    ${CH} ${CS}
    ${TMPL}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../File)
include_directories(${FileTools_INCLUDE})

add_executable(${TargetName} ${SRC})

target_link_libraries(
    ${TargetName} 
    ${FileTools_LIBRARY}
)

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
cmake_minimum_required(VERSION 2.4)
project(FileTools)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)


set(ftUSE_ZLIB 0)
set(ftZLIB_INCLUDE "")
set(ftZLIB_LIBRARY "")	

if (ftUSE_ZLIB)
    add_definitions(-DftUSE_GZ_FILE=1)
endif()

#add_definitions(-DFftUSE_COMPILER_CHECKS=0)
#add_definitions(-DftUSE_DEBUG_ASSERTS=0)


subdirs(File Compiler)
subdirs(FileFormats/Blend)
subdirs(Example)
subdirs(FileTest)

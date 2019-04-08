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
list(APPEND CMAKE_MODULE_PATH ${FileTools_SOURCE_DIR}/CMake)


set(ftINCLUDE ${FileTools_SOURCE_DIR}/File)

set(ftUSE_ZLIB 1)
set(ftZLIB_INCLUDE ${FileTools_SOURCE_DIR}/File/zlib)
set(ftZLIB_LIBRARY "zlib")	

if (ftUSE_ZLIB )
    add_definitions(-DftUSE_GZ_FILE=1)
    subdirs(File/zlib)
endif()

#add_definitions(-DFftUSE_COMPILER_CHECKS=0)
#add_definitions(-DftUSE_DEBUG_ASSERTS=0)

subdirs(File Compiler)
subdirs(FileFormats/Blend)
subdirs(FileFormats/Example)
subdirs(FileTest)
subdirs(Example)

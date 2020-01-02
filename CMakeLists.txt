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
project(FileTools)

#cmake_policy(SET CMP0077 OLD)

option(FileTools_BLD_TESTS        "Build basic tests." OFF)
option(FileTools_BLD_BLEND_FILE   "Build the blend file version." OFF)
option(FileTools_USE_ZLIB         "Allow GZ compressed files to be read." OFF)
option(FileTools_USE_OWN_ZLIB     "Define this if zlib is used as part of a larger project." OFF)
option(FileTools_BLD_EXAMPLE_FILE "Build the example file version." OFF)

if (FileTools_USE_OWN_ZLIB)
    option(FileTools_USE_ZLIB FORCE ON)
endif()



if (NOT FileTools_UTILS_EXTERNAL)

    set(FileTools_Utils_ROOT ${FileTools_SOURCE_DIR}/External/Utils)
    set(FileTools_Utils_INCLUDE ${FileTools_Utils_ROOT}/../)
    set(FileTools_Utils_LIBRARY Utils)
    set(FileTools_Utils_CMAKE   ${FileTools_Utils_ROOT}/CMake)


    if (NOT IS_DIRECTORY ${FileTools_Utils_CMAKE})
        message(STATUS "Attempting to clone Utils into ${FileTools_Utils_ROOT}")

        execute_process( 
            WORKING_DIRECTORY ${FileTools_SOURCE_DIR} 
            COMMAND git submodule update --init --checkout
            ERROR_FILE ${FileTools_BINARY_DIR}/GetSubmoduleUpdate.txt
            OUTPUT_QUIET
            ERROR_QUIET
        )
        if (NOT IS_DIRECTORY ${FileTools_Utils_CMAKE})
            message("Failed to clone Utils. See ${FileTools_BINARY_DIR}/GetSubmoduleUpdate.txt for more information.")
        endif()
    endif()

    subdirs(${FileTools_Utils_ROOT})

else()
   
    # These must be explicitly set before calling this script

    set(FileTools_Utils_INCLUDE ${Utils_INCLUDE})
    set(FileTools_Utils_LIBRARY ${Utils_LIB})
    set(FileTools_Utils_CMAKE   ${Utils_INCLUDE}/CMake)

endif()



list(APPEND CMAKE_MODULE_PATH 
    ${FileTools_SOURCE_DIR}/CMake
    ${FileTools_SOURCE_DIR}/Tools/TableCompiler/CMake
    ${FileTools_Utils_CMAKE}
)
set(FileTools_TARGET_NAME       FileTools)
set(FileTools_COMPILER_NAME     TableCompiler)
set(FileTools_DECOMPILER_NAME   TableDecompiler)


# This is the main include definition 
# to use the file classes.
set(FileTools_INCLUDE 
    ${FileTools_SOURCE_DIR}/File
    ${FileTools_Utils_INCLUDE}
)
set(FileTools_LIBRARY ${FileTools_TARGET_NAME} ${FileTools_Utils_LIBRARY})


if (FileTools_USE_ZLIB)

    if (FileTools_USE_OWN_ZLIB)
        set(FileTools_ZLIB_INCLUDE  ${FileTools_SOURCE_DIR}/File/zlib)
        set(FileTools_ZLIB_LIBRARY  zlib)
        set(FileTools_ZLIB_DEFINITIONS "-DFT_USE_ZLIB=1")

        subdirs(${FileTools_ZLIB_INCLUDE})

    else()
        SET(FileTools_ZLIB_FOUND 1)
        if (NOT FileTools_ZLIB_INCLUDE)
            message("FileTools_ZLIB_INCLUDE=<undefined>")
            SET(FileTools_ZLIB_FOUND 0)        
        endif()

        if (NOT FileTools_ZLIB_LIBRARY)
            message("FileTools_ZLIB_LIBRARY=<undefined>")
            SET(FileTools_ZLIB_FOUND 0)        
        endif()

        if (FileTools_ZLIB_FOUND EQUAL 0)
            message(FATAL_ERROR "FileTools_ZLIB_INCLUDE and FileTools_ZLIB_LIBRARY "
            "must Be defined in a higher scope if the build is using an external source version.")
        endif()
    endif()


   list(APPEND FileTools_INCLUDE
       ${FileTools_ZLIB_INCLUDE} )
   list(APPEND FileTools_LIBRARY
       ${FileTools_ZLIB_LIBRARY} )
endif()




subdirs(File)
subdirs(Tools/Compiler)
subdirs(Tools/Decompiler)

if (FileTools_BLD_BLEND_FILE)
    subdirs(FileFormats/Blend)
endif()

if (FileTools_BLD_EXAMPLE_FILE)
    subdirs(FileFormats/Example)
endif()


if (FileTools_BLD_TESTS)

    # The test files depend on Data2Array 
    # to build parsable source   
    list(APPEND CMAKE_MODULE_PATH 
        ${FileTools_SOURCE_DIR}/External/Data2Array/CMake
    )  
    subdirs(External/Data2Array)
    subdirs(Tests)

endif()

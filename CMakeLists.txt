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



set(FileTools_BLD_TESTS         FALSE           CACHE BOOL      "Build basic tests.")
set(FileTools_BLD_BLEND_FILE    FALSE           CACHE BOOL      "Build the blend file version.")
set(FileTools_USE_ZLIB          FALSE           CACHE BOOL      "Allow GZ compressed files to be read.")
set(FileTools_USE_OWN_ZLIB      FALSE           CACHE BOOL      "Define this if zlib is used as part of a larger project.")


if (NOT FileTools_UTILS_EXTERNAL)

    set(FileTools_Utils_ROOT ${FileTools_SOURCE_DIR}/External/Utils)
    set(FileTools_Utils_INCLUDE ${FileTools_Utils_ROOT}/../)
    set(FileTools_Utils_LIBRARY Utils)
    set(FileTools_Utils_CMAKE   ${FileTools_Utils_ROOT}/CMake)


    if (NOT IS_DIRECTORY ${FileTools_Utils_CMAKE})
        message(STATUS "Attempting to cloning Utils into ${FileTools_Utils_ROOT}")

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
    ${FileTools_Utils_CMAKE}
)
set(FileTools_TARGET_NAME   FileTools)
set(FileTools_COMPILER_NAME TableCompiler)

# This is the main include definition 
# to use the file classes.
set(FileTools_INCLUDE 
    ${FileTools_SOURCE_DIR}/File
    ${FileTools_Utils_INCLUDE}
)
set(FileTools_LIBRARY      ${FileTools_TARGET_NAME} ${FileTools_Utils_LIBRARY})





if (FileTools_USE_ZLIB)

    if (FileTools_USE_OWN_ZLIB)
        set(FileTools_ZLIB_INCLUDE  ${FileTools_SOURCE_DIR}/File/zlib)
        set(FileTools_ZLIB_LIBRARY  zlib)
        set(FileTools_ZLIB_DEFINITIONS "-DFT_USE_ZLIB=1")

        subdirs(${FileTools_ZLIB_INCLUDE})

    else()
        # These must defined in a higher scope

        if (NOT FileTools_ZLIB_INCLUDE)
            message(STATUS "FileTools_ZLIB_INCLUDE=<undefined>")
        endif()

        if (NOT FileTools_ZLIB_LIBRARY)
            message(STATUS "FileTools_ZLIB_LIBRARY=<undefined>")
        endif()

   endif()


   list(APPEND FileTools_INCLUDE
       ${FileTools_ZLIB_INCLUDE} )
   list(APPEND FileTools_LIBRARY
       ${FileTools_ZLIB_LIBRARY} )

endif()




subdirs(File Compiler)


if (FileTools_BLD_BLEND_FILE)
    subdirs(FileFormats/Blend)
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


#if (NOT ftUSE_OWN_ZLIB)
#    set(ftUSE_ZLIB 0)
#    set(ftZLIB_INCLUDE ${FileTools_SOURCE_DIR}/File/zlib)
#    set(ftZLIB_LIBRARY zlib)	
#    if (ftUSE_ZLIB )
#        add_definitions(-DftUSE_GZ_FILE=1)
#        subdirs(File/zlib)
#    endif()
#else()
#    if (ftUSE_ZLIB )
#        add_definitions(-DftUSE_GZ_FILE=1)
#    endif()
#endif()
#subdirs(FileFormats/Blend)
#if (NOT ftNO_EXAMPLES)
#    subdirs(FileFormats/Example)
#    subdirs(FileTest)
#    subdirs(Example)
#endif()


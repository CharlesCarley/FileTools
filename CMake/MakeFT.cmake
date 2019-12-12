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
set(FT_EXECUTABLE makeft)
# ----------------------------------------------------------------------------#
#                                                                             #
# Get a list of absolute paths                                                #
#                                                                             #
# ----------------------------------------------------------------------------#
macro(FT_ABSOLUTE_SRC TARGET)
    foreach (it ${ARGN})
        get_filename_component(N ${it} ABSOLUTE)
        set(${TARGET} ${${TARGET}} ${N})
    endforeach(it)
endmacro(FT_ABSOLUTE_SRC)

# ----------------------------------------------------------------------------#
#                                                                             #
# Get a list of names paths                                                   #
#                                                                             #
# ----------------------------------------------------------------------------#
macro(FT_BASE_SRC TARGET)
    foreach (it ${ARGN})
        get_filename_component(N ${it} NAME)
        set(${TARGET} ${${TARGET}} ${N})
    endforeach(it)
endmacro(FT_BASE_SRC)

# ----------------------------------------------------------------------------#
#                                                                             #
#   Usage: ${FT_EXECUTABLE} ${OUTFILE} {ARGN}                                #
#                                                                             #
# ----------------------------------------------------------------------------#
macro(ADD_FT TARGET)
    
    set(SRC_FILES )
    set(BASE_FILES )
    set(OUTFILE )

    ft_absolute_src(SRC_FILES ${ARGN})
    ft_base_src(BASE_FILES ${ARGN})

    get_filename_component(TARNAME ${TARGET} NAME)
    set(OUTFILE ${CMAKE_CURRENT_BINARY_DIR}/${TARNAME}.inl)


    add_custom_command(
	    OUTPUT ${OUTFILE}
	    COMMAND ${FT_EXECUTABLE} ${TARNAME} ${OUTFILE} ${SRC_FILES}
	    DEPENDS ${FT_EXECUTABLE} ${SRC_FILES}
	    )
    set(${TARGET} ${OUTFILE})
endmacro(ADD_FT)

# ----------------------------------------------------------------------------#
#                                                                             #
#   Usage: ${FT_EXECUTABLE} ${OUTFILE} {ARGN}                                #
#          Writes a validation target                                         #
#                                                                             #
# ----------------------------------------------------------------------------#
macro(ADD_FT_VALIDATOR TARGET)
    
    set(SRC_FILES )
    set(BASE_FILES )
    set(OUTFILE )

    ft_absolute_src(SRC_FILES ${ARGN})
    ft_base_src(BASE_FILES ${ARGN})

    get_filename_component(TARNAME ${TARGET} NAME)
    set(OUTFILE  ${CMAKE_CURRENT_BINARY_DIR}/${TARNAME}.inl)
    set(OUTFILEV ${CMAKE_CURRENT_BINARY_DIR}/${TARNAME}Validator.cpp)

    add_custom_command(
	    OUTPUT ${OUTFILE} ${OUTFILEV}
	    COMMAND ${FT_EXECUTABLE} ${TARNAME} ${OUTFILE} ${SRC_FILES}
	    DEPENDS ${FT_EXECUTABLE} ${SRC_FILES}
	    COMMENT ""
	    )


    add_executable(${TARNAME}Validator ${OUTFILEV})
    add_custom_command(
        TARGET ${TARNAME}Validator
        POST_BUILD
	    COMMAND ${TARNAME}Validator
	    COMMENT "Validating -> ${TARNAME}"
	    )

    set(${TARGET} ${OUTFILE})
endmacro(ADD_FT_VALIDATOR)


cmake_minimum_required( VERSION 2.8.12 )
project( cgiturl )
set( QT_VERSION_REQ "5.2" )
set( CMAKE_CXX_STANDARD 11 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" )
include(FindPkgConfig)

# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )

#
# Iconv
#

find_path( ICONV_INCLUDE_DIRS iconv.h )
find_library( ICONV_LIBRARIES NAMES iconv libiconv libiconv-2 c )

if( NOT ICONV_INCLUDE_DIRS OR NOT ICONV_LIBRARIES )
    message( "=" )
    message( "= Warning: Iconv not found, cgiturl might not build =" )
    message( "=" )
endif()

#
# Follow source files, headers
#

SET( SRCS
    main.cpp
    coding_functions.cpp
    math_functions.cpp
)

add_executable( cgiturl ${SRCS} )

target_link_libraries( cgiturl ${ICONV_LIBRARIES} )

target_include_directories( cgiturl PRIVATE ${ICONV_INCLUDE_DIR} )

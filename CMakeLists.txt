cmake_minimum_required( VERSION 3.20 )

project( match-tty CXX )

include ( FetchContent )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(fmt)
find_package(ftxui)

add_library( internal-include INTERFACE )

add_subdirectory( include )
add_subdirectory( examples )
add_subdirectory( tests )

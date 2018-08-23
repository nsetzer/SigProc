
cmake_minimum_required(VERSION 3.2)
project(sigproc)

set (CMAKE_CXX_STANDARD 11)

message("Build Profile: ${CMAKE_BUILD_TYPE} -- ${CMAKE_CXX_COMPILER_ID}")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/util")

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
message("not setting compiler flags for windows")

else()

set(CMAKE_CXX_FLAGS "-Wall -Wold-style-cast -Werror -fno-rtti -Wno-deprecated-declarations -Wno-unused-variable" CACHE INTERNAL "" FORCE)
if("${CMAKE_BUILD_TYPE}" MATCHES "Coverage")
  set(CMAKE_CXX_FLAGS "-DDEBUG ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "" FORCE)
  include(CMakeCover)
elseif("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "-DDEBUG  ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "" FORCE)
endif()

endif()


if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Qunused-arguments ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "" FORCE)
else()
    set(CMAKE_CXX_FLAGS "-Wno-error=unused-but-set-variable ${CMAKE_CXX_FLAGS}" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(sigproc)

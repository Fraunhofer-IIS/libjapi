cmake_minimum_required(VERSION 3.6)

project(libjapi VERSION 0.3)
set(SOVERSION 1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 'cmake -DCMAKE_BUILD_TYPE=Debug ../' from build folder for debug output #
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

##############################################
# Create shared and static libjapi libraries #

# configure libraries
find_package(PkgConfig)
pkg_search_module(JSONC REQUIRED IMPORTED_TARGET json-c)

# create an object lib to avoid double compilation
add_library(japi_objs OBJECT ${SOURCES} ${HEADERS})
target_compile_options(japi_objs PUBLIC "-pthread" "-D_POSIX_C_SOURCE=200809L")
target_include_directories(japi_objs PUBLIC include/ ${JSONC_INCLUDE_DIRS})
set_property(TARGET japi_objs PROPERTY POSITION_INDEPENDENT_CODE ON)

# set up actual libraries
add_library(japi        SHARED $<TARGET_OBJECTS:japi_objs>)
add_library(japi-static STATIC $<TARGET_OBJECTS:japi_objs>)

set_target_properties(japi PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  SOVERSION ${SOVERSION})

target_link_libraries(japi PkgConfig::JSONC)
target_link_libraries(japi-static PkgConfig::JSONC)

# only install libjapi.so because the static version is usually linked directly
# from the build directory
install(TARGETS japi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)

################################
# Create Doxygen documentation #

find_package(Doxygen)

if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxydir/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)

################################
# Create unit test excecutable #

add_subdirectory(googletest/googletest
                 EXCLUDE_FROM_ALL)

add_executable(testsuite
  EXCLUDE_FROM_ALL
    test/japi_test.cc
)

target_compile_options(testsuite PUBLIC "-pthread")

target_link_libraries(testsuite
  gtest_main
  japi
)

target_include_directories(testsuite
  PUBLIC
    include/
    src/
    ${JSONC_INCLUDE_DIRS}
)

add_custom_target(run_test COMMAND testsuite DEPENDS testsuite)

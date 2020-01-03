cmake_minimum_required(VERSION 3.5)

project(libjapi VERSION 0.3)
set(SOVERSION 1)

# configure libraries
find_package(PkgConfig)
pkg_search_module(JSONC REQUIRED json-c)


# header files to be installed
set(PUBLIC_HEADERS
    include/creadline.h
    include/japi_pushsrv.h
    include/japi_utils.h
    include/japi.h
    include/networking.h
    include/rw_n.h
  )

# the sources (including all header files)
set(SOURCES
    src/japi_utils.c
    src/japi.c
    src/japi_pushsrv.c
    src/rw_n.c
    src/japi_intern.h
    src/japi_pushsrv_intern.h
    src/prntdbg.h
    src/creadline.c
    src/networking.c

    ${PUBLIC_HEADERS}
  )

# create an object lib to avoid double compilation
add_library(japi_objs OBJECT ${SOURCES})
target_include_directories(japi_objs PUBLIC include/ ${JSONC_INCLUDE_DIRS})
set_property(TARGET japi_objs PROPERTY POSITION_INDEPENDENT_CODE ON)

# set up actual libraries
add_library(japi        SHARED $<TARGET_OBJECTS:japi_objs>)
add_library(japi-static STATIC $<TARGET_OBJECTS:japi_objs>)

set_target_properties(japi PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  SOVERSION ${SOVERSION})

# only install libjapi.so because the static version is usually linked directly
# from the build directory
install(TARGETS japi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)

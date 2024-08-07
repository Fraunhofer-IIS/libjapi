cmake_minimum_required(VERSION 3.14)

################################
# Create unit test excecutable #

enable_testing()

# Include GoogleTest in the project
include(FetchContent)
FetchContent_Declare(
googletest
GIT_REPOSITORY https://github.com/google/googletest.git
GIT_TAG release-1.8.0)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Define test executable
add_executable(testsuite japi_test.cc)

target_compile_options(testsuite PUBLIC "-pthread")

target_link_libraries(testsuite gtest_main japi)

# Include directories
target_include_directories(testsuite PUBLIC ../include/ ../src/
											${JSONC_INCLUDE_DIRS})

gtest_discover_tests(testsuite)

################################
# Test code coverage #
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	include(${CMAKE_SOURCE_DIR}/cmake/CodeCoverage.cmake)
	set(COVERAGE_EXCLUDES
		"doxydir"
		"/usr/include"
		"_deps/googletest"
		"test/*"
	)
	append_coverage_compiler_flags()
	setup_target_for_coverage_lcov(NAME coverage EXECUTABLE testsuite)
endif()

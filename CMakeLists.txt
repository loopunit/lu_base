cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)

# ---- Project ----

project(
  lu_base
  VERSION 0.0.0
  LANGUAGES C CXX)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

include(cmake/CPM.cmake)

find_package(CrossWindow MODULE REQUIRED)

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx")
	
file(GLOB_RECURSE sources CONFIGURE_DEPENDS 
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.mm")

# ---- Target ----

add_library(lu_base ${headers} ${sources})

target_compile_options(lu_base INTERFACE "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
target_compile_options(lu_base INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set_target_properties(lu_base PROPERTIES CXX_STANDARD 20)

target_compile_definitions(lu_base 
	PUBLIC
		SPDLOG_COMPILED_LIB=1)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(lu_base PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
if(LU_PLATFORM_WIN32 OR LU_PLATFORM_UNIVERSAL_WINDOWS)
	target_link_libraries(lu_base PUBLIC 
		Microsoft::DirectX-Headers
		Microsoft::DirectX-Guids
		userenv
		d3d12 dxcore dxgi
		CrossWindow)
#		d3dcompiler.lib
endif()

target_link_libraries(lu_base PUBLIC CrossWindow)

target_include_directories(lu_base 
	PUBLIC 
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/lu_base-0.0.0>
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/memory>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>)

add_library(loopunit::lu_base ALIAS lu_base)

if(ENABLE_TEST_COVERAGE)
  target_compile_options(lu_base PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(lu_base PUBLIC -fprofile-arcs -ftest-coverage)
endif()

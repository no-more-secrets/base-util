cmake_minimum_required( VERSION 3.12...3.12 )

if( ${CMAKE_VERSION} VERSION_LESS 3.12 )
    cmake_policy( VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} )
endif()

project(
    base-util
    VERSION 1.0
    DESCRIPTION "C++ Base [Utilities]"
    LANGUAGES CXX
)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Enable all warnings and treat warnings as errors.
function( set_warning_options target )
	target_compile_options(
        ${target} PRIVATE
        # clang/GCC warnings
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -Wextra -Wpedantic -Werror >
        # MSVC warnings
        $<$<CXX_COMPILER_ID:MSVC>:
            /Wall /WX > )
endfunction( set_warning_options )

# === clang-tidy ==================================================

find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)

if( NOT CLANG_TIDY_EXE )
    message( FATAL_ERROR "clang-tidy not found." )
else()
    #message( STATUS "clang-tidy found: ${CLANG_TIDY_EXE}" )
    set( DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-quiet" )
endif()

## If clang-tidy binary found, will enable it for the given target.
function( clang_tidy target )
    if( CLANG_TIDY_EXE )
        set_target_properties( ${target} PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif()
endfunction( clang_tidy )

# === ccache ======================================================

find_program( CCACHE_PROGRAM ccache )
if( CCACHE_PROGRAM )
    set( CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" )
    #message( STATUS "ccache found: ${CCACHE_PROGRAM}" )
else()
    message( STATUS "ccache not found." )
endif()

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

# === library =====================================================

add_library(
    base-util
    STATIC
    src/algo-par.cpp
    src/datetime.cpp
    src/fs.cpp
    src/io.cpp
    src/line-endings.cpp
    src/logger.cpp
    src/stopwatch.cpp
    src/string-util.cpp
    src/util.cpp
)

target_include_directories( base-util PUBLIC include )
target_compile_features( base-util PUBLIC cxx_std_17 )
set_target_properties( base-util PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( base-util PUBLIC stdc++fs )
target_link_libraries( base-util PUBLIC Threads::Threads )
set_warning_options( base-util )

# === main ========================================================

add_executable( main app/main.cpp )
target_compile_features( main PUBLIC cxx_std_17 )
set_target_properties( main PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( main PRIVATE base-util )
set_warning_options( main )

# === test ========================================================

add_executable(
    tests-all
    tests/common.cpp
    tests/fs.cpp
    tests/string.cpp
    tests/algo.cpp
    tests/misc.cpp
)
target_compile_features( tests-all PUBLIC cxx_std_17 )
set_target_properties( tests-all PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( tests-all PRIVATE base-util )
set_warning_options( tests-all )
add_test( tests-all tests-all )
enable_testing()

# === build type ==================================================

set( default_build_type "Debug" )
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
    message( STATUS "Setting build type to '${default_build_type}' as none was specified." )
    set( CMAKE_BUILD_TYPE "${default_build_type}" CACHE
         STRING "Choose the type of build." FORCE )
    # Set the possible values of build type for cmake-gui
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "Debug" "Release" "RelWithDebInfo")
endif()
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )

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

# === build type ==================================================

set( default_build_type "Debug" )
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
    message( STATUS "Setting build type to '${default_build_type}'." )
    set( CMAKE_BUILD_TYPE "${default_build_type}" CACHE
         STRING "Choose the type of build." FORCE )
    # Set the possible values of build type for cmake-gui
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "Debug" "Release" "RelWithDebInfo")
endif()
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )

# === subdirectories ==============================================

add_subdirectory( src )
add_subdirectory( app )
add_subdirectory( test )

# Should also automatically add a dependency on main.
add_custom_target( run
                   COMMAND main
                   WORKING_DIRECTORY ../
                   USES_TERMINAL )

# === clang-tools =================================================

if( USE_CLANG_TIDY )
    find_program( CLANG_TIDY_EXE NAMES "clang-tidy" )
    if( NOT CLANG_TIDY_EXE )
        message( FATAL_ERROR "clang-tidy not found." )
    endif()
    set( DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-quiet" )
    set_target_properties( base-util PROPERTIES
        CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif()

if( USE_IWYU )
    find_program( IWYU_EXE NAMES "include-what-you-use" )
    if( NOT IWYU_EXE )
        message( FATAL_ERROR "include-what-you-use requested, but not found." )
    endif()
    get_filename_component( REAL_IWYU_EXE "${IWYU_EXE}" REALPATH )
    set_target_properties( base-util PROPERTIES
        CXX_INCLUDE_WHAT_YOU_USE "${REAL_IWYU_EXE}" )
endif()

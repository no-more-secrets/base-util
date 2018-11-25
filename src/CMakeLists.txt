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

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

add_library(
    base-util
    STATIC
    algo-par.cpp
    conv.cpp
    datetime.cpp
    fs.cpp
    io.cpp
    line-endings.cpp
    logger.cpp
    net.cpp
    stopwatch.cpp
    string.cpp
    misc.cpp
)
add_library( base-util::base-util ALIAS base-util )

#target_include_directories( base-util PUBLIC include )
target_include_directories( base-util
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> )

target_compile_features( base-util PUBLIC cxx_std_17 )
set_target_properties( base-util PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( base-util PUBLIC stdc++fs )
target_link_libraries( base-util PUBLIC Threads::Threads )
set_warning_options( base-util )

install( TARGETS base-util
         EXPORT base-util-targets
         #LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         #RUNTIME DESTINATION bin
         INCLUDES DESTINATION include )

install( DIRECTORY include/base-util DESTINATION include )

include( CMakePackageConfigHelpers )
write_basic_package_version_file(
    base-util-config-version.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion )

install( EXPORT base-util-targets
         FILE base-util-targets.cmake
         NAMESPACE base-util::
         DESTINATION lib/cmake/base-util )

configure_file( ../cmake/base-util-config.cmake.in
                base-util-config.cmake
                @ONLY )
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/base-util-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/base-util-config-version.cmake"
    DESTINATION
        lib/cmake/base-util
)

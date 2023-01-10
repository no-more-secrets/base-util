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

target_include_directories( base-util
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> )

target_compile_features( base-util PUBLIC cxx_std_20 )
set_target_properties( base-util PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( base-util PUBLIC ${cxx-fs-lib} )
target_link_libraries( base-util PUBLIC Threads::Threads )

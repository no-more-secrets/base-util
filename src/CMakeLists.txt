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

# TODO: Do all these need to be PUBLIC?

target_include_directories( base-util PUBLIC include )
target_compile_features( base-util PUBLIC cxx_std_17 )
set_target_properties( base-util PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( base-util PUBLIC stdc++fs )
target_link_libraries( base-util PUBLIC Threads::Threads )
set_warning_options( base-util )

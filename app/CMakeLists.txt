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

add_executable( main main.cpp )
target_compile_features( main PUBLIC cxx_std_17 )
set_target_properties( main PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( main PRIVATE base-util )
set_warning_options( main )

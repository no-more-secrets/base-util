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

enable_testing()

find_package( Catch2 REQUIRED )

add_executable( tests
    main.cpp
    algo.cpp
    conv.cpp
    fs.cpp
    misc.cpp
    net.cpp
    string.cpp
)

target_compile_features( tests PUBLIC cxx_std_17 )
set_target_properties( tests PROPERTIES CXX_EXTENSIONS OFF )
add_test( tests tests )
set_warning_options( tests )
target_link_libraries( tests
                       PRIVATE
                       base-util::base-util
                       Catch2::Catch2 )

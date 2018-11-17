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

add_executable(
    tests-all
    common.cpp
    conv.cpp
    fs.cpp
    net.cpp
    string.cpp
    algo.cpp
    misc.cpp
)
target_compile_features( tests-all PUBLIC cxx_std_17 )
set_target_properties( tests-all PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( tests-all PRIVATE base-util )
set_warning_options( tests-all )
add_test( tests-all tests-all )
enable_testing()

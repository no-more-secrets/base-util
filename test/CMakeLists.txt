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

add_executable(
    tests-all
    common.cpp
    fs.cpp
)
target_compile_features( tests-all PUBLIC cxx_std_17 )
set_target_properties( tests-all PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( tests-all PRIVATE base-util )
set_warning_options( tests-all )
add_test( tests-all tests-all )

find_package( Catch2 REQUIRED )

add_executable( tests-catch
    main.cpp
    algo.cpp
    conv.cpp
    misc.cpp
    net.cpp
    string.cpp
)

target_compile_features( tests-catch PUBLIC cxx_std_17 )
set_target_properties( tests-catch PROPERTIES CXX_EXTENSIONS OFF )
add_test( tests-catch tests-catch )
set_warning_options( tests-catch )
target_link_libraries( tests-catch
                       PRIVATE
                       base-util::base-util
                       Catch2::Catch2 )

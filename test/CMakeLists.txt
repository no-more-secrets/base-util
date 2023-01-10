enable_testing()

add_executable( run-tests
    infra/main.cpp
    algo.cpp
    conv.cpp
    fs.cpp
    misc.cpp
    net.cpp
    string.cpp
)

target_compile_features( run-tests PUBLIC cxx_std_20 )
set_target_properties( run-tests PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( run-tests
                       PRIVATE
                       base-util::base-util
                       Catch2 )

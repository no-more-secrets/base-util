set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

add_executable( main main.cpp )
target_compile_features( main PUBLIC cxx_std_17 )
set_target_properties( main PROPERTIES CXX_EXTENSIONS OFF )
target_link_libraries( main PRIVATE base-util )

find_package(Threads REQUIRED)
add_executable(tree_algorithms tree_algorithms.cpp)
target_link_libraries(tree_algorithms benchmark ${CMAKE_THREAD_LIBS_INIT} Boost::headers)

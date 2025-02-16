cmake_minimum_required(VERSION 3.25)
project(Boost.Graph)

enable_testing()

# set(CMAKE_CXX_STANDARD 03)
set(Boost_NO_SYSTEM_PATHS ON)
find_package(Boost REQUIRED unit_test_framework)

add_subdirectory(test)
add_subdirectory(benchmark)

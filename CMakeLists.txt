cmake_minimum_required(VERSION 3.12)
project(Boost.Graph)

enable_testing()

# set(CMAKE_CXX_STANDARD 03)
find_package(Boost REQUIRED unit_test_framework)

# Yes, I'm cheating.
include_directories(${Boost_INCLUDE_DIRS})
add_subdirectory(test)
add_subdirectory(benchmark)

# Copyright 2018, 2019, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(boost_graph_subdir_test LANGUAGES CXX)

if(BOOST_RUNTIME_LINK STREQUAL "static")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Put .dll in the same directory as .exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(BOOST_INCLUDE_LIBRARIES graph)

add_subdirectory(../../../../ deps/boost EXCLUDE_FROM_ALL)

add_executable(main main.cpp)
target_link_libraries(main Boost::graph)

enable_testing()
add_test(NAME main COMMAND main)

add_custom_target(check VERBATIM COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --no-tests=error -C $<CONFIG>)

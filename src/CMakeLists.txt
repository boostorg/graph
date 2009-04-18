# Copyright (C) 2008 Michael Jackson
#
# Use, modification and distribution is subject to the Boost Software
# License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

add_definitions(-DBOOST_GRAPH_NO_LIB=1)

if (MSVC)
  # Without these flags, MSVC 7.1 and 8.0 crash
  add_definitions(-GR-)
endif (MSVC)

set(BOOST_GRAPH_OPTIONAL_SOURCES "")
set(BOOST_GRAPH_OPTIONAL_LIBRARIES "")

# Try to find the Expat library 
include(FindEXPAT)
if (EXPAT_FOUND)
  # We have Expat, so build the GraphML parser - TODO: Boost 1.34.x only
#  set(BOOST_GRAPH_OPTIONAL_SOURCES 
#    ${BOOST_GRAPH_OPTIONAL_SOURCES} "graphml.cpp")
  include_directories(${EXPAT_INCLUDE_DIRS})
  list(APPEND BOOST_GRAPH_OPTIONAL_LIBRARIES ${EXPAT_LIBRARIES})
endif (EXPAT_FOUND)

boost_add_library(
  boost_graph
  read_graphviz_spirit.cpp ${BOOST_GRAPH_OPTIONAL_SOURCES}
  LINK_LIBS ${BOOST_GRAPH_OPTIONAL_LIBRARIES}
  SHARED_COMPILE_FLAGS "-DBOOST_GRAPH_DYN_LINK=1"
  )


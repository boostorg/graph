// Copyright (C) 2006 Trustees of Indiana University
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

struct vertex_properties {
  std::string name;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & name;
  }  
};

struct edge_properties {
  std::string name;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & name;
  }  
};

using namespace boost;

typedef adjacency_list<vecS, vecS, undirectedS, 
               vertex_properties, edge_properties> Graph;


typedef adjacency_list<vecS, vecS, undirectedS, 
               vertex_properties> Graph_no_edge_property;

int main()
{
  {
    std::ofstream ofs("./kevin-bacon2.dat");
    archive::text_oarchive oa(ofs);
    Graph g;
    oa << g;

    Graph_no_edge_property g_n;
    oa << g_n;
  }

  {
    std::ifstream ifs("./kevin-bacon2.dat");
    archive::text_iarchive ia(ifs);
    Graph g;
    ia >> g;

    Graph_no_edge_property g_n;
    ia >> g_n;
  }
  return 0;
}

// Copyright Jeremy Siek 2004

// This file was an experiment to try supporting operator-> to access
// vertex and edge properties from a vertex or descriptor.  The
// experiment was abandoned because of const issues.

#include <string>
#include <iostream>
#include <boost/graph/digraph.hpp>
#include <boost/graph/graph_concepts.hpp>

struct city;
struct road;

typedef boost::digraph<city, road> graph_t;
struct city {
  city(std::string n) : name(n) { }
  std::string name;
};
struct road {
  unsigned int length;
};

int main()
{
  graph_t g;
  city jersey("New Jersey");

  graph_t::vertex_descriptor v = add_vertex(jersey, g);
  std::cout << "v->name: " << v->name 
            << std::endl
            << "v.index(): " << v.index()
            << std::endl;

  boost::function_requires<boost::IncidenceGraphConcept<graph_t> >();
  boost::function_requires<boost::AdjacencyGraphConcept<graph_t> >();
  
  return 0;
}

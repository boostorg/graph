#include <string>
#include <iostream>
#include <boost/graph/digraph.hpp>

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
  std::cout << "v->name: " << v->name << std::endl;
  
  return 0;
}

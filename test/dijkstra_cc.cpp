#include <boost/concept_archetype.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_archetypes.hpp>

typedef boost::default_constructible_archetype<
  boost::sgi_assignable_archetype<> > dist_value;

// So generate_infinity works...
namespace std {
  template <>
  class numeric_limits<dist_value> {
  public:
    static dist_value max() {
      return boost::static_object<dist_value>::get(); 
    }
  };
}

int main()
{
  using namespace boost;
  typedef default_constructible_archetype< 
    sgi_assignable_archetype<
    equality_comparable_archetype<> > > vertex_t;
  {
    typedef vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> graph_t;
    graph_t g;
    vertex_t s;
    typedef graph_traits<graph_t>::edge_descriptor edge_t;
    readable_property_map_archetype<edge_t, std::size_t> weight;
    readable_property_map_archetype<vertex_t, int> index;
    read_write_property_map_archetype<vertex_t, std::size_t> distance;
    dijkstra_shortest_paths(g, s, 
			    vertex_index_map(index).
			    weight_map(weight).
			    distance_map(distance));
  }
  {
    typedef vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> graph_t;
    vertex_t s;
    typedef graph_traits<graph_t>::edge_descriptor edge_t;
    readable_property_map_archetype<edge_t, std::size_t> weight;
    property_graph_archetype<graph_t, vertex_index_t, std::size_t> g;
    read_write_property_map_archetype<vertex_t, vertex_t> pred;
    dijkstra_shortest_paths(g, s,
			    predecessor_map(pred).
			    weight_map(weight));
  }
  {
    typedef vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> graph_t;
    vertex_t s;
    property_graph_archetype<graph_t, edge_weight_t, std::size_t> g;
    read_write_property_map_archetype<vertex_t, vertex_t> pred;
    readable_property_map_archetype<vertex_t, int> index;
    dijkstra_shortest_paths(g, s,
			    predecessor_map(pred).
			    vertex_index_map(index));
  }
  {
    typedef vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> graph_t;
    graph_t g;
    vertex_t s;
    typedef graph_traits<graph_t>::edge_descriptor edge_t;
    readable_property_map_archetype<edge_t, dist_value> weight;
    readable_property_map_archetype<vertex_t, int> index;
    read_write_property_map_archetype<vertex_t, color_value_archetype> color;
    read_write_property_map_archetype<vertex_t, dist_value> distance;
    binary_function_archetype<dist_value, dist_value, dist_value> 
      combine(dummy_cons);
    binary_predicate_archetype<dist_value, dist_value> 
      compare(dummy_cons);
    dijkstra_visitor<> vis;

    dijkstra_shortest_paths(g, s, color_map(color).
			    vertex_index_map(index).
			    weight_map(weight).
			    distance_map(distance).
			    distance_combine(combine).
			    distance_compare(compare).
			    visitor(vis));
  }
  return 0;
}

#include <boost/concept_archetype.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_archetypes.hpp>

int main()
{
  using namespace boost;
  typedef default_constructible_archetype< 
    sgi_assignable_archetype<
    equality_comparable_archetype<> > > vertex_t;
  {
    vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> g;
    read_write_property_map_archetype<vertex_t, color_value_archetype> color;
    depth_first_search(g, color_map(color));
  }
  {
    vertex_list_graph_archetype<vertex_t, directed_tag, 
      allow_parallel_edge_tag> g;
    readable_property_map_archetype<vertex_t, std::size_t> v_index;
    depth_first_search(g, vertex_index_map(v_index));
  }
  {
    typedef vertex_list_graph_archetype<vertex_t, undirected_tag, 
      allow_parallel_edge_tag> Graph;
    property_graph_archetype<Graph, vertex_index_t, std::size_t> g;
    dfs_visitor<> v;
    depth_first_search(g, visitor(v));
  }
  return 0;
}

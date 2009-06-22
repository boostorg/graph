#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/mcgregor_common_subgraphs.hpp>
#include <boost/property_map/shared_array_property_map.hpp>

using namespace boost;

bool was_common_subgraph_found = false, output_graphs = false;

// Callback that compares incoming graphs to the supplied common
// subgraph.
template <typename Graph>
struct test_callback {

   test_callback(Graph& common_subgraph,
                 const Graph& graph1,
                 const Graph& graph2) :                 
    m_graph1(graph1),
    m_graph2(graph2),
    m_common_subgraph(common_subgraph) { }

  template <typename CorrespondenceMapFirstToSecond,
            typename CorrespondenceMapSecondToFirst>
  bool operator()(CorrespondenceMapFirstToSecond correspondence_map_1_to_2,
                  CorrespondenceMapSecondToFirst correspondence_map_2_to_1,
                  typename graph_traits<Graph>::vertices_size_type subgraph_size) {

    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    typedef std::pair<Edge, bool> EdgeInfo;

    typedef typename property_map<Graph, vertex_index_t>::type VertexIndexMap;
    typedef typename property_map<Graph, vertex_name_t>::type VertexNameMap;
    typedef typename property_map<Graph, edge_name_t>::type EdgeNameMap;

    if (subgraph_size != num_vertices(m_common_subgraph)) {
      return (true);
    }

    // Fill membership maps for both graphs
    typedef shared_array_property_map<bool, VertexIndexMap> MembershipMap;
      
    MembershipMap membership_map1(num_vertices(m_graph1),
                                  get(vertex_index, m_graph1));

    MembershipMap membership_map2(num_vertices(m_graph2),
                                  get(vertex_index, m_graph2));

    fill_membership_maps(m_graph1, m_graph2,
                         correspondence_map_1_to_2, correspondence_map_2_to_1,
                         membership_map1, membership_map2);

    // Generate filtered graphs using membership maps
    typedef typename membership_filtered_graph_traits<Graph, MembershipMap>::graph_type
      MembershipFilteredGraph;

    MembershipFilteredGraph subgraph1 =
      make_membership_filtered_graph(m_graph1, membership_map1);

    MembershipFilteredGraph subgraph2 =
      make_membership_filtered_graph(m_graph2, membership_map2);

    VertexIndexMap vindex_map1 = get(vertex_index, subgraph1);
    VertexIndexMap vindex_map2 = get(vertex_index, subgraph2);

    VertexNameMap vname_map_common = get(vertex_name, m_common_subgraph);
    VertexNameMap vname_map1 = get(vertex_name, subgraph1);
    VertexNameMap vname_map2 = get(vertex_name, subgraph2);

    EdgeNameMap ename_map_common = get(edge_name, m_common_subgraph);
    EdgeNameMap ename_map1 = get(edge_name, subgraph1);
    EdgeNameMap ename_map2 = get(edge_name, subgraph2);

    // Verify that subgraph1 matches the supplied common subgraph
    BGL_FORALL_VERTICES_T(vertex1, subgraph1, MembershipFilteredGraph) {
      
      Vertex vertex_common = vertex(get(vindex_map1, vertex1), m_common_subgraph);

      // Match vertex names
      if (get(vname_map_common, vertex_common) != get(vname_map1, vertex1)) {

        // Keep looking
        return (true);

      }

      BGL_FORALL_VERTICES_T(vertex1_2, subgraph1, MembershipFilteredGraph) {

        Vertex vertex_common2 = vertex(get(vindex_map1, vertex1_2), m_common_subgraph);
        EdgeInfo edge_common = edge(vertex_common, vertex_common2, m_common_subgraph);
        EdgeInfo edge1 = edge(vertex1, vertex1_2, subgraph1);

        if ((edge_common.second != edge1.second) ||
            ((edge_common.second && edge1.second) &&
             (get(ename_map_common, edge_common.first) != get(ename_map1, edge1.first)))) {

          // Keep looking
          return (true);

        }
      }  
    
    } // BGL_FORALL_VERTICES_T (subgraph1)

    // Verify that subgraph2 matches the supplied common subgraph
    BGL_FORALL_VERTICES_T(vertex2, subgraph2, MembershipFilteredGraph) {
      
      Vertex vertex_common = vertex(get(vindex_map2, vertex2), m_common_subgraph);

      // Match vertex names
      if (get(vname_map_common, vertex_common) != get(vname_map2, vertex2)) {

        // Keep looking
        return (true);

      }

      BGL_FORALL_VERTICES_T(vertex2_2, subgraph2, MembershipFilteredGraph) {

        Vertex vertex_common2 = vertex(get(vindex_map2, vertex2_2), m_common_subgraph);
        EdgeInfo edge_common = edge(vertex_common, vertex_common2, m_common_subgraph);
        EdgeInfo edge2 = edge(vertex2, vertex2_2, subgraph2);

        if ((edge_common.second != edge2.second) ||
            ((edge_common.second && edge2.second) &&
             (get(ename_map_common, edge_common.first) != get(ename_map2, edge2.first)))) {

          // Keep looking
          return (true);

        }
      }  
    
    } // BGL_FORALL_VERTICES_T (subgraph2)
    
    // Check isomorphism just to be thorough
    if (verify_isomorphism(subgraph1, subgraph2, correspondence_map_1_to_2)) {

      was_common_subgraph_found = true;

      if (output_graphs) {

        std::fstream file_subgraph("found_common_subgraph.dot", std::fstream::out);
        write_graphviz(file_subgraph, subgraph1,
                       make_label_writer(get(vertex_name, m_graph1)),
                       make_label_writer(get(edge_name, m_graph1)));

      }

      // Stop iterating
      return (false);

    }
    
    // Keep looking
    return (true);
  }

private:
  const Graph& m_graph1, m_graph2;
  Graph& m_common_subgraph;
};

template <typename Graph,
          typename RandomNumberGenerator,
          typename VertexNameMap,
          typename EdgeNameMap>
void add_random_vertices(Graph& graph, RandomNumberGenerator& generator,
                         int vertices_to_create, int max_edges_per_vertex,
                         VertexNameMap vname_map, EdgeNameMap ename_map) {

  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::vector<Vertex> VertexList;

  VertexList new_vertices;

  for (int v_index = 0; v_index < vertices_to_create; ++v_index) {

    Vertex new_vertex = add_vertex(graph);
    put(vname_map, new_vertex, generator());
    new_vertices.push_back(new_vertex);

  }

  // Add edges for every new vertex. Care is taken to avoid parallel
  // edges.
  for (typename VertexList::const_iterator v_iter = new_vertices.begin();
       v_iter != new_vertices.end(); ++v_iter) {

    Vertex source_vertex = *v_iter;
    int edges_for_vertex = std::min((generator() % max_edges_per_vertex) + 1,
                                    (int)num_vertices(graph));

    while (edges_for_vertex > 0) {

      Vertex target_vertex = random_vertex(graph, generator);

      if (source_vertex == target_vertex) {
        continue;
      }

      BGL_FORALL_OUTEDGES_T(source_vertex, edge, graph, Graph) {
        if (target(edge, graph) == target_vertex) {
          continue;
        }
      }

      put(ename_map, add_edge(source_vertex, target_vertex, graph).first,
          generator());

      edges_for_vertex--;
    }
  }
}

int test_main (int argc, char *argv[]) {
  int vertices_to_create = 10;
  int max_edges_per_vertex = 2;
  std::size_t random_seed = time(0);
  
  if (argc > 1) {
    vertices_to_create = lexical_cast<int>(argv[1]);
  }
  
  if (argc > 2) {
    max_edges_per_vertex = lexical_cast<int>(argv[2]);
  }

  if (argc > 3) {
    output_graphs = lexical_cast<bool>(argv[3]);
  }
  
  if (argc > 4) {
    random_seed = lexical_cast<std::size_t>(argv[4]);
  }
  
  minstd_rand generator(random_seed);

  // Using a vecS graph here so that we don't have to mess around with
  // a vertex index map; it will be implicit.
  typedef adjacency_list<listS, vecS, directedS,
    property<vertex_name_t, unsigned int,
    property<vertex_index_t, unsigned int> >,
    property<edge_name_t, unsigned int> > Graph;

  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef graph_traits<Graph>::edge_descriptor Edge;

  typedef property_map<Graph, vertex_name_t>::type VertexNameMap;
  typedef property_map<Graph, edge_name_t>::type EdgeNameMap;

  // Generate a random common subgraph and then add random vertices
  // and edges to the two parent graphs.
  Graph common_subgraph, graph1, graph2;

  VertexNameMap vname_map_common = get(vertex_name, common_subgraph);
  VertexNameMap vname_map1 = get(vertex_name, graph1);
  VertexNameMap vname_map2 = get(vertex_name, graph2);

  EdgeNameMap ename_map_common = get(edge_name, common_subgraph);
  EdgeNameMap ename_map1 = get(edge_name, graph1);
  EdgeNameMap ename_map2 = get(edge_name, graph2);

  for (int vindex = 0; vindex < vertices_to_create; ++vindex) {
    put(vname_map_common, add_vertex(common_subgraph), generator());
  }

  BGL_FORALL_VERTICES(source_vertex, common_subgraph, Graph) {

    BGL_FORALL_VERTICES(target_vertex, common_subgraph, Graph) {
      
      if (source_vertex != target_vertex) {
        put(ename_map_common,
            add_edge(source_vertex, target_vertex, common_subgraph).first,
            generator());
      }
    }
  }

  randomize_property<vertex_name_t>(common_subgraph, generator);
  randomize_property<edge_name_t>(common_subgraph, generator);

  copy_graph(common_subgraph, graph1);
  copy_graph(common_subgraph, graph2);

  // Randomly add vertices and edges to graph1 and graph2.
  add_random_vertices(graph1, generator, vertices_to_create,
                      max_edges_per_vertex, vname_map1, ename_map1);

  add_random_vertices(graph2, generator, vertices_to_create,
                      max_edges_per_vertex, vname_map2, ename_map2);

  if (output_graphs) {

    std::fstream file_graph1("graph1.dot", std::fstream::out),
      file_graph2("graph2.dot", std::fstream::out),
      file_common_subgraph("expected_common_subgraph.dot", std::fstream::out);
               
    write_graphviz(file_graph1, graph1,
                   make_label_writer(vname_map1),
                   make_label_writer(ename_map1));

    write_graphviz(file_graph2, graph2,
                   make_label_writer(vname_map2),
                   make_label_writer(ename_map2));

    write_graphviz(file_common_subgraph, common_subgraph,
                   make_label_writer(get(vertex_name, common_subgraph)),
                   make_label_writer(get(edge_name, common_subgraph)));

  }

  std::cout << "Searching for common subgraph of size " <<
    num_vertices(common_subgraph) << std::endl;

  test_callback<Graph> user_callback(common_subgraph, graph1, graph2);

  mcgregor_common_subgraphs(graph1, graph2, user_callback,
    edges_equivalent(make_property_map_equivalent(ename_map1, ename_map2)).
    vertices_equivalent(make_property_map_equivalent(vname_map1, vname_map2)));

  BOOST_CHECK(was_common_subgraph_found);

  return 0;
}

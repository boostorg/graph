//=======================================================================
// Copyright 2007 Aaron Windsor
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>

using namespace boost;

template < typename Graph > void reset_edge_index(Graph& g)
{
    typename property_map< Graph, edge_index_t >::type index
        = get(edge_index, g);
    typename graph_traits< Graph >::edge_iterator ei, ei_end;
    typename graph_traits< Graph >::edges_size_type cnt = 0;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
        put(index, *ei, cnt++);
}

template < typename Graph > void make_line_graph(Graph& g, int size)
{
    typedef typename graph_traits< Graph >::vertex_descriptor vertex_t;

    vertex_t prev_vertex = add_vertex(g);

    for (int i = 1; i < size; ++i)
    {
        vertex_t curr_vertex = add_vertex(g);
        add_edge(curr_vertex, prev_vertex, g);
        prev_vertex = curr_vertex;
    }
}

struct UpdateVertexIndex
{
    template < typename Graph > void update(Graph& g)
    {
        typename property_map< Graph, vertex_index_t >::type index
            = get(vertex_index, g);
        typename graph_traits< Graph >::vertex_iterator vi, vi_end;
        typename graph_traits< Graph >::vertices_size_type cnt = 0;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
            put(index, *vi, cnt++);
    }
};

struct NoVertexIndexUpdater
{
    template < typename Graph > void update(Graph&) {}
};

template < typename Graph, typename VertexIndexUpdater >
void test_line_graph(VertexIndexUpdater vertex_index_updater, int size)
{

    Graph g;
    make_line_graph(g, size);
    vertex_index_updater.update(g);
    reset_edge_index(g);

    typedef std::vector< typename graph_traits< Graph >::edge_descriptor >
        edge_vector_t;
    typedef std::vector< edge_vector_t > embedding_storage_t;
    typedef iterator_property_map< typename embedding_storage_t::iterator,
        typename property_map< Graph, vertex_index_t >::type >
        embedding_t;

    embedding_storage_t embedding_storage(num_vertices(g));
    embedding_t embedding(embedding_storage.begin(), get(vertex_index, g));

    typename graph_traits< Graph >::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        std::copy(out_edges(*vi, g).first, out_edges(*vi, g).second,
            std::back_inserter(embedding[*vi]));

    BOOST_TEST(biconnected_components(
                    g, make_vector_property_map< int >(get(edge_index, g)))
        > 1);
    BOOST_TEST(boyer_myrvold_planarity_test(g));
    make_biconnected_planar(g, embedding);
    reset_edge_index(g);
    BOOST_TEST(biconnected_components(
                    g, make_vector_property_map< int >(get(edge_index, g)))
        == 1);
    BOOST_TEST(boyer_myrvold_planarity_test(g));
}

// records the joins and still adds the edges the algorithm expects
struct pair_tally
{
    template < typename Vertex, typename Graph >
    void visit_vertex_pair(Vertex u, Vertex v, Graph& g)
    {
        ::boost::add_edge(u, v, g);
        ++count;
    }
    std::size_t count = 0;
};

// a std::ref wrapped visitor keeps its count, a temporary just compiles
template < typename Graph > void test_visitor_overloads()
{
    Graph g;
    make_line_graph(g, 10);
    reset_edge_index(g);

    using edge_vector_t = std::vector< typename ::boost::graph_traits< Graph >::edge_descriptor >;
    using embedding_storage_t = std::vector< edge_vector_t >;

    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);

    embedding_storage_t embedding_storage(::boost::num_vertices(g));
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);

    auto vertex_range = ::boost::vertices(g);
    for (auto vi = vertex_range.first; vi != vertex_range.second; ++vi)
    {
        auto out_edge_range = ::boost::out_edges(*vi, g);
        std::copy(out_edge_range.first, out_edge_range.second, std::back_inserter(embedding_map[*vi]));
    }

    pair_tally tally;
    ::boost::make_biconnected_planar(g, embedding_map, edge_index_map, std::ref(tally));
    reset_edge_index(g);

    auto component_map = ::boost::make_vector_property_map< int >(edge_index_map);
    BOOST_TEST(tally.count > 0u);
    BOOST_TEST_EQ(::boost::biconnected_components(g, component_map), 1u);
    BOOST_TEST(::boost::boyer_myrvold_planarity_test(g));
}

int main(int, char*[])
{
    typedef adjacency_list< vecS, vecS, undirectedS,
        property< vertex_index_t, int >, property< edge_index_t, int > >
        VVgraph_t;

    typedef adjacency_list< vecS, listS, undirectedS,
        property< vertex_index_t, int >, property< edge_index_t, int > >
        VLgraph_t;

    typedef adjacency_list< listS, vecS, undirectedS,
        property< vertex_index_t, int >, property< edge_index_t, int > >
        LVgraph_t;

    typedef adjacency_list< listS, listS, undirectedS,
        property< vertex_index_t, int >, property< edge_index_t, int > >
        LLgraph_t;

    typedef adjacency_list< setS, setS, undirectedS,
        property< vertex_index_t, int >, property< edge_index_t, int > >
        SSgraph_t;

    test_line_graph< VVgraph_t >(NoVertexIndexUpdater(), 10);
    test_line_graph< VVgraph_t >(NoVertexIndexUpdater(), 50);

    test_line_graph< VLgraph_t >(UpdateVertexIndex(), 3);
    test_line_graph< VLgraph_t >(UpdateVertexIndex(), 30);

    test_line_graph< LVgraph_t >(NoVertexIndexUpdater(), 15);
    test_line_graph< LVgraph_t >(NoVertexIndexUpdater(), 45);

    test_line_graph< LLgraph_t >(UpdateVertexIndex(), 8);
    test_line_graph< LLgraph_t >(UpdateVertexIndex(), 19);

    test_line_graph< SSgraph_t >(UpdateVertexIndex(), 13);
    test_line_graph< SSgraph_t >(UpdateVertexIndex(), 20);

    test_visitor_overloads< VVgraph_t >();

    return boost::report_errors();
}

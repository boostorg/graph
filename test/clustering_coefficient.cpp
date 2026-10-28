// (C) Copyright 2007-2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/clustering_coefficient.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace std;
using namespace boost;

// number of vertices in the graph
static const unsigned N = 5;

template < typename Graph > struct vertex_vector
{
    typedef graph_traits< Graph > traits;
    typedef vector< typename traits::vertex_descriptor > type;
};

template < typename Graph >
void build_graph(Graph& g, typename vertex_vector< Graph >::type& v)
{
    // add vertices
    for (size_t i = 0; i < N; ++i)
    {
        v[i] = add_vertex(g);
    }

    // add edges
    add_edge(v[0], v[1], g);
    add_edge(v[1], v[2], g);
    add_edge(v[2], v[0], g);
    add_edge(v[3], v[4], g);
    add_edge(v[4], v[0], g);
}

template < typename Graph > void test_undirected()
{
    typedef typename graph_traits< Graph >::vertex_descriptor Vertex;

    typedef exterior_vertex_property< Graph, double > ClusteringProperty;
    typedef typename ClusteringProperty::container_type ClusteringContainer;
    typedef typename ClusteringProperty::map_type ClusteringMap;

    Graph g;
    vector< Vertex > v(N);
    build_graph(g, v);

    ClusteringContainer cc(num_vertices(g));
    ClusteringMap cm(cc, g);

    BOOST_ASSERT(num_paths_through_vertex(g, v[0]) == 3);
    BOOST_ASSERT(num_paths_through_vertex(g, v[1]) == 1);
    BOOST_ASSERT(num_paths_through_vertex(g, v[2]) == 1);
    BOOST_ASSERT(num_paths_through_vertex(g, v[3]) == 0);
    BOOST_ASSERT(num_paths_through_vertex(g, v[4]) == 1);

    BOOST_ASSERT(num_triangles_on_vertex(g, v[0]) == 1);
    BOOST_ASSERT(num_triangles_on_vertex(g, v[1]) == 1);
    BOOST_ASSERT(num_triangles_on_vertex(g, v[2]) == 1);
    BOOST_ASSERT(num_triangles_on_vertex(g, v[3]) == 0);
    BOOST_ASSERT(num_triangles_on_vertex(g, v[4]) == 0);

    // TODO: Need a FP approximation to assert here.
    // BOOST_ASSERT(clustering_coefficient(g, v[0]) == double(1)/3);
    BOOST_ASSERT(clustering_coefficient(g, v[1]) == 1);
    BOOST_ASSERT(clustering_coefficient(g, v[2]) == 1);
    BOOST_ASSERT(clustering_coefficient(g, v[3]) == 0);
    BOOST_ASSERT(clustering_coefficient(g, v[4]) == 0);

    all_clustering_coefficients(g, cm);

    // TODO: Need a FP approximation to assert here.
    // BOOST_ASSERT(cm[v[0]] == double(1)/3);
    BOOST_ASSERT(cm[v[1]] == 1);
    BOOST_ASSERT(cm[v[2]] == 1);
    BOOST_ASSERT(cm[v[3]] == 0);
    BOOST_ASSERT(cm[v[4]] == 0);

    // I would have used check_close, but apparently, that requires
    // me to link this against a library - which I don't really want
    // to do. Basically, this makes sure that that coefficient is
    // within some tolerance (like 1/10 million).
    double coef = mean_clustering_coefficient(g, cm);
    BOOST_ASSERT((coef - (7.0f / 15.0f)) < 1e-7f);
}

// Checks a directed graph where vertex 0 sees a reciprocal pair and vertex 3 has no out edges.
template < typename Graph > void test_directed()
{
    Graph g;
    std::vector< typename ::boost::graph_traits< Graph >::vertex_descriptor > v;
    for (std::size_t i = 0; i < 4; ++i)
    {
        v.push_back(::boost::add_vertex(g));
    }
    ::boost::add_edge(v[0], v[1], g);
    ::boost::add_edge(v[0], v[2], g);
    ::boost::add_edge(v[1], v[2], g);
    ::boost::add_edge(v[2], v[1], g);
    ::boost::add_edge(v[1], v[3], g);
    ::boost::add_edge(v[2], v[3], g);

    BOOST_TEST_EQ(::boost::num_paths_through_vertex(g, v[0]), 2u);
    BOOST_TEST_EQ(::boost::num_paths_through_vertex(g, v[1]), 2u);
    BOOST_TEST_EQ(::boost::num_paths_through_vertex(g, v[2]), 2u);
    BOOST_TEST_EQ(::boost::num_paths_through_vertex(g, v[3]), 0u);

    BOOST_TEST_EQ(::boost::num_triangles_on_vertex(g, v[0]), 2u);
    BOOST_TEST_EQ(::boost::num_triangles_on_vertex(g, v[1]), 1u);
    BOOST_TEST_EQ(::boost::num_triangles_on_vertex(g, v[2]), 1u);
    BOOST_TEST_EQ(::boost::num_triangles_on_vertex(g, v[3]), 0u);

    using clustering_property = ::boost::exterior_vertex_property< Graph, double >;
    typename clustering_property::container_type clustering(::boost::num_vertices(g));
    typename clustering_property::map_type clustering_map(clustering, g);
    BOOST_TEST_EQ(::boost::all_clustering_coefficients(g, clustering_map), 0.5);
    BOOST_TEST_EQ(clustering_map[v[0]], 1.0);
    BOOST_TEST_EQ(clustering_map[v[1]], 0.5);
    BOOST_TEST_EQ(clustering_map[v[2]], 0.5);
    BOOST_TEST_EQ(clustering_map[v[3]], 0.0);
    BOOST_TEST_EQ(::boost::mean_clustering_coefficient(g, clustering_map), 0.5);
}

// Wraps an adjacency_list without providing edge(), so lookup_edge must scan out edges.
template < typename Directed > struct no_edge_graph
{
    using base_type = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, Directed >;
    using base_traits = ::boost::graph_traits< base_type >;
    using vertex_descriptor = typename base_traits::vertex_descriptor;
    using edge_descriptor = typename base_traits::edge_descriptor;
    using adjacency_iterator = typename base_traits::adjacency_iterator;
    using out_edge_iterator = typename base_traits::out_edge_iterator;
    using degree_size_type = typename base_traits::degree_size_type;
    using directed_category = typename base_traits::directed_category;
    using edge_parallel_category = typename base_traits::edge_parallel_category;

    // Incidence and adjacency only, no adjacency matrix.
    struct traversal_category : ::boost::incidence_graph_tag, ::boost::adjacency_graph_tag
    {
    };

    const base_type* base = nullptr;
};

// Forwards out_edges to the wrapped graph.
template < typename Directed >
std::pair< typename no_edge_graph< Directed >::out_edge_iterator, typename no_edge_graph< Directed >::out_edge_iterator > out_edges(typename no_edge_graph< Directed >::vertex_descriptor v, const no_edge_graph< Directed >& g)
{
    return ::boost::out_edges(v, *g.base);
}

// Forwards out_degree to the wrapped graph.
template < typename Directed >
typename no_edge_graph< Directed >::degree_size_type out_degree(typename no_edge_graph< Directed >::vertex_descriptor v, const no_edge_graph< Directed >& g)
{
    return ::boost::out_degree(v, *g.base);
}

// Forwards adjacent_vertices to the wrapped graph.
template < typename Directed >
std::pair< typename no_edge_graph< Directed >::adjacency_iterator, typename no_edge_graph< Directed >::adjacency_iterator > adjacent_vertices(typename no_edge_graph< Directed >::vertex_descriptor v, const no_edge_graph< Directed >& g)
{
    return ::boost::adjacent_vertices(v, *g.base);
}

// Forwards source to the wrapped graph.
template < typename Directed >
typename no_edge_graph< Directed >::vertex_descriptor source(typename no_edge_graph< Directed >::edge_descriptor e, const no_edge_graph< Directed >& g)
{
    return ::boost::source(e, *g.base);
}

// Forwards target to the wrapped graph.
template < typename Directed >
typename no_edge_graph< Directed >::vertex_descriptor target(typename no_edge_graph< Directed >::edge_descriptor e, const no_edge_graph< Directed >& g)
{
    return ::boost::target(e, *g.base);
}

// Checks the clustering coefficient of a triangle on a graph that has no edge().
template < typename Directed > void test_without_edge_function(double expected)
{
    typename no_edge_graph< Directed >::base_type base(3);
    ::boost::add_edge(0, 1, base);
    ::boost::add_edge(0, 2, base);
    ::boost::add_edge(1, 2, base);
    no_edge_graph< Directed > g;
    g.base = &base;
    const std::size_t v0 = 0;
    BOOST_TEST_EQ(::boost::clustering_coefficient(g, v0), expected);
}

int main(int, char*[])
{
    typedef undirected_graph<> Graph;

    test_undirected< Graph >();
    test_directed< ::boost::directed_graph<> >();
    test_without_edge_function< ::boost::undirectedS >(1.0);
    test_without_edge_function< ::boost::directedS >(0.5);
    return ::boost::report_errors();
}

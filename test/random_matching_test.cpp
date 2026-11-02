//=======================================================================
// Copyright (c) 2005 Aaron Windsor
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================
#include <iostream>
#include <random>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>

#include <boost/graph/max_cardinality_matching.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost;

using undirected_graph = adjacency_list< vecS, vecS, undirectedS, property< vertex_index_t, int > >;
using vertex_index_map_t = property_map< undirected_graph, vertex_index_t >::type;
using mate_t = vector_property_map< graph_traits< undirected_graph >::vertex_descriptor, vertex_index_map_t >;
using vertex_iterator_t = graph_traits< undirected_graph >::vertex_iterator;
using vertex_descriptor_t = graph_traits< undirected_graph >::vertex_descriptor;
using v_size_t = graph_traits< undirected_graph >::vertices_size_type;

int main()
{
    // Test the checked matching on a random graph with n vertices and m edges.
    constexpr int n = 1000;
    constexpr int m = 1020;
    
    undirected_graph g(n);
    std::mt19937 generator(42);

    int num_edges = 0;
    bool success;

    while (num_edges < m)
    {
        vertex_descriptor_t u = random_vertex(g, generator);
        vertex_descriptor_t v = random_vertex(g, generator);
        if (u != v)
        {
            if (!edge(u, v, g).second)
                boost::tie(tuples::ignore, success) = add_edge(u, v, g);
            else
                success = false;

            if (success)
                num_edges++;
        }
    }

    mate_t mate(n);
    bool random_graph_result = checked_edmonds_maximum_cardinality_matching(g, mate);

    if (!random_graph_result)
    {
        std::cout << "Graph has edges: ";
        using edge_iterator_t = graph_traits< undirected_graph >::edge_iterator;
        edge_iterator_t ei, ei_end;
        for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
            std::cout << *ei << ", ";
        std::cout << std::endl;

        std::cout << "Matching is: ";
        vertex_iterator_t vi, vi_end;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
            if (mate[*vi] != graph_traits< undirected_graph >::null_vertex()
                && *vi < mate[*vi])
                std::cout << "{" << *vi << "," << mate[*vi] << "}, ";
        std::cout << std::endl;
    }
    BOOST_TEST(random_graph_result);

    // Now remove an edge from the random_mate matching.
    vertex_iterator_t vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        if (mate[*vi] != graph_traits< undirected_graph >::null_vertex())
            break;

    mate[mate[*vi]] = graph_traits< undirected_graph >::null_vertex();
    mate[*vi] = graph_traits< undirected_graph >::null_vertex();

    // the verifier should report this modified matching isn't maximum
    bool modified_random_verification_result
        = maximum_cardinality_matching_verifier< undirected_graph, mate_t,
            vertex_index_map_t >::verify_matching(g, mate,
            get(vertex_index, g));

    BOOST_TEST(!modified_random_verification_result);

    // find a greedy matching on the graph
    mate_t greedy_mate(n);
    greedy_matching< undirected_graph, mate_t >::find_matching(g, greedy_mate);

    // a verified maximum greedy matching can't be smaller than mate
    bool greedy_contradicts_verifier
        = matching_size(g, mate) > matching_size(g, greedy_mate)
        && maximum_cardinality_matching_verifier< undirected_graph, mate_t,
            vertex_index_map_t >::verify_matching(g, greedy_mate,
            get(vertex_index, g));
    BOOST_TEST(!greedy_contradicts_verifier);

    return boost::report_errors();
}

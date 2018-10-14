//=======================================================================
// Copyright (c) 2018 Yi Ji
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================

#include <boost/graph/max_cardinality_matching.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>

#include <iostream>                      // for std::cout
#include <boost/property_map/vector_property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/random.hpp>
#include <ctime>
#include <boost/random.hpp>
#include <boost/test/minimal.hpp>

using namespace boost;

typedef property<edge_weight_t, float, property<edge_index_t, int> > EdgeProperty;

typedef adjacency_list<vecS,
vecS,
undirectedS,
property<vertex_index_t,int>,
EdgeProperty> undirected_graph;

typedef adjacency_list<listS,
listS,
undirectedS,
property<vertex_index_t,int>,
EdgeProperty> undirected_list_graph;

typedef adjacency_matrix<undirectedS,
property<vertex_index_t,int>,
EdgeProperty> undirected_adjacency_matrix_graph;


template <typename Graph>
struct vertex_index_installer
{
    static void install(Graph&) {}
};


template <>
struct vertex_index_installer<undirected_list_graph>
{
    static void install(undirected_list_graph& g)
    {
        typedef graph_traits<undirected_list_graph>::vertex_iterator vertex_iterator_t;
        typedef graph_traits<undirected_list_graph>::vertices_size_type v_size_t;
        
        vertex_iterator_t vi, vi_end;
        v_size_t i = 0;
        for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi, ++i)
            put(vertex_index, g, *vi, i);
    }
};

template <typename Graph>
void print_graph(const Graph& g)
{
    typedef typename graph_traits<Graph>::edge_iterator edge_iterator_t;
    edge_iterator_t ei, ei_end;
    std::cout << std::endl << "The graph is: " << std::endl;
    for (boost::tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
        std::cout << "add_edge(" << source(*ei, g) << ", " << target(*ei, g) << ", EdgeProperty(" << get(edge_weight, g, *ei) << "), );" << std::endl;
}

template <typename Graph>
void weighted_matching_test(typename graph_traits<Graph>::vertices_size_type num_v,
                            typename graph_traits<Graph>::edges_size_type num_e,
                            const std::string& graph_name,
                            boost::mt19937& generator)
{
    typedef typename property_map<Graph,vertex_index_t>::type vertex_index_map_t;
    typedef vector_property_map< typename graph_traits<Graph>::vertex_descriptor, vertex_index_map_t > mate_t;
    typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator_t;
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor_t;
    
    boost::uniform_int<> distribution(0, num_v-1);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > rand_num(generator, distribution);
    
    num_e = std::min(num_e, num_v*num_v);
    typename graph_traits<Graph>::edges_size_type num_edges = 0;
    bool success;
    
    Graph g(num_v);
    vertex_index_installer<Graph>::install(g);
    
    while (num_edges < num_e)
    {
        vertex_descriptor_t u = random_vertex(g, rand_num);
        vertex_descriptor_t v = random_vertex(g, rand_num);
        if (u != v)
        {
            if (!edge(u,v,g).second)
                boost::tie(tuples::ignore, success) = add_edge(u, v, EdgeProperty(distribution(generator)), g);
            else
                success = false;
            
            if (success)
                ++num_edges;
        }
    }

    //print_graph(g);
    
    mate_t mate1(num_v), mate2(num_v);
    
    maximum_weighted_matching(g, mate1);
    brute_force_maximum_weighted_matching(g, mate2);
    
    bool same_result = (matching_weight_sum(g, mate1) == matching_weight_sum(g, mate2));
    BOOST_CHECK(same_result);
    if (!same_result)
    {
        std::cout << std::endl << "Found a weighted matching of weight sum " << matching_weight_sum(g, mate1) << std::endl
        << "While brute-force search found a weighted matching of weight sum " << matching_weight_sum(g, mate2) << std::endl;
        
        vertex_iterator_t vi, vi_end;
        
        print_graph(g);
        
        std::cout << std::endl << "The algorithmic matching is:" << std::endl;
        for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            if (mate1[*vi] != graph_traits<Graph>::null_vertex() && *vi < mate1[*vi])
                std::cout << "{" << *vi << ", " << mate1[*vi] << "}" << std::endl;
        
        std::cout << std::endl << "The brute-force matching is:" << std::endl;
        for (boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
            if (mate2[*vi] != graph_traits<Graph>::null_vertex() && *vi < mate2[*vi])
                std::cout << "{" << *vi << ", " << mate2[*vi] << "}" << std::endl;
        
        std::cout << std::endl;
    }

}


int test_main(int, char*[])
{
    // this test may take quite a while because of the brute-force verifier,
    // you can also lower the max_num_v and/or max_num_e
    
    std::size_t max_num_v = 16;
    std::size_t max_num_e = 24;
    std::size_t batch_size = 16;

    boost::mt19937 generator(static_cast<unsigned int>(std::time(0)));
    
    for (std::size_t num_v = 1; num_v <= max_num_v; ++num_v)
    {
        for (std::size_t num_e = num_v-1; num_e <= std::min(max_num_e,num_v*(num_v-1)/2); ++num_e)
        {
            for (std::size_t batch = 0; batch < batch_size; ++batch)
            {
                weighted_matching_test<undirected_graph>(num_v, num_e, "adjacency_list (using vectors)", generator);
                weighted_matching_test<undirected_list_graph>(num_v, num_e, "adjacency_list (using lists)", generator);
                weighted_matching_test<undirected_adjacency_matrix_graph>(num_v, num_e, "adjacency_matrix", generator);
            }
        }
    }
    
    return 0;
}



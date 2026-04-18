//=======================================================================
// Copyright 2026
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/geometric_graph_generator.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <fstream>
#include <map>

// Utility function to write graph to GraphML file with point positions
template < typename Graph >
void write_graph_to_graphml(Graph& g, const std::string& filename,
    const std::vector< boost::simple_point< double > >& points)
{
    std::ofstream file(filename);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Create property maps for x and y coordinates
    std::map< typename boost::graph_traits< Graph >::vertex_descriptor, double >
        x_map, y_map;

    typename boost::graph_traits< Graph >::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    {
        std::size_t idx = boost::get(boost::vertex_index, g, *vi);
        x_map[*vi] = points[idx].x;
        y_map[*vi] = points[idx].y;
    }

    boost::associative_property_map< std::map<
        typename boost::graph_traits< Graph >::vertex_descriptor, double > >
        x_pmap(x_map), y_pmap(y_map);

    // Create dynamic properties for GraphML output
    boost::dynamic_properties dp;
    dp.property("x", x_pmap);
    dp.property("y", y_pmap);
    dp.property("weight", boost::get(boost::edge_weight, g));

    // Write GraphML
    boost::write_graphml(file, g, dp, true);
}

void example_basic_random_graph()
{
    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;

    const std::size_t num_vertices = 25;
    Graph g(num_vertices);

    // Generate a container of random points w/ default rng and distribution (uniform)
    std::vector< Point > points;
    points.reserve(num_vertices);
    auto back_itr = std::back_inserter(points);
    boost::generate_random_points< Point >(num_vertices, 500, back_itr);

    // Get the edge weight map (matches edges to weight) and vertex index map
    auto weight_map = boost::get(boost::edge_weight, g);
    auto vertex_index_map = boost::get(boost::vertex_index, g);

    // Create complete graph (generate edges) 
    // using the stored points and populate weight map
    boost::connect_all_geometric(g, points, weight_map,
        vertex_index_map);
}

void example_custom_distribution()
{
    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;

    const std::size_t num_vertices = 15;
    Graph g(num_vertices);

    std::normal_distribution< double > normal_dist(50.0, 10.0);
    std::vector< Point > points;
    points.reserve(num_vertices);
    boost::generate_random_points< Point >(
        num_vertices, normal_dist, normal_dist, std::back_inserter(points));

    // Create graph
    boost::connect_all_geometric(g, points, boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));
}

void example_mst_on_euclidean_graph()
{
    // Use adjacency_list to make graphML printing possible
    using Graph = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;
    using Edge = boost::graph_traits< Graph >::edge_descriptor;

    const std::size_t num_vertices = 20;
    Graph g(num_vertices);

    // Generate and store points
    std::vector< Point > points;
    points.reserve(num_vertices);
    boost::generate_random_points < Point >(
        num_vertices, 500, std::back_inserter(points));

    // Create complete graph using adjacency_list
    boost::connect_all_geometric(g, points, boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));

    // Write full graph with GraphML format
    write_graph_to_graphml(g, "full_graph.graphml", points);

    // Compute MST
    std::vector< Edge > mst_edges;
    boost::kruskal_minimum_spanning_tree(g, std::back_inserter(mst_edges));

    // Create MST subgraph using adjacency_list
    Graph mst_graph(num_vertices);
    for (const auto& e : mst_edges)
    {
        auto src = boost::source(e, g);
        auto tgt = boost::target(e, g);
        auto weight = boost::get(boost::edge_weight, g, e);
        boost::add_edge(src, tgt,
            boost::property< boost::edge_weight_t, double >(weight), mst_graph);
    }

    // Write MST with GraphML format
    write_graph_to_graphml(mst_graph, "mst_graph.graphml", points);
}

void example_make_convenient_euclidean_graph()
{
    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;

    const std::size_t num_vertices = 10;
    const std::size_t coord_max = 100;
    Graph g(num_vertices);

    // Use simple convenience function to create complete graph
    boost::make_random_euclidean_graph(g, num_vertices, coord_max,
        boost::get(boost::edge_weight, g), boost::get(boost::vertex_index, g));
}

int main()
{
    // Generates a basic random Euclidean graph from convenience function
    example_make_convenient_euclidean_graph();

    // Generates a basic random Euclidean graph from building blocks
    example_basic_random_graph();

    // Shows how to use custom distributions (e.g. normal) with separate 
    // point generation and graph construction steps
    example_custom_distribution();

    // Computes MST on a 
    // generated Euclidean graph and output
    // GraphML files which can be viewed with tools like
    // Gephi or Cytoscape https://lite.gephi.org/ 
    example_mst_on_euclidean_graph();

    return EXIT_SUCCESS;
}
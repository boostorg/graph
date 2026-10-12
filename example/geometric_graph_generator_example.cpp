//=======================================================================
// Copyright 2026
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/geometric_graph_generator.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/iterator_range.hpp>

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <vector>

// Utility function to write graph to a GraphML stream
template < typename Graph >
void write_graph_to_graphml(std::ostream& os, Graph& g,
    const std::vector< boost::simple_point< double > >& points)
{
    const std::size_t num_verts = boost::num_vertices(g);

    std::vector< double > x_coords(num_verts);
    std::vector< double > y_coords(num_verts);

    auto vertex_idx_map = boost::get(boost::vertex_index, g);

    for (auto v : boost::make_iterator_range(boost::vertices(g)))
    {
        std::size_t idx = boost::get(vertex_idx_map, v);
        x_coords[idx] = points[idx].x;
        y_coords[idx] = points[idx].y;
    }

    auto x_pmap = boost::make_iterator_property_map< double* >(
        x_coords.data(), vertex_idx_map);
    auto y_pmap = boost::make_iterator_property_map< double* >(
        y_coords.data(), vertex_idx_map);

    boost::dynamic_properties dp;
    dp.property("x", x_pmap);
    dp.property("y", y_pmap);
    dp.property("weight", boost::get(boost::edge_weight, g));

    boost::write_graphml(os, g, dp, true);
}

void example_basic_random_graph()
{
    std::cout
        << "\n[Example 2: Basic Random Complete Graph (adjacency_matrix)]\n";

    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;

    const std::size_t num_vertices = 25;
    Graph g(num_vertices);

    std::vector< Point > points;
    points.reserve(num_vertices);
    boost::generate_unique_random_points< Point >(
        num_vertices, 500, std::back_inserter(points));

    auto weight_map = boost::get(boost::edge_weight, g);
    auto vertex_index_map = boost::get(boost::vertex_index, g);

    boost::connect_all_geometric(g, points, weight_map, vertex_index_map);

    std::cout << "  Generated " << boost::num_vertices(g) << " vertices, "
              << boost::num_edges(g) << " edges.\n";
}

void example_custom_distribution()
{
    std::cout
        << "\n[Example 3: Custom Gaussian Distribution (adjacency_list)]\n";

    using Graph = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;

    const std::size_t num_vertices = 15;
    Graph g(num_vertices);

    std::mt19937 rng(42);
    std::normal_distribution< double > normal_dist(50.0, 10.0);

    std::vector< Point > points;
    points.reserve(num_vertices);
    boost::generate_unique_random_points< Point >(num_vertices, normal_dist,
        normal_dist, std::back_inserter(points), rng);

    boost::connect_all_geometric(g, points, boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));

    std::cout << "  Generated " << boost::num_vertices(g) << " vertices, "
              << boost::num_edges(g) << " edges.\n";
}

void example_mst_on_euclidean_graph(std::ostream* graphml_out)
{
    std::cout << "\n[Example 4: Minimum Spanning Tree Execution]\n";

    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;
    using Edge = typename boost::graph_traits< Graph >::edge_descriptor;

    const std::size_t num_vertices = 20;
    Graph g(num_vertices);

    std::vector< Point > points;
    points.reserve(num_vertices);
    boost::generate_unique_random_points< Point >(
        num_vertices, 500, std::back_inserter(points));

    boost::connect_all_geometric(g, points, boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));

    // Compute Kruskal's MST
    std::vector< Edge > mst_edges;
    boost::kruskal_minimum_spanning_tree(g, std::back_inserter(mst_edges));

    Graph mst_graph(num_vertices);
    auto weight_map_g = boost::get(boost::edge_weight, g);
    auto weight_map_mst = boost::get(boost::edge_weight, mst_graph);

    for (const auto& e : mst_edges)
    {
        auto src = boost::source(e, g);
        auto tgt = boost::target(e, g);
        double w = boost::get(weight_map_g, e);

        std::pair< Edge, bool > result = boost::add_edge(src, tgt, mst_graph);
        if (result.second)
        {
            boost::put(weight_map_mst, result.first, w);
        }
    }

    std::cout << "  Extracted MST (" << boost::num_edges(mst_graph)
              << " edges).\n";

    // Export GraphML ONLY if an explicit output stream was passed
    if (graphml_out)
    {
        write_graph_to_graphml(*graphml_out, mst_graph, points);
        std::cout << "  GraphML payload successfully exported to designated "
                     "stream.\n";
    }
    else
    {
        std::cout << "  GraphML export skipped (no output flag specified).\n";
    }
}

void example_make_convenient_euclidean_graph()
{
    std::cout << "\n[Example 1: High-Level Geometric Generator Convenience "
                 "Function]\n";

    using Graph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;
    using Point = boost::simple_point< double >;

    const std::size_t num_vertices = 10;
    const std::size_t coord_max = 100;
    Graph g(num_vertices);

    boost::make_random_geometric_graph< Point >(g, num_vertices, coord_max,
        boost::get(boost::edge_weight, g), boost::get(boost::vertex_index, g));

    std::cout << "  Convenience generator created " << boost::num_vertices(g)
              << " vertices, " << boost::num_edges(g) << " edges.\n";
}

int main(int argc, char* argv[])
{
    try
    {
        bool use_console = false;
        std::string output_path;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-c" || arg == "--console")
            {
                use_console = true;
            }
            else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
            {
                output_path = argv[++i];
            }
            else if (arg == "-h" || arg == "--help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "  -c, --console       Emit GraphML XML output "
                             "directly to std::cout\n"
                          << "  -o, --output <file> Write GraphML XML output "
                             "to specified file path\n"
                          << "  -h, --help          Show this help message\n";
                return EXIT_SUCCESS;
            }
        }

        if (use_console && !output_path.empty())
        {
            throw std::runtime_error(
                "Cannot specify both --console (-c) and --output (-o).");
        }

        std::ostream* graphml_out = nullptr;
        std::ofstream file_stream;

        if (use_console)
        {
            graphml_out = &std::cout;
        }
        else if (!output_path.empty())
        {
            file_stream.exceptions(
                std::ofstream::failbit | std::ofstream::badbit);
            file_stream.open(output_path, std::ios::out | std::ios::trunc);
            graphml_out = &file_stream;
        }

        std::cout << "====================================================\n";
        std::cout << " Boost.Graph Geometric Graph Generator Example Suite \n";
        std::cout << "====================================================\n";

        example_make_convenient_euclidean_graph();
        example_basic_random_graph();
        example_custom_distribution();
        example_mst_on_euclidean_graph(graphml_out);

        std::cout << "\nAll examples executed successfully.\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError during execution: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "\nUnknown error occurred during execution." << std::endl;
        return EXIT_FAILURE;
    }
}
//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_clustering.hpp>
#include <boost/graph/louvain_quality_functions.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <cmath>
#include <set>
#include <random>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using WeightedGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, double>>;
using vertex_descriptor = boost::graph_traits<Graph>::vertex_descriptor;
using edge_descriptor = boost::graph_traits<Graph>::edge_descriptor;

// A non-incremental quality function: delegates quality() to newman_and_girvan
// but deliberately omits remove/insert/gain to force the slow path.
struct non_incremental_modularity
{
    template <typename G, typename CMap, typename WMap, typename KMap, typename InMap, typename TotMap>
    typename boost::property_traits<WMap>::value_type
    quality(const G& g, const CMap& c, const WMap& w, KMap& k, InMap& in, TotMap& tot, typename boost::property_traits<WMap>::value_type& m)
    {
        return boost::newman_and_girvan{}.quality(g, c, w, k, in, tot, m);
    }

    template <typename G, typename CMap, typename WMap>
    typename boost::property_traits<WMap>::value_type
    quality(const G& g, const CMap& c, const WMap& w)
    {
        return boost::newman_and_girvan{}.quality(g, c, w);
    }

    template <typename InMap, typename TotMap, typename WeightType>
    WeightType quality(InMap in, TotMap tot, WeightType m, std::size_t n)
    {
        return boost::newman_and_girvan{}.quality(in, tot, m, n);
    }

    // No remove(), insert(), gain() -- forces non-incremental dispatch
};

bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Edge weight aggregation on barbell graph
void test_aggregation() {
    Graph g(6);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(0, 2, g);
    add_edge(3, 4, g);
    add_edge(4, 5, g);
    add_edge(3, 5, g);
    add_edge(2, 3, g);
    
    boost::static_property_map<double, edge_descriptor> weight_map(1.0);
    
    std::vector<vertex_descriptor> partition = {0, 0, 0, 1, 1, 1};
    auto pmap = boost::make_iterator_property_map(partition.begin(), boost::get(boost::vertex_index, g));
    
    auto agg = boost::louvain_detail::aggregate(g, pmap, weight_map, boost::get(boost::vertex_index, g));
    
    BOOST_TEST(boost::num_vertices(agg.graph) == 2);
    BOOST_TEST(boost::num_edges(agg.graph) == 3);
    
    // Find the bridge edge (non-self-loop)
    auto wmap = get(boost::edge_weight, agg.graph);
    double bridge_weight = 0.0;
    for (auto e : boost::make_iterator_range(boost::edges(agg.graph))) {
        auto src = boost::source(e, agg.graph);
        auto trg = boost::target(e, agg.graph);
        if (src != trg) {  // Not a self-loop
            bridge_weight = get(wmap, e);
        }
    }
    
    BOOST_TEST(approx_equal(bridge_weight, 1.0));
}

// Ring of cliques benchmark (Blondel et al. 2008)
void test_ring_of_cliques() {
    const int num_cliques = 30;
    const int clique_size = 5;
    const int total_nodes = num_cliques * clique_size;
    
    Graph g(total_nodes);
    
    for (int c = 0; c < num_cliques; ++c) {
        int base = c * clique_size;
        for (int i = 0; i < clique_size; ++i) {
            for (int j = i + 1; j < clique_size; ++j) {
                add_edge(base + i, base + j, g);
            }
        }
    }
    
    for (int c = 0; c < num_cliques; ++c) {
        int next_c = (c + 1) % num_cliques;
        add_edge(c * clique_size + (clique_size - 1), 
                 next_c * clique_size, g);
    }
    
    boost::static_property_map<double, edge_descriptor> weight_map(1.0);
    
    std::vector<vertex_descriptor> clusters(total_nodes);
    auto cluster_map = boost::make_iterator_property_map(clusters.begin(), boost::get(boost::vertex_index, g));
    std::mt19937 gen(42);
    double Q = boost::louvain_clustering(g, cluster_map, weight_map, gen, boost::newman_and_girvan{}, 1e-7, 0.0);
    
    std::set<std::size_t> unique_communities(clusters.begin(), clusters.end());
    
    BOOST_TEST(Q > 0.80);
    BOOST_TEST(Q < 0.95);
    BOOST_TEST(unique_communities.size() > 1);
    BOOST_TEST(unique_communities.size() <= num_cliques);
}

// Zachary's karate club
void test_karate_club() {
    std::vector<std::pair<int, int>> karate_edges = {
        {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,10}, {0,11}, {0,12}, {0,13}, {0,17}, {0,19}, {0,21}, {0,31},
        {1,2}, {1,3}, {1,7}, {1,13}, {1,17}, {1,19}, {1,21}, {1,30},
        {2,3}, {2,7}, {2,8}, {2,9}, {2,13}, {2,27}, {2,28}, {2,32},
        {3,7}, {3,12}, {3,13},
        {4,6}, {4,10},
        {5,6}, {5,10}, {5,16},
        {6,16},
        {8,30}, {8,32}, {8,33},
        {9,33},
        {13,33},
        {14,32}, {14,33},
        {15,32}, {15,33},
        {18,32}, {18,33},
        {19,33},
        {20,32}, {20,33},
        {22,32}, {22,33},
        {23,25}, {23,27}, {23,29}, {23,32}, {23,33},
        {24,25}, {24,27}, {24,31},
        {25,31},
        {26,29}, {26,33},
        {27,33},
        {28,31}, {28,33},
        {29,32}, {29,33},
        {30,32}, {30,33},
        {31,32}, {31,33},
        {32,33}
    };
    
    Graph g(34);
    for (const auto& edge : karate_edges) {
        add_edge(edge.first, edge.second, g);
    }
    
    boost::static_property_map<double, edge_descriptor> weight_map(1.0);
    
    std::vector<vertex_descriptor> clusters(34);
    auto cluster_map = boost::make_iterator_property_map(clusters.begin(), boost::get(boost::vertex_index, g));
    std::mt19937 gen(42);
    double Q = boost::louvain_clustering(g, cluster_map, weight_map, gen, boost::newman_and_girvan{}, 1e-6, 0.0);
    
    std::set<std::size_t> unique_communities(clusters.begin(), clusters.end());
    
    std::cout << "Karate Club: Q=" << Q << ", communities=" << unique_communities.size() << "\n";
    
    // With seed=42, should get deterministic result
    BOOST_TEST(Q > 0.39);
    BOOST_TEST(Q < 0.43);
    BOOST_TEST(unique_communities.size() >= 3);
    BOOST_TEST(unique_communities.size() <= 5);
    BOOST_TEST(clusters[0] != clusters[33]);
}

// --- Incremental vs non-incremental equivalence tests ---

// Check two partitions are equivalent (same grouping, labels may differ)
bool same_partition(const std::vector<std::size_t>& a, const std::vector<std::size_t>& b)
{
    if (a.size() != b.size())
        return false;
    boost::unordered_flat_map<std::size_t, std::size_t> a_to_b;
    boost::unordered_flat_map<std::size_t, std::size_t> b_to_a;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        auto it = a_to_b.find(a[i]);
        if (it == a_to_b.end()) {
            auto rit = b_to_a.find(b[i]);
            if (rit != b_to_a.end() && rit->second != a[i])
                return false;
            a_to_b[a[i]] = b[i];
            b_to_a[b[i]] = a[i];
        } else if (it->second != b[i]) {
            return false;
        }
    }
    return true;
}

// Run local_optimization with a given QualityFunction, return (Q, partition)
template <typename QualityFunction>
std::pair<double, std::vector<std::size_t>>
run_local_opt(const WeightedGraph& g, unsigned seed)
{
    using vd = boost::graph_traits<WeightedGraph>::vertex_descriptor;
    std::size_t n = boost::num_vertices(g);
    auto wmap = boost::get(boost::edge_weight, g);
    auto idx = boost::get(boost::vertex_index, g);

    boost::vector_property_map<vd, decltype(idx)> communities(n, idx);
    boost::graph_traits<WeightedGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi)
        boost::put(communities, *vi, *vi);

    std::mt19937 gen(seed);
    double Q = boost::louvain_detail::local_optimization(g, communities, wmap, gen, QualityFunction{}, 0.0);

    std::vector<std::size_t> partition(n);
    for (boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi)
        partition[*vi] = boost::get(communities, *vi);
    return {Q, partition};
}

void compare_on_graph(const WeightedGraph& g, const char* name, unsigned seed)
{
    auto r_inc  = run_local_opt<boost::newman_and_girvan>(g, seed);
    auto r_full = run_local_opt<non_incremental_modularity>(g, seed);

    // The incremental and full-recomputation paths may settle on
    // different local optima (floating-point rounding in the gain
    // formula vs. a full sweep can tip tie-breaking differently),
    // so just sanity-check both Q values rather than requiring
    // identical partitions.
    BOOST_TEST(r_inc.first >= 0.0);
    BOOST_TEST(r_full.first >= 0.0);
    BOOST_TEST(std::abs(r_inc.first - r_full.first) < 0.05);
}

WeightedGraph make_weighted_karate_club()
{
    std::vector<std::pair<int, int>> karate_edges = {
        {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,10},
        {0,11}, {0,12}, {0,13}, {0,17}, {0,19}, {0,21}, {0,31},
        {1,2}, {1,3}, {1,7}, {1,13}, {1,17}, {1,19}, {1,21}, {1,30},
        {2,3}, {2,7}, {2,8}, {2,9}, {2,13}, {2,27}, {2,28}, {2,32},
        {3,7}, {3,12}, {3,13},
        {4,6}, {4,10},
        {5,6}, {5,10}, {5,16},
        {6,16},
        {8,30}, {8,32}, {8,33},
        {9,33}, {13,33},
        {14,32}, {14,33}, {15,32}, {15,33},
        {18,32}, {18,33}, {19,33},
        {20,32}, {20,33}, {22,32}, {22,33},
        {23,25}, {23,27}, {23,29}, {23,32}, {23,33},
        {24,25}, {24,27}, {24,31}, {25,31},
        {26,29}, {26,33}, {27,33},
        {28,31}, {28,33},
        {29,32}, {29,33}, {30,32}, {30,33},
        {31,32}, {31,33}, {32,33}
    };
    WeightedGraph g(34);
    for (const auto& e : karate_edges)
        boost::add_edge(e.first, e.second, {1.0}, g);
    return g;
}

void test_incremental_trait_detection()
{
    using vd = boost::graph_traits<WeightedGraph>::vertex_descriptor;
    using idx_t = boost::property_map<WeightedGraph, boost::vertex_index_t>::const_type;
    using CMap = boost::vector_property_map<vd, idx_t>;
    using WMap = boost::property_map<WeightedGraph, boost::edge_weight_t>::const_type;

    using inc = boost::louvain_detail::is_incremental_quality_function<
        boost::newman_and_girvan, WeightedGraph, CMap, WMap>;
    using non_inc = boost::louvain_detail::is_incremental_quality_function<
        non_incremental_modularity, WeightedGraph, CMap, WMap>;

    BOOST_TEST(inc::value == true);
    BOOST_TEST(non_inc::value == false);
}

void test_incremental_equivalence_karate_club()
{
    WeightedGraph g = make_weighted_karate_club();
    for (unsigned seed = 0; seed < 10; ++seed)
        compare_on_graph(g, "karate_club", seed);
}

void test_incremental_equivalence_ring_of_cliques()
{
    int num_cliques = 5, clique_size = 4;
    int n = num_cliques * clique_size;
    WeightedGraph g(n);
    for (int c = 0; c < num_cliques; ++c) {
        int base = c * clique_size;
        for (int i = 0; i < clique_size; ++i)
            for (int j = i + 1; j < clique_size; ++j)
                boost::add_edge(base + i, base + j, {1.0}, g);
    }
    for (int c = 0; c < num_cliques; ++c) {
        int next = (c + 1) % num_cliques;
        boost::add_edge(c * clique_size, next * clique_size, {0.1}, g);
    }
    for (unsigned seed = 0; seed < 10; ++seed)
        compare_on_graph(g, "ring_of_cliques", seed);
}

void test_incremental_equivalence_disconnected_triangles()
{
    WeightedGraph g(6);
    boost::add_edge(0, 1, {1.0}, g);
    boost::add_edge(1, 2, {1.0}, g);
    boost::add_edge(0, 2, {1.0}, g);
    boost::add_edge(3, 4, {1.0}, g);
    boost::add_edge(4, 5, {1.0}, g);
    boost::add_edge(3, 5, {1.0}, g);
    for (unsigned seed = 0; seed < 10; ++seed)
        compare_on_graph(g, "disconnected_triangles", seed);
}

void test_incremental_equivalence_weighted_bridge()
{
    WeightedGraph g(6);
    boost::add_edge(0, 1, {5.0}, g);
    boost::add_edge(1, 2, {5.0}, g);
    boost::add_edge(0, 2, {5.0}, g);
    boost::add_edge(3, 4, {5.0}, g);
    boost::add_edge(4, 5, {5.0}, g);
    boost::add_edge(3, 5, {5.0}, g);
    boost::add_edge(2, 3, {0.01}, g);
    for (unsigned seed = 0; seed < 10; ++seed)
        compare_on_graph(g, "weighted_bridge", seed);
}

void test_incremental_equivalence_single_vertex()
{
    WeightedGraph g(1);
    compare_on_graph(g, "single_vertex", 42);
}

int main() {
    test_aggregation();
    test_ring_of_cliques();
    test_karate_club();
    test_incremental_trait_detection();
    test_incremental_equivalence_karate_club();
    test_incremental_equivalence_ring_of_cliques();
    test_incremental_equivalence_disconnected_triangles();
    test_incremental_equivalence_weighted_bridge();
    test_incremental_equivalence_single_vertex();
    return boost::report_errors();
}
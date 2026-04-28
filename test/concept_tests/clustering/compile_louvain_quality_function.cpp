//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Verify that louvain_clustering accepts:
//  - The built-in newman_and_girvan
//  - A user-defined non-incremental quality function
//  - A user-defined incremental quality function

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_clustering.hpp>
#include <boost/graph/louvain_quality_functions.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <set>

struct custom_non_incremental_modularity
{
    template <typename Graph, typename CommunityMap, typename WeightMap>
    static typename boost::property_traits<WeightMap>::value_type
    quality(const Graph& g, const CommunityMap& communities, const WeightMap& w)
    {
        return boost::newman_and_girvan{}.quality(g, communities, w);
    }
};

struct custom_incremental_modularity
{
    template <typename Graph, typename CommunityMap, typename WeightMap>
    static typename boost::property_traits<WeightMap>::value_type
    quality(const Graph& g, const CommunityMap& communities, const WeightMap& w)
    {
        return boost::newman_and_girvan{}.quality(g, communities, w);
    }

    template <typename Graph, typename CommunityMap, typename WeightMap, typename KMap, typename InMap, typename TotMap>
    static typename boost::property_traits<WeightMap>::value_type
    quality(const Graph& g, const CommunityMap& communities, const WeightMap& w, KMap& k, InMap& in, TotMap& tot, typename boost::property_traits<WeightMap>::value_type& m)
    {
        return boost::newman_and_girvan{}.quality(g, communities, w, k, in, tot, m);
    }

    template <typename InMap, typename TotMap>
    static typename boost::property_traits<InMap>::value_type
    quality(const InMap& in, const TotMap& tot, typename boost::property_traits<InMap>::value_type m, std::size_t n)
    {
        return boost::newman_and_girvan{}.quality(in, tot, m, n);
    }

    template <typename InMap, typename TotMap, typename CommunityType, typename WeightType>
    static void remove(InMap& in, TotMap& tot, CommunityType c, WeightType k_v, WeightType k_v_in, WeightType w_selfloop)
    {
        boost::newman_and_girvan{}.remove(in, tot, c, k_v, k_v_in, w_selfloop);
    }

    template <typename InMap, typename TotMap, typename CommunityType, typename WeightType>
    static void insert(InMap& in, TotMap& tot, CommunityType c, WeightType k_v, WeightType k_v_in, WeightType w_selfloop)
    {
        boost::newman_and_girvan{}.insert(in, tot, c, k_v, k_v_in, w_selfloop);
    }

    template <typename TotMap, typename WeightType, typename CommunityType>
    static WeightType gain(const TotMap& tot, WeightType m, CommunityType c, WeightType k_v_in, WeightType k_v)
    {
        return boost::newman_and_girvan{}.gain(tot, m, c, k_v_in, k_v);
    }
};


using Graph = boost::adjacency_list<
    boost::vecS, boost::vecS, boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, double>>;

Graph make_two_triangles()
{
    Graph g(6);
    // Triangle
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);
    // Triangle
    boost::add_edge(3, 4, 1.0, g);
    boost::add_edge(4, 5, 1.0, g);
    boost::add_edge(3, 5, 1.0, g);
    // Bridge
    boost::add_edge(2, 3, 0.01, g);
    return g;
}

template <typename QualityFunction>
void run_with_quality_function(const Graph& g, const char* name)
{
    using vertex_t = boost::graph_traits<Graph>::vertex_descriptor;

    std::vector<vertex_t> clusters(boost::num_vertices(g));
    auto cmap = boost::make_iterator_property_map(clusters.begin(), boost::get(boost::vertex_index, g));
    auto wmap = boost::get(boost::edge_weight, g);

    std::mt19937 rng(42);
    double Q = boost::louvain_clustering(g, cmap, wmap, rng, QualityFunction{});

    BOOST_TEST(Q >= 0.0);
    BOOST_TEST(Q <= 1.0);

    std::set<vertex_t> communities(clusters.begin(), clusters.end());
    BOOST_TEST(communities.size() >= 1u);
}

void test_trait_detection()
{
    using vertex_t = boost::graph_traits<Graph>::vertex_descriptor;
    using idx_t = boost::property_map<Graph, boost::vertex_index_t>::const_type;
    using CMap = boost::vector_property_map<vertex_t, idx_t>;
    using WMap = boost::property_map<Graph, boost::edge_weight_t>::const_type;

    using ng_inc = boost::louvain_detail::is_incremental_quality_function<boost::newman_and_girvan, Graph, CMap, WMap>;
    BOOST_TEST(ng_inc::value == true);

    using custom_inc = boost::louvain_detail::is_incremental_quality_function<custom_incremental_modularity, Graph, CMap, WMap>;
    BOOST_TEST(custom_inc::value == true);

    using custom_non_inc = boost::louvain_detail::is_incremental_quality_function<custom_non_incremental_modularity, Graph, CMap, WMap>;
    BOOST_TEST(custom_non_inc::value == false);
}


void test_builtin_newman_girvan()
{
    Graph g = make_two_triangles();
    run_with_quality_function<boost::newman_and_girvan>(g, "newman_and_girvan");
}

void test_custom_non_incremental()
{
    Graph g = make_two_triangles();
    run_with_quality_function<custom_non_incremental_modularity>(g, "custom_non_incremental");
}

void test_custom_incremental()
{
    Graph g = make_two_triangles();
    run_with_quality_function<custom_incremental_modularity>(g, "custom_incremental");
}

int main()
{
    test_trait_detection();
    test_builtin_newman_girvan();
    test_custom_non_incremental();
    test_custom_incremental();
    return boost::report_errors();
}
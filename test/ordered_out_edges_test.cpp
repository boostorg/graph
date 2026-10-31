//=======================================================================
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <cstddef>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using name_property = boost::property< boost::edge_name_t, std::string >;

// Orders stored edges by target, then by edge name.
template < class StoredEdge > struct order_by_name
{
    bool operator()(const StoredEdge& e1, const StoredEdge& e2) const
    {
        const auto target1 = e1.get_target();
        const auto target2 = e2.get_target();
        const auto& name1 = boost::get(boost::edge_name, e1);
        const auto& name2 = boost::get(boost::edge_name, e2);
        return std::tie(target1, name1) < std::tie(target2, name2);
    }
};

// Selects a multiset of out edges sorted by order_by_name.
struct multiset_by_nameS
{
};

// Selects a set of out edges sorted by order_by_name.
struct set_by_nameS
{
};

namespace boost
{
// Maps multiset_by_nameS to its container.
template < class ValueType > struct container_gen< multiset_by_nameS, ValueType >
{
    using type = std::multiset< ValueType, order_by_name< ValueType > >;
};

// Maps set_by_nameS to its container.
template < class ValueType > struct container_gen< set_by_nameS, ValueType >
{
    using type = std::set< ValueType, order_by_name< ValueType > >;
};

// Allows parallel edges for multiset_by_nameS.
template <> struct parallel_edge_traits< multiset_by_nameS >
{
    using type = allow_parallel_edge_tag;
};

// Disallows parallel edges for set_by_nameS.
template <> struct parallel_edge_traits< set_by_nameS >
{
    using type = disallow_parallel_edge_tag;
};
}

// Checks edge and edge_range on parallel named edges.
template < class Directed > void test_edge_lookup()
{
    using Graph = boost::adjacency_list< multiset_by_nameS, boost::vecS, Directed, boost::no_property, name_property >;

    Graph g(3);
    boost::add_edge(0, 1, name_property("joe"), g);
    boost::add_edge(0, 1, name_property("chandler"), g);
    boost::add_edge(1, 2, name_property("curly"), g);
    auto name_map = boost::get(boost::edge_name, g);

    auto found_edge = boost::edge(0, 1, g);
    BOOST_TEST(found_edge.second);
    BOOST_TEST_EQ(boost::get(name_map, found_edge.first), std::string("chandler"));

    auto range = boost::edge_range(0, 1, g);
    std::vector< std::string > names;
    for (auto e : boost::make_iterator_range(range.first, range.second))
    {
        names.push_back(boost::get(name_map, e));
    }
    const std::vector< std::string > expected_names{ "chandler", "joe" };
    BOOST_TEST_ALL_EQ(names.begin(), names.end(), expected_names.begin(), expected_names.end());

    auto missing_edge = boost::edge(0, 2, g);
    BOOST_TEST(!missing_edge.second);
}

// Checks that remove_edge erases a named edge when parallel edges are disallowed.
template < class Directed > void test_remove_edge()
{
    using Graph = boost::adjacency_list< set_by_nameS, boost::vecS, Directed, boost::no_property, name_property >;

    Graph g(3);
    boost::add_edge(0, 1, name_property("joe"), g);
    boost::add_edge(1, 2, name_property("curly"), g);
    boost::remove_edge(0, 1, g);

    auto removed_edge = boost::edge(0, 1, g);
    BOOST_TEST(!removed_edge.second);
    auto kept_edge = boost::edge(1, 2, g);
    BOOST_TEST(kept_edge.second);
    constexpr std::size_t expected_edge_count = 1;
    BOOST_TEST_EQ(boost::num_edges(g), expected_edge_count);
}

int main()
{
    test_edge_lookup< boost::directedS >();
    test_edge_lookup< boost::bidirectionalS >();
    test_edge_lookup< boost::undirectedS >();
    test_remove_edge< boost::directedS >();
    test_remove_edge< boost::bidirectionalS >();
    test_remove_edge< boost::undirectedS >();
    return boost::report_errors();
}

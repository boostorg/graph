// Copyright (C) Vladimir Prus 2003.
// Copyright (C) Cromwell D. Enage 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

typedef boost::adjacency_list<
    boost::vecS, boost::vecS, boost::directedS,
    boost::property<boost::vertex_rank_t, std::size_t>
> G1;
typedef boost::graph_traits<G1>::vertex_descriptor V1;
typedef boost::graph_traits<G1>::vertex_iterator V1Itr;
typedef boost::adjacency_list<
    boost::vecS, boost::setS, boost::directedS,
    boost::property<boost::vertex_discover_time_t, std::size_t>
> G2;
typedef boost::graph_traits<G2>::vertex_descriptor V2;

#include <boost/graph/properties.hpp>

class copier
{
    const G1& _g1;
    G2& _g2;

public:
    copier(const G1& g1, G2& g2) : _g1(g1), _g2(g2)
    {
    }

    void operator()(const V1& v1, const V2& v2) const
    {
        put(
            get(boost::vertex_discover_time, this->_g2), v2, (
                num_vertices(this->_g1) - 1 -
                get(get(boost::vertex_rank, this->_g1), v1)
            )
        );
    }
};

#include <boost/graph/copy.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/core/lightweight_test.hpp>
#include <vector>

int main()
{
    G1 g1_1, g1_2;
    G2 g2;
    V1 v1_1 = add_vertex(g1_1);
    V1 v1_2 = add_vertex(g1_1);

    add_edge(v1_1, v1_2, g1_1);
    put(get(boost::vertex_rank, g1_1), v1_1, 4);
    put(get(boost::vertex_rank, g1_1), v1_2, 8);
    v1_1 = add_vertex(g1_1);
    v1_2 = add_vertex(g1_1);
    add_edge(v1_1, v1_2, g1_1);
    put(get(boost::vertex_rank, g1_1), v1_1, 15);
    put(get(boost::vertex_rank, g1_1), v1_2, 16);
    v1_1 = add_vertex(g1_1);
    v1_2 = add_vertex(g1_1);
    add_edge(v1_1, v1_2, g1_1);
    put(get(boost::vertex_rank, g1_1), v1_1, 23);
    put(get(boost::vertex_rank, g1_1), v1_2, 42);
    add_edge(vertex(1, g1_1), vertex(4, g1_1), g1_1);
    add_edge(vertex(3, g1_1), vertex(0, g1_1), g1_1);
    add_edge(vertex(5, g1_1), vertex(2, g1_1), g1_1);
    boost::copy_graph(g1_1, g1_2);
    BOOST_TEST_EQ(num_vertices(g1_1), num_vertices(g1_2));

    for (std::size_t i = 0; i < num_vertices(g1_2); ++i)
    {
        BOOST_TEST_EQ(
            get(get(boost::vertex_rank, g1_1), vertex(i, g1_1)),
            get(get(boost::vertex_rank, g1_2), vertex(i, g1_2))
        );
    }

    BOOST_TEST(is_adjacent(g1_2, vertex(0, g1_2), vertex(1, g1_2)));
    BOOST_TEST(is_adjacent(g1_2, vertex(1, g1_2), vertex(4, g1_2)));
    BOOST_TEST(is_adjacent(g1_2, vertex(2, g1_2), vertex(3, g1_2)));
    BOOST_TEST(is_adjacent(g1_2, vertex(3, g1_2), vertex(0, g1_2)));
    BOOST_TEST(is_adjacent(g1_2, vertex(4, g1_2), vertex(5, g1_2)));
    BOOST_TEST(is_adjacent(g1_2, vertex(5, g1_2), vertex(2, g1_2)));

    std::vector<
        V2
    > o2c(num_vertices(g1_1), boost::graph_traits<G2>::null_vertex());
    copier c(g1_1, g2);

    boost::copy_graph(
        g1_1,
        g2,
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#if !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS) || ( \
        defined(BOOST_NO_CXX11_DECLTYPE) \
    )
        boost::graph::keywords::_vertex_copy =
#endif
        c,
#if !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS) || ( \
        defined(BOOST_NO_CXX11_DECLTYPE) \
    )
        boost::graph::keywords::_orig_to_copy =
#endif
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        boost::vertex_copy(c).orig_to_copy(
#endif
            make_iterator_property_map(
                o2c.begin(),
                get(boost::vertex_index, g1_1)
            )
#if !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        )
#endif
    );

    V1Itr v1_i, v1_end;

    for (boost::tie(v1_i, v1_end) = vertices(g1_1); v1_i != v1_end; ++v1_i)
    {
        BOOST_TEST_EQ(
            num_vertices(
                g1_1
            ) - 1 - get(get(boost::vertex_rank, g1_1), *v1_i),
            get(
                get(boost::vertex_discover_time, g2),
                o2c[get(get(boost::vertex_index, g1_1), *v1_i)]
            )
        );
    }

    BOOST_TEST(is_adjacent(g2, o2c[0], o2c[1]));
    BOOST_TEST(is_adjacent(g2, o2c[1], o2c[4]));
    BOOST_TEST(is_adjacent(g2, o2c[2], o2c[3]));
    BOOST_TEST(is_adjacent(g2, o2c[3], o2c[0]));
    BOOST_TEST(is_adjacent(g2, o2c[4], o2c[5]));
    BOOST_TEST(is_adjacent(g2, o2c[5], o2c[2]));

    return boost::report_errors();
}


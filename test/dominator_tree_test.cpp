//=======================================================================
// Copyright (C) 2005 Jong Soo Park <jongsoo.park -at- gmail.com>
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/dominator_tree.hpp>

using namespace std;

struct DominatorCorrectnessTestSet
{
    typedef pair< int, int > edge;

    int numOfVertices;
    vector< edge > edges;
    vector< int > correctIdoms;
};

using namespace boost;

// a workaround for the C++ standard before C++17, after switching to C++17,
// the method may be just inlined into the run_test() with constexpr if.
namespace detail
{

template < typename Graph >
void index_graph(Graph&, std::true_type /*IsRandomAccessAdjacentList*/)
{
    // nothing to do for already indexed adjacent list
}

template < typename Graph >
void index_graph(Graph& g, std::false_type /*IsRandomAccessAdjacentList*/)
{
    using IndexMap = typename property_map< Graph, vertex_index_t >::type;
    IndexMap indexMap(get(vertex_index, g));
    typename graph_traits< Graph >::vertex_iterator uItr, uEnd;
    int j = 0;
    for (boost::tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr, ++j)
    {
        put(indexMap, *uItr, j);
    }
}

} // namespace detail

template < typename OEL, typename VL, typename D, typename VP, typename EP,
    typename GP, typename EL >
void index_graph(adjacency_list< OEL, VL, D, VP, EP, GP, EL >& g)
{
    using Traits = adjacency_list_traits< OEL, VL, D, EL >;
    ::detail::index_graph(
        g, std::integral_constant< bool, Traits::is_rand_access::value > {});
}

template < typename D, typename VP, typename EP, typename GP, typename A >
void index_graph(adjacency_matrix<D, VP, EP, GP, A>&)
{
    // nothing to do for already indexed adjacent matrix
}

template < typename Graph >
void run_test()
{
    using edge = DominatorCorrectnessTestSet::edge;

    DominatorCorrectnessTestSet testSet[7];

    // Tarjan's paper
    testSet[0].numOfVertices = 13;
    testSet[0].edges.push_back(edge(0, 1));
    testSet[0].edges.push_back(edge(0, 2));
    testSet[0].edges.push_back(edge(0, 3));
    testSet[0].edges.push_back(edge(1, 4));
    testSet[0].edges.push_back(edge(2, 1));
    testSet[0].edges.push_back(edge(2, 4));
    testSet[0].edges.push_back(edge(2, 5));
    testSet[0].edges.push_back(edge(3, 6));
    testSet[0].edges.push_back(edge(3, 7));
    testSet[0].edges.push_back(edge(4, 12));
    testSet[0].edges.push_back(edge(5, 8));
    testSet[0].edges.push_back(edge(6, 9));
    testSet[0].edges.push_back(edge(7, 9));
    testSet[0].edges.push_back(edge(7, 10));
    testSet[0].edges.push_back(edge(8, 5));
    testSet[0].edges.push_back(edge(8, 11));
    testSet[0].edges.push_back(edge(9, 11));
    testSet[0].edges.push_back(edge(10, 9));
    testSet[0].edges.push_back(edge(11, 0));
    testSet[0].edges.push_back(edge(11, 9));
    testSet[0].edges.push_back(edge(12, 8));
    testSet[0].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(3);
    testSet[0].correctIdoms.push_back(3);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(7);
    testSet[0].correctIdoms.push_back(0);
    testSet[0].correctIdoms.push_back(4);

    // Appel. p441. figure 19.4
    testSet[1].numOfVertices = 7;
    testSet[1].edges.push_back(edge(0, 1));
    testSet[1].edges.push_back(edge(1, 2));
    testSet[1].edges.push_back(edge(1, 3));
    testSet[1].edges.push_back(edge(2, 4));
    testSet[1].edges.push_back(edge(2, 5));
    testSet[1].edges.push_back(edge(4, 6));
    testSet[1].edges.push_back(edge(5, 6));
    testSet[1].edges.push_back(edge(6, 1));
    testSet[1].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[1].correctIdoms.push_back(0);
    testSet[1].correctIdoms.push_back(1);
    testSet[1].correctIdoms.push_back(1);
    testSet[1].correctIdoms.push_back(2);
    testSet[1].correctIdoms.push_back(2);
    testSet[1].correctIdoms.push_back(2);

    // Appel. p449. figure 19.8
    testSet[2].numOfVertices = 13, testSet[2].edges.push_back(edge(0, 1));
    testSet[2].edges.push_back(edge(0, 2));
    testSet[2].edges.push_back(edge(1, 3));
    testSet[2].edges.push_back(edge(1, 6));
    testSet[2].edges.push_back(edge(2, 4));
    testSet[2].edges.push_back(edge(2, 7));
    testSet[2].edges.push_back(edge(3, 5));
    testSet[2].edges.push_back(edge(3, 6));
    testSet[2].edges.push_back(edge(4, 7));
    testSet[2].edges.push_back(edge(4, 2));
    testSet[2].edges.push_back(edge(5, 8));
    testSet[2].edges.push_back(edge(5, 10));
    testSet[2].edges.push_back(edge(6, 9));
    testSet[2].edges.push_back(edge(7, 12));
    testSet[2].edges.push_back(edge(8, 11));
    testSet[2].edges.push_back(edge(9, 8));
    testSet[2].edges.push_back(edge(10, 11));
    testSet[2].edges.push_back(edge(11, 1));
    testSet[2].edges.push_back(edge(11, 12));
    testSet[2].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[2].correctIdoms.push_back(0);
    testSet[2].correctIdoms.push_back(0);
    testSet[2].correctIdoms.push_back(1);
    testSet[2].correctIdoms.push_back(2);
    testSet[2].correctIdoms.push_back(3);
    testSet[2].correctIdoms.push_back(1);
    testSet[2].correctIdoms.push_back(2);
    testSet[2].correctIdoms.push_back(1);
    testSet[2].correctIdoms.push_back(6);
    testSet[2].correctIdoms.push_back(5);
    testSet[2].correctIdoms.push_back(1);
    testSet[2].correctIdoms.push_back(0);

    testSet[3].numOfVertices = 8, testSet[3].edges.push_back(edge(0, 1));
    testSet[3].edges.push_back(edge(1, 2));
    testSet[3].edges.push_back(edge(1, 3));
    testSet[3].edges.push_back(edge(2, 7));
    testSet[3].edges.push_back(edge(3, 4));
    testSet[3].edges.push_back(edge(4, 5));
    testSet[3].edges.push_back(edge(4, 6));
    testSet[3].edges.push_back(edge(5, 7));
    testSet[3].edges.push_back(edge(6, 4));
    testSet[3].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[3].correctIdoms.push_back(0);
    testSet[3].correctIdoms.push_back(1);
    testSet[3].correctIdoms.push_back(1);
    testSet[3].correctIdoms.push_back(3);
    testSet[3].correctIdoms.push_back(4);
    testSet[3].correctIdoms.push_back(4);
    testSet[3].correctIdoms.push_back(1);

    // Muchnick. p256. figure 8.21
    testSet[4].numOfVertices = 8, testSet[4].edges.push_back(edge(0, 1));
    testSet[4].edges.push_back(edge(1, 2));
    testSet[4].edges.push_back(edge(2, 3));
    testSet[4].edges.push_back(edge(2, 4));
    testSet[4].edges.push_back(edge(3, 2));
    testSet[4].edges.push_back(edge(4, 5));
    testSet[4].edges.push_back(edge(4, 6));
    testSet[4].edges.push_back(edge(5, 7));
    testSet[4].edges.push_back(edge(6, 7));
    testSet[4].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[4].correctIdoms.push_back(0);
    testSet[4].correctIdoms.push_back(1);
    testSet[4].correctIdoms.push_back(2);
    testSet[4].correctIdoms.push_back(2);
    testSet[4].correctIdoms.push_back(4);
    testSet[4].correctIdoms.push_back(4);
    testSet[4].correctIdoms.push_back(4);

    // Muchnick. p253. figure 8.18
    testSet[5].numOfVertices = 8, testSet[5].edges.push_back(edge(0, 1));
    testSet[5].edges.push_back(edge(0, 2));
    testSet[5].edges.push_back(edge(1, 6));
    testSet[5].edges.push_back(edge(2, 3));
    testSet[5].edges.push_back(edge(2, 4));
    testSet[5].edges.push_back(edge(3, 7));
    testSet[5].edges.push_back(edge(5, 7));
    testSet[5].edges.push_back(edge(6, 7));
    testSet[5].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[5].correctIdoms.push_back(0);
    testSet[5].correctIdoms.push_back(0);
    testSet[5].correctIdoms.push_back(2);
    testSet[5].correctIdoms.push_back(2);
    testSet[5].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[5].correctIdoms.push_back(1);
    testSet[5].correctIdoms.push_back(0);

    // Cytron's paper, fig. 9
    testSet[6].numOfVertices = 14, testSet[6].edges.push_back(edge(0, 1));
    testSet[6].edges.push_back(edge(0, 13));
    testSet[6].edges.push_back(edge(1, 2));
    testSet[6].edges.push_back(edge(2, 3));
    testSet[6].edges.push_back(edge(2, 7));
    testSet[6].edges.push_back(edge(3, 4));
    testSet[6].edges.push_back(edge(3, 5));
    testSet[6].edges.push_back(edge(4, 6));
    testSet[6].edges.push_back(edge(5, 6));
    testSet[6].edges.push_back(edge(6, 8));
    testSet[6].edges.push_back(edge(7, 8));
    testSet[6].edges.push_back(edge(8, 9));
    testSet[6].edges.push_back(edge(9, 10));
    testSet[6].edges.push_back(edge(9, 11));
    testSet[6].edges.push_back(edge(10, 11));
    testSet[6].edges.push_back(edge(11, 9));
    testSet[6].edges.push_back(edge(11, 12));
    testSet[6].edges.push_back(edge(12, 2));
    testSet[6].edges.push_back(edge(12, 13));
    testSet[6].correctIdoms.push_back((numeric_limits< int >::max)());
    testSet[6].correctIdoms.push_back(0);
    testSet[6].correctIdoms.push_back(1);
    testSet[6].correctIdoms.push_back(2);
    testSet[6].correctIdoms.push_back(3);
    testSet[6].correctIdoms.push_back(3);
    testSet[6].correctIdoms.push_back(3);
    testSet[6].correctIdoms.push_back(2);
    testSet[6].correctIdoms.push_back(2);
    testSet[6].correctIdoms.push_back(8);
    testSet[6].correctIdoms.push_back(9);
    testSet[6].correctIdoms.push_back(9);
    testSet[6].correctIdoms.push_back(11);
    testSet[6].correctIdoms.push_back(0);

    for (size_t i = 0; i < sizeof(testSet) / sizeof(testSet[0]); ++i)
    {
        const int numOfVertices = testSet[i].numOfVertices;

        Graph g(testSet[i].edges.begin(), testSet[i].edges.end(), numOfVertices);

        using Vertex = typename graph_traits< Graph >::vertex_descriptor;
        using IndexMap = typename property_map< Graph, vertex_index_t >::type;
        IndexMap indexMap(get(vertex_index, g));
        using PredMap
            = iterator_property_map< typename vector< Vertex >::iterator, IndexMap >;

        index_graph(g);

        vector< Vertex > domTreePredVector, domTreePredVector2;

        // Lengauer-Tarjan dominator tree algorithm
        domTreePredVector = vector< Vertex >(
            num_vertices(g), graph_traits< Graph >::null_vertex());
        PredMap domTreePredMap
            = make_iterator_property_map(domTreePredVector.begin(), indexMap);

        lengauer_tarjan_dominator_tree(g, vertex(0, g), domTreePredMap);

        vector< int > idom(num_vertices(g));
        typename graph_traits< Graph >::vertex_iterator uItr, uEnd;
        for (boost::tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr)
        {
            if (get(domTreePredMap, *uItr)
                != graph_traits< Graph >::null_vertex())
                idom[get(indexMap, *uItr)]
                    = get(indexMap, get(domTreePredMap, *uItr));
            else
                idom[get(indexMap, *uItr)] = (numeric_limits< int >::max)();
        }

        copy(idom.begin(), idom.end(), ostream_iterator< int >(cout, " "));
        cout << endl;

        // dominator tree correctness test
        BOOST_TEST(std::equal(
            idom.begin(), idom.end(), testSet[i].correctIdoms.begin()));

        // compare results of fast version and slow version of dominator tree
        domTreePredVector2 = vector< Vertex >(
            num_vertices(g), graph_traits< Graph >::null_vertex());
        domTreePredMap
            = make_iterator_property_map(domTreePredVector2.begin(), indexMap);

        iterative_bit_vector_dominator_tree(g, vertex(0, g), domTreePredMap);

        vector< int > idom2(num_vertices(g));
        for (boost::tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr)
        {
            if (get(domTreePredMap, *uItr)
                != graph_traits< Graph >::null_vertex())
                idom2[get(indexMap, *uItr)]
                    = get(indexMap, get(domTreePredMap, *uItr));
            else
                idom2[get(indexMap, *uItr)] = (numeric_limits< int >::max)();
        }

        copy(idom2.begin(), idom2.end(), ostream_iterator< int >(cout, " "));
        cout << endl;

        size_t k;
        for (k = 0; k < num_vertices(g); ++k)
            BOOST_TEST(domTreePredVector[k] == domTreePredVector2[k]);
    }
    cout << endl;
}

int main(int, char*[])
{
    using AdjacencyListList = adjacency_list< listS, listS, bidirectionalS,
        property< vertex_index_t, std::size_t >, no_property >;

    using AdjacencyListVec = adjacency_list< listS, vecS, bidirectionalS >;

    using AdjacencyMatrix = adjacency_matrix< directedS >;

    run_test< AdjacencyListList >();
    run_test< AdjacencyListVec >();
    run_test< AdjacencyMatrix >();

    return boost::report_errors();
}

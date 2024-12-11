#include <boost/core/lightweight_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
/*
#include <boost/graph/two_bit_color_map.hpp>
*/
#include <boost/graph/is_connected.hpp>

#include <boost/graph/two_bit_color_map.hpp>

using namespace std;
using namespace boost;

// todo: consider mdspan when it's widely implemented
inline size_t coord_to_idx(size_t height, size_t x, size_t y)
{
    return y * height + x;
}

template < class Graph >
void fill_square_graph(Graph& graph, size_t size)
{
    const size_t W = size, H = size;

    for (size_t i = 0; i < W; ++i)
    {
        for (size_t j = 0; j < H; ++j)
        {
            size_t idx = coord_to_idx(H, i, j);
            if (i > 0)
                add_edge(coord_to_idx(H, i - 1, j), idx, graph);
            if (j > 0)
                add_edge(coord_to_idx(H, i, j - 1), idx, graph);
        }
    }
}

int main(int argc, char* argv[])
{
    // the side length of the square graph
    size_t size = 20;
    if (argc > 1)
    {
        size = lexical_cast< int >(argv[1]);
    }

    {
        // test is_strongly_connected
        typedef adjacency_list<> dir_graph_t;
        dir_graph_t g(size * size);
        // the directed graph is not strongly connected
        fill_square_graph(g, size);

        BOOST_TEST(is_connected(g) == false);
        BOOST_TEST(is_connected(g, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g) == false);
        BOOST_TEST(is_connected(g, is_connected_kind::unilateral) == false);

        // now make it strongly connected with one more edge from the last vertex
        // to the first
        add_edge(coord_to_idx(size, size - 1, size - 1), 0, g);

        BOOST_TEST(is_connected(g) == true);
        BOOST_TEST(is_connected(g, is_connected_kind::strong) == true);
        BOOST_TEST(is_strongly_connected(g) == true);
        BOOST_TEST(is_connected(g, is_connected_kind::unilateral) == true);
    }

    {
        // test is_weakly_connected -- it requires bidirectional graph
        typedef adjacency_list< vecS, vecS, bidirectionalS > bidir_graph_t;
        bidir_graph_t g(size * size);
        fill_square_graph(g, size);
        // the directed graph is not strongly connected
        BOOST_TEST(is_connected(g, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g) == false);
        // but it is weakly connected
        BOOST_TEST(is_connected(g, is_connected_kind::weak) == true);
        BOOST_TEST(is_weakly_connected(g) == true);

        // another graph, 2 disconnected vertices
        bidir_graph_t g2(2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == false);

        // make it weakly connected
        add_edge(0, 1, g2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == true);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
    }

    {
        // test undirected is_connected, it is the same for all connection kinds
        typedef adjacency_list< vecS, vecS, undirectedS > undir_graph_t;
        undir_graph_t g(size * size);
        // the undirected graph is already strongly connected
        fill_square_graph(g, size);

        BOOST_TEST(is_connected(g) == true);
        BOOST_TEST(is_connected_undirected(g) == true);
        BOOST_TEST(is_connected(g, is_connected_kind::strong) == true);
        BOOST_TEST(is_connected(g, is_connected_kind::weak) == true);

        // now fill fewer edges, so one row is disconnected
        undir_graph_t g2(size * (size + 1));
        fill_square_graph(g2, size);

        BOOST_TEST(is_connected(g2) == false);
        BOOST_TEST(is_connected_undirected(g2) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == false);

    }

    {
        // test all is_connected kinds
        typedef adjacency_list< vecS, vecS, bidirectionalS > bidir_graph_t;
        bidir_graph_t g(size * size);
        fill_square_graph(g, size);

        BOOST_TEST(is_connected(g, is_connected_kind::weak) == true);
        BOOST_TEST(is_weakly_connected(g) == true);

        BOOST_TEST(is_connected(g, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g) == false);

        BOOST_TEST(is_connected(g, is_connected_kind::unilateral) == false);
        BOOST_TEST(is_unilaterally_connected(g) == false);

        bidir_graph_t g2(2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::unilateral) == false);

        add_edge(0, 1, g2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == true);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::unilateral) == true);


        // test making a graph weakly, then unilaterally, then strongly connected
        bidir_graph_t g3(3);
        BOOST_TEST(is_connected(g3, is_connected_kind::weak) == false);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == false);
        BOOST_TEST(is_connected(g3, is_connected_kind::strong) == false);
        add_edge(0, 1, g3);
        add_edge(2, 1, g3);
        BOOST_TEST(is_connected(g3, is_connected_kind::weak) == true);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == false);
        BOOST_TEST(is_connected(g3, is_connected_kind::strong) == false);
        add_edge(0, 2, g3);
        BOOST_TEST(is_connected(g3, is_connected_kind::weak) == true);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == true);
        BOOST_TEST(is_connected(g3, is_connected_kind::strong) == false);
        add_edge(1, 0, g3);
        BOOST_TEST(is_connected(g3, is_connected_kind::weak) == true);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == true);
        BOOST_TEST(is_connected(g3, is_connected_kind::strong) == true);

        // Test the usage of the old interface with ColorMap as the second arg.
        // This causes static assertion. I don't see a way to test for it, so
        // it's commented out.
        // BOOST_TEST(is_connected(g3, two_bit_color_map<>(0)) == false);
    }

    /*
    {
        // test that adjacency matrix implements bidirectional graph
        // (see another PR [todo: link]
        typedef boost::adjacency_matrix<> bidir_graph_t;
        bidir_graph_t g(size * size);
        fill_square_graph(g, size);

        BOOST_TEST(is_connected(g, is_connected_kind::weak) == true);
        BOOST_TEST(is_weakly_connected(g) == true);

        BOOST_TEST(is_connected(g, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g) == false);

        BOOST_TEST(is_connected(g, is_connected_kind::unilateral) == false);
        BOOST_TEST(is_unilaterally_connected(g) == false);

        bidir_graph_t g2(2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == false);
        BOOST_TEST(is_weakly_connected(g2) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g2) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::unilateral) == false);
        BOOST_TEST(is_unilaterally_connected(g2) == false);

        add_edge(0, 1, g2);
        BOOST_TEST(is_connected(g2, is_connected_kind::weak) == true);
        BOOST_TEST(is_weakly_connected(g2) == true);
        BOOST_TEST(is_connected(g2, is_connected_kind::strong) == false);
        BOOST_TEST(is_strongly_connected(g2) == false);
        BOOST_TEST(is_connected(g2, is_connected_kind::unilateral) == true);
        BOOST_TEST(is_unilaterally_connected(g2) == true);

        bidir_graph_t g3(3);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == false);
        add_edge(0, 1, g3);
        add_edge(2, 1, g3);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == false);
        add_edge(0, 2, g3);
        BOOST_TEST(is_connected(g3, is_connected_kind::unilateral) == true);
    }
    */

    // consider testing it if we decide to preserve the old interface
    // test_colormap_reuse(size);

    return boost::report_errors();
}

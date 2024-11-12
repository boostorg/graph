#include <boost/core/lightweight_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/two_bit_color_map.hpp>

using namespace std;
using namespace boost;

// todo: consider mdspan when it's widely implemented
inline size_t coord_to_idx(size_t height, size_t x, size_t y) {
    return y * height + x;
}

template <class Graph>
void fill_square_graph(Graph& graph, size_t size) {
    const size_t W = size, H = size;

    for (size_t i = 0; i < W; ++i) {
        for (size_t j = 0; j < H; ++j) {
            size_t idx = coord_to_idx(H, i, j);
            if (i > 0)
                add_edge(coord_to_idx(H, i - 1, j), idx, graph);
            if (j > 0)
                add_edge(coord_to_idx(H, i, j - 1), idx, graph);
        }
    }
}

template <typename Graph>
void run_test(const Graph& g, bool exp_result) {
    bool result = is_connected(g, boost::make_two_bit_color_map(num_vertices(g), get(boost::vertex_index, g)));
    BOOST_TEST(exp_result == result);
}

int main(int argc, char* argv[])
{
    // the side length of the square graph
    size_t size = 20;
    if (argc > 1)
    {
        size = lexical_cast< int >(argv[1]);
    }

    typedef adjacency_list<> dir_graph_t;
    dir_graph_t g(size * size);
    // the directed graph is not strongly connected
    fill_square_graph(g, size);
    run_test(g, false);

    // now make it connected with one more edge from the last vertex to the first
    add_edge(coord_to_idx(size, size - 1, size - 1), 0, g);
    run_test(g, true);

    typedef adjacency_list<vecS, vecS, undirectedS> undir_graph_t;
    undir_graph_t ug(size * size);
    // the undirected graph is already strongly connected
    fill_square_graph(g, size);
    run_test(g, true);

    // now fill fewer edges, so one row is disconnected
    undir_graph_t ug2(size * (size + 1));
    fill_square_graph(ug2, size);
    run_test(ug2, false);

    return boost::report_errors();
}


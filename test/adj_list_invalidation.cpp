
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

using namespace std;
using namespace boost;

// The purpose of this test is simply to provide a testing ground for the
// invalidation of iterators and descriptors.

template <typename Graph>
void make_graph(Graph& g)
{
    // Build a simple (barbell) graph.
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    Vertex u = add_vertex(g);
    Vertex v = add_vertex(g);
    add_edge(u, v, g);
}

template <typename Graph>
void invalidate_edges()
{
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    typedef typename graph_traits<Graph>::edge_iterator EdgeIterator;

    Graph g;
    make_graph(g);

    // The actual test. These are valid here.
    EdgeIterator i = edges(g).first;
    Edge e = *i;

    // Add a vertex, see what breaks.
    add_vertex(g);
    int x;
    cout << "...edge iter" << endl;
    x = g[*i];
    cout << "...edge desc" << endl;
    x = g[e];
};

template <typename Graph>
void invalidate_vertices()
{
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename graph_traits<Graph>::vertex_iterator VertexIterator;

    Graph g;
    make_graph(g);

    // The actual test. These are valid here.
    VertexIterator i = vertices(g).first;
    Vertex v = *i;

    // Add a vertex, see what breaks.
    add_vertex(g);
    int x;
    cout << "...vert iter" << endl;
    x = g[*i];
    cout << "...vert desc" << endl;
    x = g[v];
}

int main()
{
    typedef adjacency_list<vecS, vecS, undirectedS, int, int> VVU;
//     invalidate_vertices<VVU>();
//     invalidate_edges<VVU>();

    typedef adjacency_list<vecS, vecS, directedS, int, int> VVD;
//     invalidate_vertices<VVD>();
//     invalidate_edges<VVD>();

    typedef adjacency_list<vecS, vecS, bidirectionalS, int, int> VVB;
//     invalidate_vertices<VVB>();
//     invalidate_edges<VVB>();
}


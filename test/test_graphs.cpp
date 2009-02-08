
#include <iostream>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/directed_graph.hpp>

using namespace std;
using namespace boost;

struct node
{
    node() : n() { }
    node(int n) : n(n) { }

    bool operator==(node const& x) const    { return n == x.n; }
    bool operator<(node const& x) const     { return n < x.n; }

    int n;
};

struct arc
{
    arc() : n() { }
    arc(int n) : n(n) { }

    bool operator==(arc const& x) const     { return n == x.n; }

    int n;
};

// TODO: Finish implementing this test. Actually, generalize the test so that
// it works for lots of different graph types.

template <typename Graph>
void test()
{
    typedef typename Graph::vertex_descriptor Vertex;
    Graph g;
    BOOST_ASSERT(num_vertices(g) == 0);

}

int main()
{
    test< undirected_graph<node, arc> >();
    test< directed_graph<node, arc> >();
}


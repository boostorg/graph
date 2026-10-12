#include <boost/config.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

using namespace std;
using namespace boost;

int main(int argc, char**)
{
    typedef adjacency_list< setS, vecS, undirectedS,
        property< vertex_color_t, default_color_type,
            property< vertex_degree_t, int,
                property< vertex_priority_t, double > > > >
        Graph;

    typedef graph_traits< Graph >::vertex_descriptor Vertex;
    typedef graph_traits< Graph >::vertices_size_type size_type;

    Graph g = Graph(4);
    property_map< Graph, vertex_index_t >::type index_map
        = get(vertex_index, g);

    std::vector< Vertex > inv_perm(num_vertices(g));
    std::vector< size_type > perm(num_vertices(g));

    add_edge(0, 1, g);
    add_edge(0, 2, g);

    sloan_ordering(g, inv_perm.begin(), get(vertex_color, g),
        make_degree_map(g), get(vertex_priority, g), 1, 2);

    int correct[] = { 3, 1, 0, 2 };

    size_t idx = 0;
    for (typename std::vector< Vertex >::const_iterator i = inv_perm.begin();
         i != inv_perm.end(); ++i)
    {
        BOOST_ASSERT(correct[idx] == *i);
        ++idx;
    }
}

#include <boost/graph/adjacency_list.hpp>
typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, boost::property<vertex_id_t, std::size_t>, boost::property<edge_id_t, std::size_t> > Graph;
typedef boost::property<vertex_id_t, std::size_t> VertexID;
typedef boost::property<edge_id_t, std::size_t> EdgeID;

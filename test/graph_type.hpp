#include <boost/graph/adjacency_list.hpp>
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<vertex_id_t, std::size_t>, boost::property<edge_id_t, std::size_t> > Graph;
typedef boost::property<vertex_id_t, std::size_t> VertexId;
typedef boost::property<edge_id_t, std::size_t> EdgeID;

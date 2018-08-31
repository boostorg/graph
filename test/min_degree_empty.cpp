#include <boost/graph/copy.hpp>
#include <iostream>
#include <stdlib.h>
#include <boost/graph/minimum_degree_ordering.hpp>
#include <boost/graph/adjacency_list.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> G;

int main(int argc, char** argv)
{
	size_t n=10;
	G g(n);

	std::vector<int> inverse_perm(n, 0);
	std::vector<int> supernode_sizes(n, 1);
	BOOST_AUTO(id, boost::get(boost::vertex_index, g));
	std::vector<int> degree(n, 0);
	std::vector<int> io(n, 0);
	std::vector<int> o(n, 0);

// with an edge, there is no issue.
//	boost::add_edge(1, 2, g);

	// this crashes (assert fail) with unpatched bucket sorter.
	boost::minimum_degree_ordering
		(g,
		 boost::make_iterator_property_map(&degree[0], id, degree[0]),
		 &io[0],
		 &o[0],
		 boost::make_iterator_property_map(&supernode_sizes[0], id, supernode_sizes[0]),
		 0,
		 id
		);
}

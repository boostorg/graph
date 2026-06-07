
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/personalized_page_rank.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

int main(int, char*[]) {
    using namespace boost;
    // deliberately hard (slow-converging) graph
    using Graph = adjacency_list<vecS, vecS, directedS>;
    std::vector<std::pair<int,int>> edges = {
        {0,1},{1,0},{1,2},{2,1},{2,3},{3,2},
        {4,5},{5,4},{5,6},{6,5},{6,7},{7,6},{7,8},{8,7},{8,9},{9,8},{9,10},{10,9},
        {0,3},{3,0},{1,3},{3,1},{1,4},{4,1},
        {4,6},{6,4},{6,9},{9,6},{6,8},{8,6},{7,9},{9,7},{8,10},{10,8},
        {11,10},{10,11},{10,12},{12,10}
    };
    Graph g(edges.begin(), edges.end(), 13);

    std::vector<double> ranks(num_vertices(g));
    auto rank_map = make_iterator_property_map(ranks.begin(), get(vertex_index, g));
    std::vector<double> personalization(num_vertices(g));
    auto personalization_map = make_iterator_property_map(personalization.begin(), get(vertex_index, g));
    personalization[0] = 1;
    personalization[1] = 1;
    personalization[2] = 1;
    personalization[3] = 1;

    std::size_t max_iters(100); // Convergence is so bad in this graph that it needs such a high cap.
    auto weight = spectral_weights(g);
    auto convergence1 = graph::vertex_map_convergence(max_iters, 1.E-9);
    convergence1 = graph::personalized_page_rank(g, weight, personalization_map, rank_map, convergence1);

    std::cout << "ended after "<<max_iters-convergence1.get_remaining_iters() << " iterations\n";
    std::cout << std::fixed << std::setprecision(4);
    for (std::size_t v = 0; v < num_vertices(g); ++v) 
    {
        std::cout << "vertex " << v << "  rank=" << ranks[v] << "\n";
    }
    BOOST_ASSERT(ranks[0]==ranks[2]); // should be exactly equal due to symmetry, even when considering floating point coarseness
    BOOST_ASSERT(ranks[0]<ranks[1]+0.1);
    BOOST_ASSERT(ranks[8]==ranks[9]);
    BOOST_ASSERT(ranks[11]==ranks[12]);
    BOOST_ASSERT(ranks[11]>0);
    BOOST_ASSERT(convergence1.has_converged());

    // COMPUTING A HIGH-PASS VERSION BY PASSING NEGATIVE DAMPING (resulting values can be negative)
    // This is not completely correct yet for computing graph gradients, because it needs to eskew normalization,
    // so we need to be able to customize said normalization.
    // Damping should generaly be in the range [-1,1].
    auto renorm_weight = renormalized_weights(g);
    auto convergence2 = graph::vertex_map_convergence(max_iters, 1.E-9);
    convergence2 = graph::personalized_page_rank(g, renorm_weight, personalization_map, rank_map, convergence2, -0.8);
    std::cout << "ended after "<<max_iters-convergence2.get_remaining_iters() << " iterations\n";
    std::cout << std::fixed << std::setprecision(4);
    for (std::size_t v = 0; v < num_vertices(g); ++v) 
    {
        std::cout << "vertex " << v << "  flow=" << ranks[v] << "\n";
    }
    BOOST_ASSERT(ranks[0]==ranks[2]); // should be exactly equal due to symmetry, even when considering floating point coarseness
    BOOST_ASSERT(ranks[0]>ranks[1]+0.1);
    BOOST_ASSERT(ranks[8]==ranks[9]);
    BOOST_ASSERT(ranks[11]==ranks[12]);
    BOOST_ASSERT(ranks[11]<0);
    BOOST_ASSERT(convergence1.has_converged());
}

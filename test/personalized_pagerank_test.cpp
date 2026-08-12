
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/personalized_page_rank.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <boost/core/lightweight_test.hpp>

using DirectedGraph = typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using DirectedVertex = typename boost::graph_traits<DirectedGraph>::vertex_descriptor;
using DirectedEdge = typename boost::graph_traits<DirectedGraph>::edge_descriptor;

struct custom_rank_convergence: public boost::graph::rank_convergence
{
    explicit custom_rank_convergence(std::size_t iters, double tol=0) : boost::graph::rank_convergence(iters,tol) {}
    std::size_t get_remaining_iters() const { return iters; }
};

void directed_graph_tests(double damping, double renormalize)
{
    std::vector<std::pair<std::vector<std::pair<int,int>>, int>> graph_defs = {
        // deliberately hard symmetric graph
        {{
            {0,1},{1,0},{1,2},{2,1},{2,3},{3,2},
            {4,5},{5,4},{5,6},{6,5},{6,7},{7,6},{7,8},{8,7},{8,9},{9,8},{9,10},{10,9},
            {0,3},{3,0},{1,3},{3,1},{1,4},{4,1},
            {4,6},{6,4},{6,9},{9,6},{6,8},{8,6},{7,9},{9,7},{8,10},{10,8},
            {11,10},{10,11},{10,12},{12,10}
        }, 13},
        // undirected circle with 0 and 2 being symmetric, and a non-symmetric directed one-way blocks 2 hops away from 0 ({7,6} blocked) and 2 ({4,5} blocked)
        {{ {0,1},{1,0},{1,3},{3,1},{3,2},{2,3},{2,4},{4,2},{5,4},{5,6},{6,5},{6,7},{7,0},{0,7}}, 8},
        // fully undirected graph
        {{ {0,1}, {2,1}, {0,3}, {2,3}, {3,4}, {4,5}, {1,5}, {5,2}, {5,0}}, 6},
        // same as above but missing incoming edges for 0 and 2 (whole graph is a sink, needs correct normalization that guards against zero to not yield nans)
        {{ {0,1}, {2,1}, {0,3}, {2,3}, {3,4}, {4,5}, {1,5}}, 6}
    };
    for(auto& graph_details : graph_defs)
    {
        DirectedGraph g(graph_details.first.begin(), graph_details.first.end(), graph_details.second);
        std::vector<double> ranks(num_vertices(g));
        auto rank_map = boost::make_iterator_property_map(ranks.begin(), get(boost::vertex_index, g));
        std::vector<double> personalization(num_vertices(g));
        auto personalization_map = boost::make_iterator_property_map(personalization.begin(), get(boost::vertex_index, g));
        personalization[0] = 1;
        personalization[1] = 1;
        personalization[2] = 1;
        personalization[3] = 1;

        std::size_t max_iters(300); // Convergence is just that bad in tested graphs; usually it's much lower.'
        auto weight = boost::make_function_property_map<DirectedEdge, double>([&g,renormalize](DirectedEdge e){
            auto denom = out_degree(source(e, g), g) * out_degree(target(e, g), g);
            if(denom==0) return 0.0;
            return 1.0 / std::sqrt(renormalize+double(denom));
        });
        auto convergence = custom_rank_convergence(max_iters, 1.E-9);
        convergence = boost::graph::personalized_page_rank(g, weight, personalization_map, rank_map, convergence, damping);

        // the following asserts hold for all tested graphs and personalization: 0,1,2,3 plus some other nodes is a mini-cluster with 0 and 2 being structurally symmetric
        BOOST_TEST(convergence.get_remaining_iters()<max_iters);                                     // ran
        BOOST_TEST(convergence.get_remaining_iters()>0 || (damping<0.0));                            // converged (derivatives may not converge)
        BOOST_TEST((ranks[0]<ranks[1]+0.1) || (damping<0.0) || damping>1.0);                         // holds for all graphs given low-pass damping
        BOOST_TEST((ranks[0]!=ranks[1]) == (damping!=0.0));                                          // but not the same normally, the same 1.0 personalization if damping is zero
        BOOST_TEST(ranks[0]==ranks[2] || (damping<=-1.0));                                           // equal due to symmetry, even under non-convergence and floating coarseness
        BOOST_TEST(std::abs(ranks[0]-ranks[2])<1.E-14);                                              // approximately equal in cases where even addition order matters
        BOOST_TEST((ranks[0]<0.0)||(ranks[1]<0.0)||(ranks[num_vertices(g)-1]<=0.0)||(damping>0.0));  // ensure that negative flows are possible for negative damping
    }
}

int main(int, char*[])
{
    directed_graph_tests(0.9, 0);    // normal mode
    directed_graph_tests(0.9, 1.0);  // with renormalization
    directed_graph_tests(0.99, 0);   // huge damping (asymptotically exponentially slow convergence as we approach 1.0)
    directed_graph_tests(0.0, 0);    // just yield the personalization again
    directed_graph_tests(-0.8, 1.0); // negative flow = some kind of derivate
    directed_graph_tests(-1.0, 1.0); // huge negative flow
    return boost::report_errors();
}

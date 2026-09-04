#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/page_rank.hpp>
#include <boost/graph/personalized_page_rank.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <iostream>
#include <vector>
#include <iomanip>

using DirectedGraph = typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using DirectedVertex = typename boost::graph_traits<DirectedGraph>::vertex_descriptor;
using DirectedEdge = typename boost::graph_traits<DirectedGraph>::edge_descriptor;

using BidirectionalGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
using BidirectionalVertex = typename boost::graph_traits<BidirectionalGraph>::vertex_descriptor;
using BidirectionalEdge = typename boost::graph_traits<BidirectionalGraph>::edge_descriptor;

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

void compliance_comparison_tests(double damping, int repetitions)
{
    // compares personalized pagerank with the non-personalized pagerank algorithm implementation
    const unsigned damping_id = static_cast<unsigned>(std::lround(damping * 100.0));
    std::mt19937 rng(damping_id);
    for (int repetition=0; repetition<repetitions; ++repetition)
    {
        // random graph of 5 to 55 vertices
        std::size_t n = 5 + (rng() % 50);
        std::size_t max_edges = n*(n-1)/2;
        BidirectionalGraph g;
        boost::generate_random_graph(g, n, rng() % (max_edges+1), rng, false, false);

        // run pagerank
        std::vector<double> pagerank_ranks(n);
        auto pagerank_map = boost::make_iterator_property_map(
            pagerank_ranks.begin(), 
            get(boost::vertex_index, g)
        );
        boost::graph::page_rank(g, pagerank_map, boost::graph::n_iterations(10000), damping);

        // run personalized pagerank
        std::vector<double> personalized_pagerank_ranks(n);
        auto personalized_result_map = boost::make_iterator_property_map(
            personalized_pagerank_ranks.begin(), 
            get(boost::vertex_index, g)
        );
        std::vector<double> personalization(n);
        for(int i=0;i<n;++i)
            personalization[i] = 1.0;
        auto personalization_map = boost::make_iterator_property_map(
            personalization.begin(), 
            get(boost::vertex_index, g)
        );
        auto edge_weight = boost::make_function_property_map<BidirectionalEdge, double>([&g](BidirectionalEdge e){
            const auto degree = out_degree(source(e, g), g);
            return 1.0 / degree;
        });
        boost::graph::personalized_page_rank(g, edge_weight, personalization_map, personalized_result_map, custom_rank_convergence(10000), damping);

        // compare
        std::size_t differences = 0;
        double expected_l1_norm = 0;
        double actual_l1_norm = 0;
        for (std::size_t v=0;v<n;++v)
        {
            const double expected = pagerank_map[v];
            const double actual   = personalized_result_map[v];
            BOOST_TEST(std::isfinite(expected));
            BOOST_TEST(std::isfinite(actual));
            if(std::abs(expected - actual) > 1.E-9) 
                ++differences;
            expected_l1_norm += expected;
            actual_l1_norm += actual;
        }
        
        // EXPALAINING DIFFERENCES IN personalized_page_rank VS page_rank
        // The way page_rank has been implemented, it does NOT preserve the
        // input L1 norm when there are dangling nodes. Those nodes are, however,
        // assigned a known score `1-damping`. Rescaling the final result is not
        // enough because this fixed dangling node treatment "poisons" the
        // value of other nodes.
        //
        // On the other hand, personalized_page_rank strives to maintain largely 
        // the same invariants with the added constraint that the input L1 norm
        // is ALWAYS preserved. This means that dangling node values are scaled
        // into an unknown (though fixed)
        //
        // Applying `boost::graph::remove_dangling_links` on the graph
        // before ranking its nots is how this discrepancy is mitigated. 
        // However, this is not done in this testm in order to prepare for 
        // future extensions where the personalized_page_rank normalization 
        // schema can be controlled externally.
        // 
        // Currently, commenting out `put(to_rank, v, get(to_rank, v)/l1_norm);`,
        // which is what should be controlled, lets the test pass without skipping
        // anything.
        if(std::abs(expected_l1_norm-n)>1.E-9) 
            continue;
        // the following should guarantee equivalence
        BOOST_TEST(std::abs(expected_l1_norm-actual_l1_norm)<1.E-9); // easier to investigate if not held true
        BOOST_TEST(!differences); // stronger condition that all ranks should be similar
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

    for(int i=0;i<=99;++i)
        compliance_comparison_tests(0.01*i, 20);
    return boost::report_errors();
}

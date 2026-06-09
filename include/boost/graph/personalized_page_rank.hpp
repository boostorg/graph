// Copyright 2026 Emmanouil Krasanakis

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Emmanouil Krasanakis

#ifndef BOOST_GRAPH_PERSONALIZED_PAGE_RANK_HPP
#define BOOST_GRAPH_PERSONALIZED_PAGE_RANK_HPP

#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/overloading.hpp>
#include <boost/graph/detail/mpi_include.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <vector>

namespace boost
{
namespace graph
{
    struct rank_convergence
    {
        explicit rank_convergence(std::size_t iters, double tol=0) : iters(iters), tol(tol) {} // allowing tolerance for early stopping
        template < typename RankMap, typename RankMap2, typename Graph >
        bool operator()(const RankMap& current, const RankMap2& previous, const Graph& g)
        {
            if (--iters == 0)
                return true;
            if (!tol)
                return false;
            using rank_type = typename property_traits< RankMap >::value_type;
            rank_type sum_abs(0);
            for (auto v : boost::make_iterator_range(vertices(g)))
                sum_abs += std::abs(get(current, v) - get(previous, v));
            return sum_abs*num_vertices(g)<tol;
        }
        std::size_t iters;
        double tol;
    };

    namespace personalized_page_rank_detail
    {
        template <
            typename Graph,
            typename WeightMap,
            typename PersonalizationMap,
            typename RankMap,
            typename RankMap2 >
        void personalized_page_rank_step(
            const Graph& g,
            WeightMap weight_map,
            PersonalizationMap personalization_map,
            RankMap from_rank,
            RankMap2 to_rank,
            typename property_traits< RankMap >::value_type damping,
            incidence_graph_tag)
        {
            using rank_type = typename property_traits< RankMap >::value_type;
            rank_type l1_norm(0); // Computing the norm simultaneously avoids an extra summing iteration.

            // Initialize the constant part of maps.
            for (auto v : boost::make_iterator_range(vertices(g))) 
            {
                auto v_constant = rank_type(1 - damping) * get(personalization_map, v);
                put(to_rank, v, v_constant);
                l1_norm += v_constant;
            }
            
            // Accumulate from neighbors.
            for (auto u : boost::make_iterator_range(vertices(g)))
            {
                rank_type u_rank_factor = damping * get(from_rank, u);
                rank_type l1_accumulated_norm(0); // TBD: Consider making l1_norm volatile to reduce accumulation errors.
                for (auto e : boost::make_iterator_range(out_edges(u, g))) 
                {
                    auto v = target(e, g);
                    rank_type u_rank_out = get(weight_map, e)*u_rank_factor;
                    put(to_rank, v, get(to_rank, v) + u_rank_out);
                    l1_accumulated_norm += u_rank_out;
                }
                l1_norm += l1_accumulated_norm;
            }
            // If there are negative edge weights, or if negative damping is used, l1_norm could be zero or near-zero. 
            // Division in those cases is conceptually correct for floating point weights, and actually expected behavior. 
            // That said, such edge cases are impossible to arise for all typical algorithm uses.
            for (auto v : boost::make_iterator_range(vertices(g))) 
                put(to_rank, v, get(to_rank, v)/l1_norm); 
        }

        template <
            typename Graph,
            typename WeightMap,
            typename PersonalizationMap,
            typename RankMap,
            typename RankMap2 >
        void personalized_page_rank_step(
            const Graph& g,
            WeightMap weight_map,
            PersonalizationMap personalization_map,
            RankMap from_rank,
            RankMap2 to_rank,
            typename property_traits< RankMap >::value_type damping,
            bidirectional_graph_tag)
        {
            using damping_type = typename property_traits< RankMap >::value_type;
            damping_type l1_norm(0); // Computing the norm simultaneously avoids an extra summing iteration.
            for (auto v : boost::make_iterator_range(vertices(g)))
            {
                damping_type rank(0);
                for (auto e : boost::make_iterator_range(in_edges(v, g))) 
                    rank += get(from_rank, source(e, g))*get(weight_map, e);
                auto v_score = (damping_type(1) - damping) * get(personalization_map, v) + damping * rank;
                put(to_rank, v, v_score);
                l1_norm += v_score;
            }
            // See above function for potential division by zero comments.
            for (auto v : boost::make_iterator_range(vertices(g))) 
                put(to_rank, v, get(to_rank, v)/l1_norm);
        }
    } // end namespace personalized_page_rank_detail

    template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap,
        typename Done,
        typename RankMap2 >
    Done personalized_page_rank(
        const Graph& g,
        WeightMap weight_map,
        PersonalizationMap personalization_map,
        RankMap rank_map,
        Done done,
        typename property_traits< RankMap >::value_type damping,
        RankMap2 rank_map2
        BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
    {
        using rank_type = typename property_traits< PersonalizationMap >::value_type;
        rank_type personalization_norm(0);
        for (auto v : boost::make_iterator_range(vertices(g))) 
            personalization_norm += get(personalization_map, v);

        // TBD: This implementation couples iterators when possible under reduced L1 cache invalidation assumptions,
        // but this is not necessarily the case because we may be grabbing 2x memory lanes each time to write there.
        // Could investigate which pattern is faster in the future.
        for (auto v : boost::make_iterator_range(vertices(g)))
        {
            rank_type value = get(personalization_map, v)/personalization_norm;
            put(personalization_map, v, value);
            put(rank_map, v, value);
        }
        
        bool to_map_2 = true;
        do
        {
            typedef typename graph_traits< Graph >::traversal_category category;
            if (to_map_2)
                personalized_page_rank_detail::personalized_page_rank_step(g, weight_map, personalization_map, rank_map, rank_map2, damping, category());
            else
                personalized_page_rank_detail::personalized_page_rank_step(g, weight_map, personalization_map, rank_map2, rank_map, damping, category());
            to_map_2 = !to_map_2;
        } 
        while ((to_map_2  && !done(rank_map,  rank_map2, g)) || (!to_map_2 && !done(rank_map2, rank_map,  g))); // Done may not be symmetric.
        
        // Now multiply the result with personalization_norm to restore the order of magnitude and store it in rank_map.
        // Also restore the original personalization_map's magnitude for reuse (this is lossy up to numerical tolerance 
        // but leaner than making a copy).
        if (!to_map_2)
        {
            for (auto v : boost::make_iterator_range(vertices(g)))
            {
                put(rank_map, v, get(rank_map2, v)*personalization_norm);
                put(personalization_map, v, get(personalization_map, v)*personalization_norm);
            }
        }
        else
        {
            for (auto v : boost::make_iterator_range(vertices(g)))
            {
                put(rank_map, v, get(rank_map, v)*personalization_norm);
                put(personalization_map, v, get(personalization_map, v)*personalization_norm);
            }
        }
        return done;
    }

    template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap,
        typename Done >
    Done personalized_page_rank(
        const Graph& g,
        WeightMap weight_map,
        PersonalizationMap personalization_map,
        RankMap rank_map,
        Done done,
        typename property_traits< RankMap >::value_type damping)
    {
        using rank_type = typename property_traits< RankMap >::value_type;
        std::vector< rank_type > ranks2(num_vertices(g));
        return personalized_page_rank(g, weight_map, personalization_map, rank_map, done, damping,
            make_iterator_property_map(ranks2.begin(), get(vertex_index, g)));
    }

    template < typename Graph, typename PersonalizationMap, typename RankMap >
    rank_convergence personalized_page_rank(
        const Graph& g,
        PersonalizationMap personalization_map,
        RankMap rank_map,
        typename property_traits< RankMap >::value_type damping=0.85)
    {
        // This is the most traditional personalized PageRank implementation, with minimized signature.
        using Edge = typename graph_traits<Graph>::edge_descriptor;
        using rank_type = typename property_traits< RankMap >::value_type;
        std::vector< rank_type > ranks2(num_vertices(g));
        auto markovian_weights = make_function_property_map<Edge, double>([&g](Edge e){ return 1.0 / out_degree(source(e, g), g); });
        return personalized_page_rank(g,
            markovian_weights, 
            personalization_map, 
            rank_map, 
            rank_convergence(100, 1.E-9), 
            damping,
            make_iterator_property_map(ranks2.begin(), get(vertex_index, g)));
    }

}
} // end namespace boost::graph

#endif // BOOST_GRAPH_PERSONALIZED_PAGE_RANK_HPP

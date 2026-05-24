// Copyright 2026 Emmanouil Krasanakis

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Emmanouil Krasanakis

#ifndef BOOST_GRAPH_PAGE_RANK_HPP
#define BOOST_GRAPH_PAGE_RANK_HPP

#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/overloading.hpp>
#include <boost/graph/detail/mpi_include.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/throw_exception.hpp>
#include <vector>

namespace boost
{
    namespace graph
    {

        template < typename Graph >
        auto asymmetric_normalization_weights(const Graph& g) {
            using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
            return make_function_property_map<Edge, double>(
                [&g](Edge e) {
                    auto u = source(e, g);
                    return 1.0 / out_degree(u, g);
                });
        }

        template < typename Graph >
        auto symmetric_normalization_weights(const Graph& g) {
            using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
            return make_function_property_map<Edge, double>(
                [&g](Edge e) {
                    auto u = source(e, g);
                    auto v = target(e, g);
                    return 1.0 / std::sqrt(out_degree(u, g) * out_degree(v, g));
                });
        }

        class BOOST_SYMBOL_VISIBLE vertex_map_not_converged : public std::exception
        {
        };

        struct vertex_map_convergence
        {
            explicit vertex_map_convergence(std::size_t n, double tol=0) : n(n), tol(tol) {} // allowing tolerance for early stopping

            template < typename RankMap, typename RankMap2, typename Graph >
            bool operator()(
                const RankMap& rank_map,
                const RankMap2& rank_map2,
                const Graph& g)
            {
                if ( n-- == 0 )
                {
                    if(tol) boost::throw_exception(vertex_map_not_converged());
                    return true;
                }
                if (!tol)
                    return false;

                typedef typename property_traits< RankMap >::value_type rank_type;
                rank_type sum_abs(0);
                BGL_FORALL_VERTICES_T(v, g, Graph)
                {
                    rank_type difference = get(rank_map, v)-get(rank_map2, v);
                    if(difference<0)
                        difference = -difference;
                    sum_abs += difference;
                }
                return sum_abs<tol;
            }

            std::size_t get_remaining_iters() const
            {
                return n;
            }

        private:
            std::size_t n;
            double tol;
        };

        namespace detail
        {
            template <
            typename Graph,
            typename WeightMap,
            typename PersonalizationMap,
            typename RankMap,
            typename RankMap2 >
            void personalized_page_rank_step(
                const Graph& g,
                const WeightMap& weight_map,
                PersonalizationMap personalization_map,
                RankMap from_rank,
                RankMap2 to_rank,
                typename property_traits< RankMap >::value_type damping,
                incidence_graph_tag)
            {
                typedef typename property_traits< RankMap >::value_type rank_type;
                rank_type l1_norm(0); // Computing the norm simultaneously avoids an extra summing iteration.

                // Initialize the constant part of maps
                BGL_FORALL_VERTICES_T(v, g, Graph) {
                    auto v_constant = rank_type(1 - damping) * get(personalization_map, v);
                    put(to_rank, v, v_constant);
                    l1_norm += v_constant;
                }

                BGL_FORALL_VERTICES_T(u, g, Graph)
                {
                    rank_type u_rank_factor = damping * get(from_rank, u);
                    rank_type l1_accumulated_norm(0); // TBD: Consider making l1_norm volatile even, to reduce accumulation errors.
                    BGL_FORALL_OUTEDGES_T(u, e, g, Graph)
                    {
                        auto v = target(e, g);
                        rank_type u_rank_out = get(weight_map, e)*u_rank_factor;
                        put(to_rank, v, get(to_rank, v) + u_rank_out);
                        l1_accumulated_norm += u_rank_out;
                    }
                    l1_norm += l1_accumulated_norm;
                }
                BGL_FORALL_VERTICES_T(v, g, Graph) put(to_rank, v, get(to_rank, v)/l1_norm);
            }

            template <
            typename Graph,
            typename WeightMap,
            typename PersonalizationMap,
            typename RankMap,
            typename RankMap2 >
            void page_rank_step(
                const Graph& g,
                const WeightMap& weight_map,
                PersonalizationMap personalization_map,
                RankMap from_rank,
                RankMap2 to_rank,
                typename property_traits< RankMap >::value_type damping,
                bidirectional_graph_tag)
            {
                typedef typename property_traits< RankMap >::value_type damping_type;
                damping_type l1_norm(0); // Computing the norm simultaneously avoids an extra summing iteration.
                BGL_FORALL_VERTICES_T(v, g, Graph)
                {
                    typename property_traits< RankMap >::value_type rank(0);
                    BGL_FORALL_INEDGES_T(v, e, g, Graph) rank += get(from_rank, source(e, g))*get(weight_map, e);//get(from_rank, source(e, g)) / out_degree(source(e, g), g);
                    auto v_score = (damping_type(1) - damping) * get(personalization_map, v) + damping * rank;
                    put(to_rank, v, v_score);
                    l1_norm += v_score;
                }
                BGL_FORALL_VERTICES_T(v, g, Graph) put(to_rank, v, get(to_rank, v)/l1_norm);
            }
        } // end namespace detail

        template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap,
        typename Done,
        typename RankMap2 >
        void personalized_page_rank(
            const Graph& g,
            const WeightMap& weight_map,
            PersonalizationMap personalization_map,
            RankMap rank_map,
            Done& done,
            typename property_traits< RankMap >::value_type damping,
            typename graph_traits< Graph >::vertices_size_type n,
            RankMap2 rank_map2
            BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
        {
            typedef typename property_traits< PersonalizationMap >::value_type rank_type;

            rank_type personalization_norm(0);
            BGL_FORALL_VERTICES_T(v, g, Graph) personalization_norm += get(personalization_map, v);
            if(!personalization_norm) personalization_norm = 1; // TBD: perhaps throw error here

            // TBD: This implementation couples iterators when possible under reduced L1 cache invalidation assumptions,
            // but this is not ncessarily the case due to writes. Perhaps investigate which pattern is faster.
            BGL_FORALL_VERTICES_T(v, g, Graph)
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
                    detail::personalized_page_rank_step(g, weight_map, personalization_map, rank_map, rank_map2, damping, category());
                else
                    detail::personalized_page_rank_step(g, weight_map, personalization_map, rank_map2, rank_map, damping, category());
                to_map_2 = !to_map_2;
            } while (!done(rank_map, rank_map2, g)); // Symmetric call signature that should be unaffected by mode.

            if (!to_map_2)
            {
                BGL_FORALL_VERTICES_T(v, g, Graph)
                {
                    put(rank_map, v, get(rank_map2, v)*personalization_norm);
                    put(personalization_map, v, get(personalization_map, v)*personalization_norm); // restore to near-exact original
                }
            }
            else
            {
                BGL_FORALL_VERTICES_T(v, g, Graph)
                {
                    put(rank_map, v, get(rank_map, v)*personalization_norm);
                    put(personalization_map, v, get(personalization_map, v)*personalization_norm); // restore to near-exact original
                }
            }
        }

        template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap,
        typename Done >
        void personalized_page_rank(
            const Graph& g,
            const WeightMap& weight_map,
            PersonalizationMap personalization_map,
            RankMap rank_map,
            Done& done,
            typename property_traits< RankMap >::value_type damping,
            typename graph_traits< Graph >::vertices_size_type n)
        {
            typedef typename property_traits< RankMap >::value_type rank_type;

            std::vector< rank_type > ranks2(num_vertices(g));
            personalized_page_rank(g, weight_map, personalization_map, rank_map, done, damping, n,
                                   make_iterator_property_map(ranks2.begin(), get(vertex_index, g)));
        }

        template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap,
        typename Done >
        inline void personalized_page_rank(
            const Graph& g,
            const WeightMap& weight_map,
            PersonalizationMap personalization_map,
            RankMap rank_map,
            Done& done,
            typename property_traits< RankMap >::value_type damping = 0.85)
        {
            personalized_page_rank(g, weight_map, personalization_map, rank_map, done, damping, num_vertices(g));
        }

        template <
        typename Graph,
        typename WeightMap,
        typename PersonalizationMap,
        typename RankMap >
        inline void personalized_page_rank(
            const Graph& g,
            const WeightMap& weight_map,
            PersonalizationMap personalization_map,
            RankMap rank_map,
            typename property_traits< RankMap >::value_type damping = 0.85)
        {
            std::size_t n_iters(1.0/(1-damping)+0.5); // accounts for "most" random walks
            auto convergence = vertex_map_convergence(n_iters);
            personalized_page_rank(g, weight_map, personalization_map, rank_map, convergence, damping, num_vertices(g));
        }

        template <
        typename Graph,
        typename PersonalizationMap,
        typename RankMap >
        inline void personalized_page_rank(
            const Graph& g,
            PersonalizationMap personalization_map,
            RankMap rank_map,
            typename property_traits< RankMap >::value_type damping = 0.85)
        {
            personalized_page_rank(g, asymmetric_normalization(g), personalization_map, rank_map);
        }

    }
} // end namespace boost::graph

#include BOOST_GRAPH_MPI_INCLUDE(<boost/graph/distributed/personalized_page_rank.hpp>)

#endif // BOOST_GRAPH_PAGE_RANK_HPP

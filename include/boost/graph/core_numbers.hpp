// Copyright 2007 Stanford University
// Authors: David Gleich
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_CORE_NUMBERS_HPP
#define BOOST_GRAPH_CORE_NUMBERS_HPP

#include <boost/pending/mutable_queue.hpp>

/*
 * KCore
 *
 * Requirement: IncidenceGraph
 */

namespace boost {

// A linear time O(m) algorithm to compute the in-degree core number
// of a graph for unweighted graphs.
//
// and a O((n+m) log n) algorithm to compute the in-edge-weight core
// numbers of a weighted graph.
//
// The linear algorithm comes from:
//      @article{DBLP:journals/corr/cs-DS-0310049,
//          author = {Vladimir Batagelj and Matjaz Zaversnik},
//          title     = {An O(m) Algorithm for Cores Decomposition of Networks},
//          journal   = {The Computing Research Repository (CoRR)},
//          volume    = {cs.DS/0310049},
//          year      = {2003},
//          ee        = {http://arxiv.org/abs/cs.DS/0310049},
//          bibsource = {DBLP, http://dblp.uni-trier.de}
//      }

namespace detail {
    // implement a constant_property_map to simplify compute_in_degree
    // for the weighted and unweighted case
    // this is based on dummy property map
    // TODO: This is virtually the same as constant_property_map in
    // graph/property_maps. Perhaps we should be using that instead of this..
    template <typename ValueType>
    class constant_value_property_map
        : public boost::put_get_helper<ValueType,
            constant_value_property_map<ValueType>
        >
    {
    public:
        typedef void key_type;
        typedef ValueType value_type;
        typedef const ValueType& reference;
        typedef boost::readable_property_map_tag category;
        inline constant_value_property_map(ValueType cc) : c(cc) { }
        inline constant_value_property_map(const constant_value_property_map<ValueType>& x)
            : c(x.c) { }
        template <class Vertex>
        inline reference operator[](Vertex) const { return c; }
    protected:
        ValueType c;
    };

    // the core numbers start as the indegree or inweight.  This function
    // will initialize these values
    template <typename Graph, typename CoreMap, typename EdgeWeightMap>
    void compute_in_degree_map(Graph& g, CoreMap d, EdgeWeightMap wm)
    {
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;
        typename graph_traits<Graph>::out_edge_iterator ei,ei_end;
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            put(d,*vi,0);
        }
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            for (tie(ei,ei_end) = out_edges(*vi,g); ei!=ei_end; ++ei) {
                put(d,target(*ei,g),get(d,target(*ei,g))+get(wm,*ei));
            }
        }
    }

    // the version for weighted graphs is a little different
    template <typename Graph, typename CoreMap,
        typename EdgeWeightMap, typename MutableQueue>
    typename property_traits<CoreMap>::value_type
    core_numbers_impl(Graph& g, CoreMap c, EdgeWeightMap wm, MutableQueue& Q)
    {
        typename property_traits<CoreMap>::value_type v_cn = 0;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;
        while (!Q.empty())
        {
            // remove v from the Q, and then decrease the core numbers
            // of its successors
            vertex v = Q.top();
            Q.pop();
            v_cn = get(c,v);
            typename graph_traits<Graph>::out_edge_iterator oi,oi_end;
            for (tie(oi,oi_end) = out_edges(v,g); oi!=oi_end; ++oi) {
                vertex u = target(*oi,g);
                // if c[u] > c[v], then u is still in the graph,
                if (get(c,u) > v_cn) {
                    // remove the edge
                    put(c,u,get(c,u)-get(wm,*oi));
                    Q.update(u);
                }
            }
        }
        return (v_cn);
    }

    template <typename Graph, typename CoreMap, typename EdgeWeightMap,
              typename IndexMap>
    typename property_traits<CoreMap>::value_type
    core_numbers_dispatch(Graph&g, CoreMap c, EdgeWeightMap wm, IndexMap im)
    {
        typedef typename property_traits<CoreMap>::value_type D;
        typedef std::less<D> Cmp;
        typedef indirect_cmp<CoreMap,Cmp > IndirectCmp;
        IndirectCmp icmp(c, Cmp());
        // build the mutable queue
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;
        typedef mutable_queue<vertex, std::vector<vertex>, IndirectCmp,
            IndexMap> MutableQueue;
        MutableQueue Q(num_vertices(g), icmp, im);
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            Q.push(*vi);
        }
        return core_numbers_impl(g, c, wm, Q);
    }

    // the version for the unweighted case
    // for this functions CoreMap must be initialized
    // with the in degree of each vertex
    template <typename Graph, typename CoreMap, typename PositionMap>
    typename property_traits<CoreMap>::value_type
    core_numbers_impl(Graph& g, CoreMap c, PositionMap pos)
    {
        typedef typename graph_traits<Graph>::vertices_size_type size_type;
        typedef typename graph_traits<Graph>::degree_size_type degree_type;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;

        // store the vertex core numbers
        typename property_traits<CoreMap>::value_type v_cn = 0;

        // compute the maximum degree (degrees are in the coremap)
        typename graph_traits<Graph>::degree_size_type max_deg = 0;
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            max_deg = (std::max)(max_deg, get(c,*vi));
        }
        // store the vertices in bins by their degree
        // allocate two extra locations to ease boundary cases
        std::vector<size_type> bin(max_deg+2);
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            ++bin[get(c,*vi)];
        }
        // this loop sets bin[d] to the starting position of vertices
        // with degree d in the vert array for the bucket sort
        size_type cur_pos = 0;
        for (degree_type cur_deg = 0; cur_deg < max_deg+2; ++cur_deg) {
            degree_type tmp = bin[cur_deg];
            bin[cur_deg] = cur_pos;
            cur_pos += tmp;
        }
        // perform the bucket sort with pos and vert so that
        // pos[0] is the vertex of smallest degree
        std::vector<vertex> vert(num_vertices(g));
        for (tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi) {
            vertex v=*vi;
            size_type p=bin[get(c,v)];
            put(pos,v,p);
            vert[p]=v;
            ++bin[get(c,v)];
        }
        // we ``abused'' bin while placing the vertices, now,
        // we need to restore it
        std::copy(boost::make_reverse_iterator(bin.end()-2),
            boost::make_reverse_iterator(bin.begin()),
            boost::make_reverse_iterator(bin.end()-1));
        // now simulate removing the vertices
        for (size_type i=0; i < num_vertices(g); ++i) {
            vertex v = vert[i];
            v_cn = get(c,v);
            typename graph_traits<Graph>::out_edge_iterator oi,oi_end;
            for (tie(oi,oi_end) = out_edges(v,g); oi!=oi_end; ++oi) {
                vertex u = target(*oi,g);
                // if c[u] > c[v], then u is still in the graph,
                if (get(c,u) > v_cn) {
                    degree_type deg_u = get(c,u);
                    degree_type pos_u = get(pos,u);
                    // w is the first vertex with the same degree as u
                    // (this is the resort operation!)
                    degree_type pos_w = bin[deg_u];
                    vertex w = vert[pos_w];
                    if (u!=v) {
                        // swap u and w
                        put(pos,u,pos_w);
                        put(pos,w,pos_u);
                        vert[pos_w] = u;
                        vert[pos_u] = w;
                    }
                    // now, the vertices array is sorted assuming
                    // we perform the following step
                    // start the set of vertices with degree of u
                    // one into the future (this now points at vertex
                    // w which we swapped with u).
                    ++bin[deg_u];
                    // we are removing v from the graph, so u's degree
                    // decreases
                    put(c,u,get(c,u)-1);
                }
            }
        }
        return v_cn;
    }

} // namespace detail

template <typename Graph, typename CoreMap>
typename property_traits<CoreMap>::value_type
core_numbers(Graph& g, CoreMap c)
{
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    detail::compute_in_degree_map(g,c,
        detail::constant_value_property_map<
            typename property_traits<CoreMap>::value_type>(1) );
    return detail::core_numbers_impl(g,c,
        make_iterator_property_map(
            std::vector<size_type>(num_vertices(g)).begin(),get(vertex_index, g))
    );
}

template <typename Graph, typename CoreMap, typename EdgeWeightMap,
          typename VertexIndexMap>
typename property_traits<CoreMap>::value_type
core_numbers(Graph& g, CoreMap c, EdgeWeightMap wm, VertexIndexMap vim)
{
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    detail::compute_in_degree_map(g,c,wm);
    return detail::core_numbers_dispatch(g,c,wm,vim);
}

template <typename Graph, typename CoreMap, typename EdgeWeightMap>
typename property_traits<CoreMap>::value_type
core_numbers(Graph& g, CoreMap c, EdgeWeightMap wm)
{
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    detail::compute_in_degree_map(g,c,wm);
    return detail::core_numbers_dispatch(g,c,wm,get(vertex_index,g));
}

template <typename Graph, typename CoreMap>
typename property_traits<CoreMap>::value_type
weighted_core_numbers(Graph& g, CoreMap c)
{ return core_numbers(g,c,get(edge_weight,g)); }

} // namespace boost

#endif // BOOST_GRAPH_CORE_NUMBERS_HPP


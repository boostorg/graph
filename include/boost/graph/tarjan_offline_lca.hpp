/* boost/graph/tarjan_offline_lca.hpp header file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_GRAPH_TARJAN_OFFLINE_LCA_HPP
#define BOOST_GRAPH_TARJAN_OFFLINE_LCA_HPP

/*
 * Defines the std::pair class template and its associated std::make_pair
 * function template.
 */
#include <utility>

/*
 * Defines the std::vector class template.
 */
#include <vector>

/*
 * Defines the std::map class template.
 */
#include <map>

/*
 * Defines the boost::function_requires function template and the basic concept
 * check templates.
 */
#include <boost/concept_check.hpp>

/*
 * Defines the boost::associative_property_map and boost::property_traits class
 * templates and the boost::get and boost::put function templates.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::tie function template.
 */
#include <boost/utility.hpp>

/*
 * Defines the boost::is_same class template.
 */
#include <boost/type_traits/is_same.hpp>

/*
 * Defines the boost::disjoint_sets_with_storage class template.
 */
#include <boost/pending/disjoint_sets.hpp>

/*
 * Defines the BGL concept check templates.
 */
#include <boost/graph/graph_concepts.hpp>

/*
 * Defines the boost::graph_traits class template.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the boost::vertex_index_t property tag.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::bgl_named_params class template and its helper function
 * templates.
 */
#include <boost/graph/named_function_params.hpp>

/*
 * Defines the boost::base_visitor class template and the boost::null_visitor
 * class.
 */
#include <boost/graph/visitors.hpp>

/*
 * Defines the boost::depth_first_search and boost::make_dfs_visitor function
 * templates.
 */
#include <boost/graph/depth_first_search.hpp>

/*
 * The graph type and its associated non-member function templates must be
 * defined externally.
 */
//#include <boost/graph/adjacency_list.hpp>

/*
 * Defines the boost::inverse_index_map class template.
 */
#include <boost/graph/map_utility.hpp>

namespace boost {

  namespace detail {

/*
    template <typename DisjointSets, typename PredecessorMap>
    class tarjan_offline_lca_set_maker :
      public base_visitor<
        tarjan_offline_lca_set_maker<DisjointSets,PredecessorMap> >
    {
     public:
        typedef on_discover_vertex event_filter;

     private:
        DisjointSets&   m_dsets;
        PredecessorMap& m_ancestor_map;

     public:
        explicit tarjan_offline_lca_set_maker(
            DisjointSets& dsets, PredecessorMap& ancestor_map) :
            m_dsets(dsets), m_ancestor_map(ancestor_map)
        {
        }

        template <typename Graph>
        void operator()(
            typename graph_traits<Graph>::vertex_descriptor u, Graph& g)
        {
            m_dsets.make_set(u);
            put(m_ancestor_map, m_dsets.find_set(u), u);
        }
    };

    template <typename AncestorMatrix, typename DisjointSets,
              typename PredecessorMap, typename VertexColorMap>
    class tarjan_offline_lca_ancestor_mapper :
      public base_visitor<
        tarjan_offline_lca_ancestor_mapper<AncestorMatrix,DisjointSets,
                                           PredecessorMap,VertexColorMap> >
    {
     public:
        typedef on_finish_vertex event_filter;

     private:
        typedef typename property_traits<VertexColorMap>::value_type
                VertexColorValue;
        typedef color_traits<VertexColorValue>
                VertexColor;

        AncestorMatrix& m_ancestor_matrix;
        DisjointSets&   m_dsets;
        PredecessorMap& m_ancestor_map;
        VertexColorMap& m_color_map;

     public:
        tarjan_offline_lca_ancestor_mapper(
            AncestorMatrix& ancestor_matrix, DisjointSets& dsets,
            PredecessorMap& ancestor_map, VertexColorMap& color_map) :
            m_ancestor_matrix(ancestor_matrix), m_dsets(dsets),
            m_ancestor_map(ancestor_map), m_color_map(color_map)
        {
        }

        template <typename Graph>
        void operator()(
            typename graph_traits<Graph>::vertex_descriptor u, Graph& g)
        {
            typename graph_traits<Graph>::adjacency_iterator ai, aend;

            for (tie(ai, aend) = adjacent_vertices(u, g); ai != aend; ++ai)
            {
                m_dsets.union_set(u, *ai);
                put(m_ancestor_map, m_dsets.find_set(u), u);
std::cout << "    union(" << u << "," << *ai << ")=";
std::cout << get(m_ancestor_map, m_dsets.find_set(u)) << std::endl;
            }

            typename graph_traits<Graph>::vertex_iterator vi, vend;
            VertexColorValue color;

            for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
            {
                color = get(m_color_map, *vi);

                if (color == VertexColor::gray())
                {
std::cout << "  anc(" << u << ")=" << get(m_ancestor_map, m_dsets.find_set(u));
std::cout << ", anc(" << *vi << ")=" << get(m_ancestor_map, m_dsets.find_set(*vi)) << std::endl;
                    // *vi == u or is an ancestor of u.
                    m_ancestor_matrix(u, *vi) = m_ancestor_matrix(*vi, u) = *vi;
                }
                else if (color == VertexColor::black())
                {
std::cout << "  set(" << u << ")=" << m_dsets.find_set(u);
std::cout << ", anc(" << u << ")=" << get(m_ancestor_map, m_dsets.find_set(u));
std::cout << ", set(" << *vi << ")=" << m_dsets.find_set(*vi);
std::cout << ", anc(" << *vi << ")=" << get(m_ancestor_map, m_dsets.find_set(*vi)) << std::endl;
                    // *vi and u are in different subtrees.
                    m_ancestor_matrix(u, *vi) = m_ancestor_matrix(*vi, u)
                                              = get(m_ancestor_map,
                                                    m_dsets.find_set(*vi));
                }
            }
        }
    };
*/

    template <typename Graph, typename AncestorMatrix, typename DisjointSets,
              typename PredecessorMap, typename VertexColorMap>
    void tarjan_offline_lca_impl(
        const Graph& g, typename graph_traits<Graph>::vertex_descriptor u,
        AncestorMatrix& ancestor_matrix, DisjointSets& dsets,
        PredecessorMap ancestor_map, VertexColorMap color_map)
    {
        typedef typename property_traits<VertexColorMap>::value_type
                VertexColorValue;
        typedef color_traits<VertexColorValue>
                VertexColor;

        dsets.make_set(u);
        put(ancestor_map, dsets.find_set(u), u);

        typename graph_traits<Graph>::adjacency_iterator ai, aend;

        for (tie(ai, aend) = adjacent_vertices(u, g); ai != aend; ++ai)
        {
            tarjan_offline_lca_impl(g, *ai, ancestor_matrix, dsets,
                                    ancestor_map, color_map);
            dsets.union_set(u, *ai);
            put(ancestor_map, dsets.find_set(u), u);
        }

        put(color_map, u, VertexColor::black());

        typename graph_traits<Graph>::vertex_iterator vi, vend;

        for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
        {
            if (get(color_map, *vi) == VertexColor::black())
            {
                ancestor_matrix(u, *vi) = ancestor_matrix(*vi, u)
                                        = get(ancestor_map,
                                              dsets.find_set(*vi));
            }
        }
    }
  }  // namespace detail

/*
 * The kitchen-sink variant.
 */
template <typename Graph, typename AncestorMatrix, typename DisjointSets,
          typename PredecessorMap, typename VertexColorMap,
          typename EventVisitorList>
void tarjan_offline_lca(
    const Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
    AncestorMatrix& ancestor_matrix, DisjointSets dsets,
    PredecessorMap ancestor_map, VertexColorMap color_map, EventVisitorList vis)
{
    typedef graph_traits<Graph>
            GraphTraits;
    typedef typename property_traits<VertexColorMap>::value_type
            VertexColorValue;
    typedef color_traits<VertexColorValue>
            VertexColor;
/*
    typedef detail::tarjan_offline_lca_set_maker<DisjointSets,PredecessorMap>
            Visitor1;
    typedef detail::tarjan_offline_lca_ancestor_mapper<
              AncestorMatrix,DisjointSets,PredecessorMap,VertexColorMap>
            Visitor2;
    typedef dfs_visitor<
              std::pair<Visitor1,std::pair<Visitor2,EventVisitorList> > >
            DFSVisitor;
*/

    typename GraphTraits::vertex_iterator vi, vend;

    for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
    {
        put(color_map, *vi, VertexColor::white());
    }

/*
    Visitor1   vis1(dsets, ancestor_map);
    Visitor2   vis2(ancestor_matrix, dsets, ancestor_map, color_map);
    DFSVisitor vis0 =
      make_dfs_visitor(
        std::make_pair(vis1, std::make_pair(vis2, vis)));

    dsets.make_set(source);
    put(ancestor_map, dsets.find_set(source), source);
    depth_first_search(g, vis0, color_map, source);
*/
    tarjan_offline_lca_impl(g, source, ancestor_matrix, dsets, ancestor_map,
                            color_map);
}

/*
 * The default variant.
 */
template <typename Graph, typename AncestorMatrix>
void tarjan_offline_lca(
    const Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
    AncestorMatrix& ancestor_matrix)
{
    typedef graph_traits<Graph>
            GraphTraits;
    typedef typename GraphTraits::vertex_descriptor
            Vertex;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef boost::associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename property_map<Graph,vertex_index_t>::type
            IndexMap;
    typedef inverse_index_map<Graph>
            InverseIndexMap;
    typedef boost::disjoint_sets_with_storage<IndexMap,const InverseIndexMap&>
            DisjointSets;

    IndexMap                        index_map = get(vertex_index_t(), g);
    InverseIndexMap                 inv_index_map(g);
    DisjointSets                    dsets(num_vertices(g), index_map,
                                          inv_index_map);
    VertexMap                       vert_map;
    PredecessorMap                  ancestor_map(vert_map);
    std::vector<default_color_type> color(num_vertices(g));
    null_visitor                    n_vis;

    tarjan_offline_lca(
        g, source, ancestor_matrix, dsets, ancestor_map,
        make_iterator_property_map(color.begin(), index_map, white_color),
        n_vis);
}
}  // namespace boost

#endif /* BOOST_GRAPH_TARJAN_OFFLINE_LCA_HPP */


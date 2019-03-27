
//=======================================================================
// Copyright 2008
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_METRIC_TSP_APPROX_HPP
#define BOOST_GRAPH_METRIC_TSP_APPROX_HPP

// metric_tsp_approx
// Generates an approximate tour solution for the traveling salesperson
// problem in polynomial time. The current algorithm guarantees a tour with a
// length at most as long as 2x optimal solution. The graph should have
// 'natural' (metric) weights such that the triangle inequality is maintained.
// Graphs must be fully interconnected.

// TODO:
// There are a couple of improvements that could be made.
// 1) Change implementation to lower uppper bound Christofides heuristic
// 2) Implement a less restrictive TSP heuristic (one that does not rely on
//    triangle inequality).
// 3) Determine if the algorithm can be implemented without creating a new
//    graph.

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/mpl/vector.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/declval.hpp>

#if defined(BOOST_NO_CXX11_DECLTYPE)
#include <boost/typeof/typeof.hpp>
#endif

namespace boost { namespace detail {

#if !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
    template <typename T, typename G>
    class is_tsp_tour_visitor_impl
    {
        typedef typename boost::remove_const<T>::type _m_T;

        template <typename B, typename P>
        static graph_yes_tag
            _check_visit_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().visit_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().visit_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_visit_v(...);

    public:
        typedef mpl::bool_<
            (
                sizeof(
                    is_tsp_tour_visitor_impl<T,G>::_check_visit_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            )
        > type;
    };

    template <typename T, typename G>
    struct is_tsp_tour_visitor
        : mpl::eval_if<
            is_bgl_graph<G>,
            is_tsp_tour_visitor_impl<T,G>,
            mpl::false_
        >::type
    {
    };

    typedef argument_with_graph_predicate<
        is_tsp_tour_visitor
    > tsp_tour_visitor_predicate;
#else   // defined(BOOST_NO_CXX11_DECLTYPE) && !defined(BOOST_TYPEOF_KEYWORD)
    typedef visitor_predicate tsp_tour_visitor_predicate;
#endif  // !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
}}
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>
#include <boost/graph/graph_as_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/lookup_edge.hpp>
#include <boost/throw_exception.hpp>

namespace boost
{
    // Define a concept for the concept-checking library.
    template <typename Visitor, typename Graph>
    struct TSPVertexVisitorConcept
    {
    private:
        Visitor vis_;
    public:
        typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

        BOOST_CONCEPT_USAGE(TSPVertexVisitorConcept)
        {
            Visitor vis(vis_);  // require copy construction
            Graph g(1);
            Vertex v(*vertices(g).first);
            vis.visit_vertex(v, g); // require visit_vertex
        }
    };

    // Tree visitor that keeps track of a preorder traversal of a tree
    // TODO: Consider migrating this to the graph_as_tree header.
    // TODO: Parameterize the underlying stores so it doesn't have to be a vector.
    template<typename Node, typename Tree> class PreorderTraverser
    {
    private:
        std::vector<Node>& path_;
    public:
        typedef typename std::vector<Node>::const_iterator const_iterator;

        PreorderTraverser(std::vector<Node>& p) : path_(p) {}

        void preorder(Node n, const Tree&)
        { path_.push_back(n); }

        void inorder(Node, const Tree&) const {}
        void postorder(Node, const Tree&) const {}

        const_iterator begin() const { return path_.begin(); }
        const_iterator end() const { return path_.end(); }
    };

    // Default tsp tour visitor that puts the tour in an OutputIterator
    template <typename OutItr>
    class tsp_tour_visitor
    {
        OutItr itr_;
    public:
        tsp_tour_visitor(OutItr itr)
            : itr_(itr)
        { }

        template <typename Vertex, typename Graph>
        void visit_vertex(Vertex v, const Graph&)
        {
            BOOST_CONCEPT_ASSERT((OutputIterator<OutItr, Vertex>));
            *itr_++ = v;
        }

    };

    // Tsp tour visitor that adds the total tour length.
    template<typename Graph, typename WeightMap, typename OutIter, typename Length>
    class tsp_tour_len_visitor
    {
        typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
        BOOST_CONCEPT_ASSERT((OutputIterator<OutIter, Vertex>));

        OutIter iter_;
        Length& tourlen_;
        WeightMap& wmap_;
        Vertex previous_;

        // Helper function for getting the null vertex.
        Vertex null()
        { return graph_traits<Graph>::null_vertex(); }

    public:
        tsp_tour_len_visitor(Graph const&, OutIter iter, Length& l, WeightMap& map)
            : iter_(iter), tourlen_(l), wmap_(map), previous_(null())
        { }

        void visit_vertex(Vertex v, const Graph& g)
        {
            typedef typename graph_traits<Graph>::edge_descriptor Edge;

            // If it is not the start, then there is a
            // previous vertex
            if(previous_ != null())
            {
                // NOTE: For non-adjacency matrix graphs g, this bit of code
                // will be linear in the degree of previous_ or v. A better
                // solution would be to visit edges of the graph, but that
                // would require revisiting the core algorithm.
                Edge e;
                bool found;
                boost::tie(e, found) = lookup_edge(previous_, v, g);
                if(!found) {
                    BOOST_THROW_EXCEPTION(not_complete());
                }

                tourlen_ += wmap_[e];
            }

            previous_ = v;
            *iter_++ = v;
        }
    };

    // Object generator(s)
    template <typename OutIter>
    inline tsp_tour_visitor<OutIter>
    make_tsp_tour_visitor(OutIter iter)
    { return tsp_tour_visitor<OutIter>(iter); }

    template <typename Graph, typename WeightMap, typename OutIter, typename Length>
    inline tsp_tour_len_visitor<Graph, WeightMap, OutIter, Length>
    make_tsp_tour_len_visitor(Graph const& g, OutIter iter, Length& l, WeightMap map)
    { return tsp_tour_len_visitor<Graph, WeightMap, OutIter, Length>(g, iter, l, map); }
} // end namespace boost

#include <boost/graph/detail/dummy_output_iterator.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/core/enable_if.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/mpl/has_key.hpp>
#else
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/parameter/binding.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif
#endif

namespace boost {

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    BOOST_PARAMETER_FUNCTION(
      (
        boost::enable_if<
          typename mpl::eval_if<
            typename mpl::has_key<
              Args,
              boost::graph::keywords::tag::result
            >::type,
            mpl::true_,
            mpl::has_key<
              Args,
              boost::graph::keywords::tag::visitor
            >
          >::type,
          bool
        >
      ), metric_tsp_approx, ::boost::graph::keywords::tag,
      (required
        (graph, *(detail::argument_predicate<is_vertex_list_graph>))
      )
      (deduced
        (optional
          (result
            ,*(detail::argument_predicate<detail::is_iterator>)
            ,graph_detail::dummy_output_iterator()
          )
          (visitor
            ,*(detail::tsp_tour_visitor_predicate)
            ,make_tsp_tour_visitor(result)
          )
          (root_vertex
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_of_graph
              >
            )
            ,detail::get_default_starting_vertex(graph)
          )
          (weight_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_edge_property_map_of_graph
              >
            )
            ,detail::edge_or_dummy_property_map(graph, edge_weight)
          )
          (vertex_index_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_to_integer_map_of_graph
              >
            )
            ,detail::vertex_or_dummy_property_map(graph, vertex_index)
          )
        )
      )
    )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    template <
        typename VertexListGraph,
        typename WeightMap,
        typename VertexIndexMap,
        typename TSPVertexVisitor
    >
    void metric_tsp_approx_from_vertex(
        const VertexListGraph& graph,
        typename graph_traits<VertexListGraph>::vertex_descriptor root_vertex,
        WeightMap weight_map,
        VertexIndexMap vertex_index_map,
        TSPVertexVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<
                WeightMap, VertexIndexMap, TSPVertexVisitor
            >, mpl::true_
        >::type = mpl::true_()
#endif
    )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
    {
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
        typedef typename boost::remove_const<
            typename boost::remove_reference<graph_type>::type
        >::type VertexListGraph;
        typedef typename boost::remove_const<
            typename boost::remove_reference<vertex_index_map_type>::type
        >::type VertexIndexMap;
        typedef typename boost::remove_const<
            typename boost::remove_reference<visitor_type>::type
        >::type TSPVertexVisitor;
#endif

        BOOST_CONCEPT_ASSERT((VertexListGraphConcept<VertexListGraph>));
        BOOST_CONCEPT_ASSERT((
            TSPVertexVisitorConcept<TSPVertexVisitor, VertexListGraph>
        ));

        // Types related to the input graph (GVertex is a template parameter).
        typedef typename graph_traits<
            VertexListGraph
        >::vertex_descriptor GVertex;
        typedef typename graph_traits<
            VertexListGraph
        >::vertex_iterator GVItr;

        // We build a custom graph in this algorithm.
        typedef adjacency_list<
            vecS, vecS, directedS, no_property, no_property
        > MSTImpl;
        typedef graph_traits<MSTImpl>::vertex_descriptor Vertex;
        typedef graph_traits<MSTImpl>::vertex_iterator VItr;

        // And then re-cast it as a tree.
        typedef iterator_property_map<
            std::vector<Vertex>::iterator,
            property_map<MSTImpl, vertex_index_t>::type
        > ParentMap;
        typedef graph_as_tree<MSTImpl, ParentMap> Tree;
        typedef tree_traits<Tree>::node_descriptor Node;

        // A predecessor map.
        typedef std::vector<GVertex> PredMap;
        typedef iterator_property_map<
            typename PredMap::iterator, VertexIndexMap
        > PredPMap;

        PredMap preds(num_vertices(graph));
        PredPMap pred_pmap(preds.begin(), vertex_index_map);

        // Compute a spanning tree over the in put g.
        prim_minimum_spanning_tree(
            graph,
            pred_pmap,
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
            boost::graph::keywords::_root_vertex = root_vertex,
            boost::graph::keywords::_vertex_index_map = vertex_index_map,
            boost::graph::keywords::_weight_map = weight_map
#else
            boost::root_vertex(root_vertex)
            .vertex_index_map(vertex_index_map)
            .weight_map(weight_map)
#endif
        );

        // Build a MST using the predecessor map from prim mst
        MSTImpl mst(num_vertices(graph));
        std::size_t cnt = 0;
        std::pair<VItr, VItr> mst_verts(vertices(mst));

        for(typename PredMap::iterator vi(preds.begin()); vi != preds.end();
            ++vi, ++cnt)
        {
            if(get(vertex_index_map, *vi) != cnt) {
                add_edge(*next(mst_verts.first, get(vertex_index_map, *vi)),
                         *next(mst_verts.first, cnt), mst);
            }
        }

        // Build a tree abstraction over the MST.
        std::vector<Vertex> parent(num_vertices(mst));
        Tree t(mst, *vertices(mst).first,
            make_iterator_property_map(parent.begin(),
            get(vertex_index, mst)));

        // Create tour using a preorder traversal of the mst
        std::vector<Node> tour;
        PreorderTraverser<Node, Tree> tvis(tour);
        traverse_tree(get(vertex_index_map, root_vertex), t, tvis);

        std::pair<GVItr, GVItr> g_verts(vertices(graph));
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
        TSPVertexVisitor vis = visitor;
#endif

        for(PreorderTraverser<Node, Tree>::const_iterator curr(tvis.begin());
            curr != tvis.end(); ++curr)
        {
            // TODO: This is will be O(n^2) if vertex storage of g != vecS.
            GVertex v = *next(
                g_verts.first,
                get(get(vertex_index, mst), *curr)
            );
            vis.visit_vertex(v, graph);
        }

        // Connect back to the start of the tour
        vis.visit_vertex(root_vertex, graph);

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
        return true;
#endif
    }

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    BOOST_PARAMETER_FUNCTION(
      (bool), metric_tsp_approx_from_vertex, ::boost::graph::keywords::tag,
      (required
        (graph, *(detail::argument_predicate<is_vertex_list_graph>))
      )
      (deduced
        (required
          (visitor, *(detail::tsp_tour_visitor_predicate))
          (root_vertex
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_of_graph
              >
            )
          )
        )
        (optional
          (weight_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_edge_property_map_of_graph
              >
            )
            ,detail::edge_or_dummy_property_map(graph, edge_weight)
          )
          (vertex_index_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_to_integer_map_of_graph
              >
            )
            ,detail::vertex_or_dummy_property_map(graph, vertex_index)
          )
        )
      )
    )
    {
        return metric_tsp_approx(
            graph, visitor, root_vertex, weight_map, vertex_index_map
        );
    }

    BOOST_PARAMETER_FUNCTION(
      (bool),
      metric_tsp_approx_tour_from_vertex, ::boost::graph::keywords::tag,
      (required
        (graph, *(detail::argument_predicate<is_vertex_list_graph>))
      )
      (deduced
        (required
          (result, *(detail::argument_predicate<detail::is_iterator>))
          (root_vertex
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_of_graph
              >
            )
          )
        )
        (optional
          (weight_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_edge_property_map_of_graph
              >
            )
            ,detail::edge_or_dummy_property_map(graph, edge_weight)
          )
          (vertex_index_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_to_integer_map_of_graph
              >
            )
            ,detail::vertex_or_dummy_property_map(graph, vertex_index)
          )
        )
      )
    )
    {
        return metric_tsp_approx(
            graph, result, root_vertex, weight_map, vertex_index_map
        );
    }

    BOOST_PARAMETER_FUNCTION(
      (bool), metric_tsp_approx_tour, ::boost::graph::keywords::tag,
      (required
        (graph, *(detail::argument_predicate<is_vertex_list_graph>))
      )
      (deduced
        (required
          (result, *(detail::argument_predicate<detail::is_iterator>))
        )
        (optional
          (weight_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_edge_property_map_of_graph
              >
            )
            ,detail::edge_or_dummy_property_map(graph, edge_weight)
          )
          (vertex_index_map
            ,*(
              detail::argument_with_graph_predicate<
                detail::is_vertex_to_integer_map_of_graph
              >
            )
            ,detail::vertex_or_dummy_property_map(graph, vertex_index)
          )
        )
      )
    )
    {
        return metric_tsp_approx(graph, result, weight_map, vertex_index_map);
    }
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    template <
        typename VertexListGraph,
        typename WeightMap,
        typename TSPVertexVisitor
    >
    void metric_tsp_approx_from_vertex(
        VertexListGraph& g,
        typename graph_traits<VertexListGraph>::vertex_descriptor start,
        WeightMap w,
        TSPVertexVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<WeightMap, TSPVertexVisitor>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, start, w, get(vertex_index, g), vis);
    }

    template <typename VertexListGraph, typename TSPVertexVisitor>
    void metric_tsp_approx(
        VertexListGraph& g,
        TSPVertexVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::is_argument_pack<TSPVertexVisitor>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, *vertices(g).first,
            get(edge_weight, g), get(vertex_index, g), vis);
    }

    template <
        typename VertexListGraph,
        typename WeightMap,
        typename TSPVertexVisitor
    >
    void metric_tsp_approx(
        VertexListGraph& g,
        WeightMap w,
        TSPVertexVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<WeightMap, TSPVertexVisitor>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, *vertices(g).first, w,
            get(vertex_index, g), vis);
    }

    template <
        typename VertexListGraph,
        typename WeightMap,
        typename VertexIndexMap,
        typename TSPVertexVisitor
    >
    void metric_tsp_approx(
        VertexListGraph& g,
        WeightMap w,
        VertexIndexMap id,
        TSPVertexVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<
                WeightMap, VertexIndexMap, TSPVertexVisitor
            >, mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, *vertices(g).first, w, id, vis);
    }

    template <typename VertexListGraph, typename OutputIterator>
    void metric_tsp_approx_tour(
        VertexListGraph& g,
        OutputIterator o
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::is_argument_pack<OutputIterator>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, *vertices(g).first,
            get(edge_weight, g), get(vertex_index, g),
            tsp_tour_visitor<OutputIterator>(o));
    }

    template <
        typename VertexListGraph,
        typename WeightMap,
        typename OutputIterator
    >
    void metric_tsp_approx_tour(
        VertexListGraph& g,
        WeightMap w,
        OutputIterator o
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<WeightMap, OutputIterator>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, *vertices(g).first,
            w, tsp_tour_visitor<OutputIterator>(o));
    }

    template <typename VertexListGraph, typename OutputIterator>
    void metric_tsp_approx_tour_from_vertex(
        VertexListGraph& g,
        typename graph_traits<VertexListGraph>::vertex_descriptor start,
        OutputIterator o
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::is_argument_pack<OutputIterator>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, start, get(edge_weight, g),
            get(vertex_index, g), tsp_tour_visitor<OutputIterator>(o));
    }

    template <
        typename VertexListGraph,
        typename WeightMap,
        typename OutputIterator
    >
    void metric_tsp_approx_tour_from_vertex(
        VertexListGraph& g,
        typename graph_traits<VertexListGraph>::vertex_descriptor start,
        WeightMap w,
        OutputIterator o
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<WeightMap, OutputIterator>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        metric_tsp_approx_from_vertex(g, start, w, get(vertex_index, g),
            tsp_tour_visitor<OutputIterator>(o));
    }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    template <typename VertexListGraph, typename Args>
    inline void metric_tsp_approx(
        VertexListGraph& g,
        const Args& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()
    )
    {
        metric_tsp_approx_from_vertex(
            g,
            detail::get_default_starting_vertex(g),
            args[
                boost::graph::keywords::_weight_map |
                detail::edge_or_dummy_property_map(g, edge_weight)
            ],
            args[
                boost::graph::keywords::_vertex_index_map |
                detail::vertex_or_dummy_property_map(g, vertex_index)
            ],
            args[
                boost::graph::keywords::_visitor |
                make_tsp_tour_visitor(
                    args[
                        boost::graph::keywords::_result |
                        graph_detail::dummy_output_iterator()
                    ]
                )
            ]
        );
    }

    template <typename VertexListGraph, typename Args>
    inline void metric_tsp_approx_tour(
        VertexListGraph& g,
        const Args& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()
    )
    {
        metric_tsp_approx_from_vertex(
            g,
            detail::get_default_starting_vertex(g),
            args[
                boost::graph::keywords::_weight_map |
                detail::edge_or_dummy_property_map(g, edge_weight)
            ],
            args[
                boost::graph::keywords::_vertex_index_map |
                detail::vertex_or_dummy_property_map(g, vertex_index)
            ],
            make_tsp_tour_visitor(
                args[
                    boost::graph::keywords::_result |
                    graph_detail::dummy_output_iterator()
                ]
            )
        );
    }

    template <typename VertexListGraph, typename Args>
    inline void metric_tsp_approx_from_vertex(
        VertexListGraph& g,
        typename graph_traits<VertexListGraph>::vertex_descriptor start,
        const Args& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()
    )
    {
        metric_tsp_approx_from_vertex(
            g,
            start,
            args[
                boost::graph::keywords::_weight_map |
                detail::edge_or_dummy_property_map(g, edge_weight)
            ],
            args[
                boost::graph::keywords::_vertex_index_map |
                detail::vertex_or_dummy_property_map(g, vertex_index)
            ],
            args[
                boost::graph::keywords::_visitor |
                make_tsp_tour_visitor(
                    args[
                        boost::graph::keywords::_result |
                        graph_detail::dummy_output_iterator()
                    ]
                )
            ]
        );
    }

    template <typename VertexListGraph, typename Args>
    inline void metric_tsp_approx_tour_from_vertex(
        VertexListGraph& g,
        typename graph_traits<VertexListGraph>::vertex_descriptor start,
        const Args& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()
    )
    {
        metric_tsp_approx_from_vertex(
            g,
            start,
            args[
                boost::graph::keywords::_weight_map |
                detail::edge_or_dummy_property_map(g, edge_weight)
            ],
            args[
                boost::graph::keywords::_vertex_index_map |
                detail::vertex_or_dummy_property_map(g, vertex_index)
            ],
            make_tsp_tour_visitor(
                args[
                    boost::graph::keywords::_result |
                    graph_detail::dummy_output_iterator()
                ]
            )
        );
    }

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
    template < \
        typename Graph, typename TA \
        BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA) \
    > \
    inline void name( \
        Graph& g, const TA& ta \
        BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
        typename boost::enable_if< \
            parameter::are_tagged_arguments< \
                TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
            >, mpl::true_ \
        >::type = mpl::true_() \
    ) \
    { \
        name( \
            g, \
            parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta)) \
        ); \
    }

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD_WITH_VERTEX(z, n, name) \
    template < \
        typename Graph, typename TA \
        BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA) \
    > \
    inline void name( \
        Graph& g, typename graph_traits<Graph>::vertex_descriptor start, \
        const TA& ta \
        BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
        typename boost::enable_if< \
            parameter::are_tagged_arguments< \
                TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
            >, mpl::true_ \
        >::type = mpl::true_() \
    ) \
    { \
        name( \
            g, start, \
            parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta)) \
        ); \
    }

BOOST_PP_REPEAT_FROM_TO(
    1, 5, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, metric_tsp_approx
)
BOOST_PP_REPEAT_FROM_TO(
    1, 5, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, metric_tsp_approx_tour
)
BOOST_PP_REPEAT_FROM_TO(
    1, 4, BOOST_GRAPH_PP_FUNCTION_OVERLOAD_WITH_VERTEX,
    metric_tsp_approx_from_vertex
)
BOOST_PP_REPEAT_FROM_TO(
    1, 4, BOOST_GRAPH_PP_FUNCTION_OVERLOAD_WITH_VERTEX,
    metric_tsp_approx_tour_from_vertex
)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD_WITH_VERTEX
#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
} //boost

#endif // BOOST_GRAPH_METRIC_TSP_APPROX_HPP

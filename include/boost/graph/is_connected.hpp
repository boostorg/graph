#ifndef BOOST_GRAPH_IS_CONNECTED_HPP
#define BOOST_GRAPH_IS_CONNECTED_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/neighbor_bfs.hpp>
#include <boost/graph/create_condensation_graph.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace boost
{

struct connected_kind {
    enum weak_t { weak };
    enum unilateral_t { unilateral };
    enum strong_t { strong };
    enum unspecified_t { unspecified };
};

// used for static assert below
BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_value_type_ic, value_type, false)

template < typename Graph, typename ConnectedKind = connected_kind::unspecified_t >
inline bool is_connected(const Graph& g, ConnectedKind kind = connected_kind::unspecified)
{
    // Issue error message if these functions are called with a ColorMap.
    // This is to gracefully handle possible old usages of is_connected.
    // todo: consider checking for ColorValueConcept as well
    BOOST_STATIC_ASSERT_MSG(
        (mpl::not_< has_value_type_ic< ConnectedKind > >::value),
        "ColorMap argument to is_connected is deprecated. Omit the second "
        "argument for undirected graphs or specify connected_kind::strong "
        "to preserve the old behavior.");

    BOOST_STATIC_ASSERT_MSG(
        (mpl::not_< mpl::and_ <
                is_same < ConnectedKind, connected_kind::unspecified_t >,
                is_directed_graph< Graph > > >::value),
        "connected_kind must be specified for directed graphs");

/*
// all wrong
    // BOOST_CONCEPT_ASSERT((VertexIndexGraphConcept< Graph >));
    BOOST_STATIC_ASSERT_MSG(
        (mpl::not_< is_same<
//            typename property_map< Graph, vertex_index_t >::const_type,
//             typename property_value< Graph, vertex_index_t >::type,
            typename property_value< vertex_property_type< Graph >, vertex_index_t >::type,
            void > >::value),
        "graph has no vertex_index map");
*/

    // typedef typename graph_traits< Graph >::directed_category Cat;
    return is_connected(g, get(boost::vertex_index, g), kind);
}


template < typename Graph, typename VertexIndex, typename ConnectedKind >
inline bool is_connected(const Graph& g, VertexIndex vertex_index, ConnectedKind kind)
{
    BOOST_STATIC_ASSERT_MSG(
        (mpl::not_< mpl::and_ <
                is_same < ConnectedKind, connected_kind::unspecified_t >,
                is_directed_graph< Graph > > >::value),
        "connected_kind must be specified for directed graphs");

    typedef typename graph_traits< Graph >::directed_category Cat;
    return is_connected_dispatch(g, Cat(), kind, vertex_index);
}






// Undirected graph
template < typename IncidenceGraph, typename ConnectedKind, typename VertexIndexMap >
inline bool is_connected_dispatch(const IncidenceGraph& g, undirected_tag,
                                  ConnectedKind, VertexIndexMap vertex_index)
{
    // ignore the connection kind and vertex_index for undirected graph
    return is_connected_undirected(g, vertex_index);
}

template < typename IncidenceGraph >
inline bool is_connected_undirected(const IncidenceGraph& g)
{
    BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< IncidenceGraph >));

    return is_connected_undirected(g, get(vertex_index, g));
}

template < typename IncidenceGraph, typename VertexIndexMap >
inline bool is_connected_undirected(const IncidenceGraph& g, VertexIndexMap vertex_index)
{
    BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< IncidenceGraph >));

    return is_connected_undirected(g,
        make_two_bit_color_map(num_vertices(g), vertex_index), vertex_index);
}


// color should start out white for every vertex
template < typename IncidenceGraph, typename VertexColorMap, typename VertexIndexMap >
inline bool is_connected_undirected(const IncidenceGraph& g,
    VertexColorMap colormap, VertexIndexMap)
{
    typedef typename property_traits< VertexColorMap >::value_type ColorValue;
    typedef color_traits< ColorValue > Color;

    default_dfs_visitor vis;
    depth_first_visit(g, detail::get_default_starting_vertex(g), vis, colormap);

    // If an undirected graph is connected, then each vertex is reachable in a
    // single DFS visit. If any vertex was unreachable, grpah is not connected.
    typename graph_traits< IncidenceGraph >::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
        if (get(colormap, *ui) == Color::white())
            return false;
    return true;
}

// Directed graph, strongly connected
template < typename IncidenceGraph, typename VertexIndexMap >
inline bool is_connected_dispatch(const IncidenceGraph& g, directed_tag,
                                  connected_kind::strong_t, VertexIndexMap vertex_index)
{
    return is_strongly_connected(g, vertex_index);
}

template < typename Graph >
inline bool is_strongly_connected(const Graph& g)
{
    return is_strongly_connected(g, get(vertex_index, g));
}

template < typename Graph, typename VertexIndexMap >
inline bool is_strongly_connected(const Graph& g, VertexIndexMap vertex_index)
{
    // A directed graph is stronly connected if and only if all its vertices
    // are in a sinlge stronly connected component

    BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
    BOOST_STATIC_ASSERT((is_directed_graph< Graph >::value));

    // Run the Tarjan's SCC algorithm
    std::vector< size_t > comp_map(num_vertices(g));
    size_t num_scc = strong_components(
        g, make_iterator_property_map(comp_map.begin(), vertex_index), 
//        g, make_safe_iterator_property_map(comp_map.begin(), vertex_index), 
        vertex_index_map(vertex_index));

    return num_scc == 1;
}


// Directed graph, weakly connected
template < typename IncidenceGraph, typename VertexIndexMap >
inline bool is_connected_dispatch(const IncidenceGraph& g, directed_tag,
                                  connected_kind::weak_t, VertexIndexMap vertex_index)
{
    return is_weakly_connected(g, vertex_index);
}

template < typename BidirectionalGraph >
inline bool is_weakly_connected(const BidirectionalGraph& g)
{
    return is_weakly_connected(g, get(boost::vertex_index, g));
}

template < typename BidirectionalGraph, typename VertexIndexMap >
inline bool is_weakly_connected(const BidirectionalGraph& g, VertexIndexMap vertex_index)
{
    BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept< BidirectionalGraph >));

    // For now do an undirected BFS walk
    return is_weakly_connected(g,
        make_two_bit_color_map(num_vertices(g), vertex_index), vertex_index);
}

template < typename BidirectionalGraph , typename VertexColorMap, typename VertexIndexMap >
inline bool is_weakly_connected(const BidirectionalGraph& g, VertexColorMap colormap, VertexIndexMap vertex_index)
{
    // A directed graph is weakly connected if and only if all its vertices
    // can be reached in a single undirected BFS (or DFS) visit.
    typedef typename property_traits< VertexColorMap >::value_type ColorValue;
    typedef color_traits< ColorValue > Color;

    // todo: consider reimplementing this as DFS (is_connected above) over an
    // as_undirected adaptor.

    neighbor_breadth_first_visit(
        g,
        detail::get_default_starting_vertex(g),
        color_map(colormap)
    );

    typename graph_traits< BidirectionalGraph >::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
        if (get(colormap, *ui) == Color::white())
            return false;
    return true;
}

// Directed graph, unilaterally connected
template < typename IncidenceGraph, typename VertexIndexMap>
inline bool is_connected_dispatch(const IncidenceGraph& g, directed_tag,
                                  connected_kind::unilateral_t,
                                  VertexIndexMap vertex_index_map)
{
    return is_unilaterally_connected(g, vertex_index_map);
}

namespace detail {

    template < typename Graph >
    struct unique_topological_order_visitor : public default_dfs_visitor
    {
        typedef typename graph_traits< Graph >::vertex_descriptor vertex_t;
        vertex_t last_vertex;
        bool& result;

        unique_topological_order_visitor(bool& result)
            : last_vertex(graph_traits< Graph >::null_vertex()), result(result)
        {
            result = true;
        }

        // Check that each finished vertex has an arrow to the last finished one
        template < typename Vertex, typename G >
        void finish_vertex(const Vertex& u, const G& g)
        {
            // todo: consider using TerminatorFunc or an exception to exit
            // the DFS early (performance optimization)
            if (result == false)
                return;

            if (last_vertex != graph_traits< Graph >::null_vertex())
            {
                if (!edge(u, last_vertex, g).second)
                {
                    result = false;
                }
            }
            last_vertex = u;
        }

    };

}

// Checks if the graph is in unique topological order, i.e. linear
template < typename Graph >
bool has_unique_topological_order(const Graph& g)
{
    bool result;
    detail::unique_topological_order_visitor<Graph> vis(result);
    depth_first_search(g, visitor(vis));
    return result;
}


template < typename Graph >
inline bool is_unilaterally_connected(const Graph& g)
{
    return is_unilaterally_connected(g, get(vertex_index, g));
}


template < typename Graph, typename VertexIndexMap >
inline bool is_unilaterally_connected(const Graph& g, VertexIndexMap vertex_index)
{
    // A directed graph is unilaterally connected if and only if its
    // condensation graph is in unique topological order.
    // Warning: condensation might be slow and consume 2x memory for the graph.

    BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
    BOOST_STATIC_ASSERT((is_directed_graph< Graph >::value));

    // Run the Tarjan's SCC algorithm
    std::vector< size_t > comp_number_store(num_vertices(g));
    auto comp_number = make_iterator_property_map(comp_number_store.begin(), vertex_index);

    size_t num_scc = strong_components(g, comp_number, vertex_index_map(vertex_index));
    if (num_scc == 1) // strongly connected case
        return true;

    // Build the condensation graph
    adjacency_list<> c;
    std::vector< std::vector< typename graph_traits< Graph >::vertex_descriptor > > components;
    build_component_lists(g, num_scc, comp_number, components);
    create_condensation_graph(g, components, comp_number, c);

    // Check if the condensation is linear
    return has_unique_topological_order(c);
}


} // namespace boost

#endif /* BOOST_GRAPH_IS_CONNECTED_HPP */



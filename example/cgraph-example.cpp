/* C++ Standard Library */
#include <iostream>

/* Boost Graph Library */
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph/cgraph_graph.hpp>

/* Graphviz */
#include <graphviz/gvc.h>

void example_a()
{
    GVC_t * gvc = ::gvContext();

    Agraph_t * g = ::agopen("g", Agdirected, nullptr);
    Agnode_t * n = ::agnode(g, "n", 1);
    Agnode_t * m = ::agnode(g, "m", 1);
    Agedge_t * e = ::agedge(g, n, m, nullptr, 1);
    ::agsafeset(n, "color", "red", "");

    ::gvLayout(gvc, g, "dot");
    ::gvRender(gvc, g, "plain", stdout);

    ::gvFreeLayout(gvc, g);
    ::agclose(g);

    ::gvFreeContext(gvc);
}

void example_b()
{
    // create a typedef for the Graph type
    typedef boost::cgraph_graph_ptr Graph;
    typedef boost::property_map<Graph, boost::graph_name_t>::type GraphNameMap;
    typedef boost::property_map<Graph, boost::vertex_index_t>::type VertexIndexMap;
    typedef boost::property_map<Graph, boost::vertex_name_t>::type VertexNameMap;
    typedef boost::property_map<Graph, boost::edge_index_t>::type EdgeIndexMap;
    typedef boost::property_map<Graph, boost::edge_name_t>::type EdgeNameMap;

    BOOST_CONCEPT_ASSERT((boost::MultiPassInputIteratorConcept<Graph>));

    /* graph concepts */
    BOOST_CONCEPT_ASSERT((boost::GraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::BidirectionalGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::EdgeListGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::VertexAndEdgeListGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::EdgeMutableGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::VertexMutableGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::MutableIncidenceGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::MutableBidirectionalGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::MutableEdgeListGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::VertexMutablePropertyGraphConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::EdgeMutablePropertyGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::AdjacencyMatrixConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyGraphConcept<Graph, boost::graph_traits<Graph>::edge_descriptor, boost::edge_index_t>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyGraphConcept<Graph, boost::graph_traits<Graph>::vertex_descriptor, boost::vertex_index_t>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyGraphConcept<Graph, Graph, boost::graph_name_t>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyGraphConcept<Graph, boost::graph_traits<Graph>::edge_descriptor, boost::edge_name_t>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyGraphConcept<Graph, boost::graph_traits<Graph>::vertex_descriptor, boost::vertex_name_t>));
//    BOOST_CONCEPT_ASSERT((boost::PropertyGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::LvaluePropertyGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::VertexIndexGraphConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::EdgeIndexGraphConcept<Graph>));

    /* utility concepts */
//    BOOST_CONCEPT_ASSERT((boost::ColorValueConcept<C>));
//    BOOST_CONCEPT_ASSERT((boost::BasicMatrixConcept<M, I, V>));
//    BOOST_CONCEPT_ASSERT((boost::NumericValueConcept<Numeric>));
//    BOOST_CONCEPT_ASSERT((boost::DegreeMeasureConcept<Measure, Graph>));
//    BOOST_CONCEPT_ASSERT((boost::DistanceMeasureConcept<Measure, Graph>));

    /* iterator concepts */
    BOOST_CONCEPT_ASSERT((boost::InputIteratorConcept<Graph>));
//    BOOST_CONCEPT_ASSERT((boost::OutputIteratorConcept<Graph, ValueT));
    BOOST_CONCEPT_ASSERT((boost::ForwardIteratorConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::Mutable_ForwardIteratorConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::BidirectionalIteratorConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::Mutable_BidirectionalIteratorConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::RandomAccessIteratorConcept<Graph>));
    BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIteratorConcept<Graph>));

    /* property map concepts */
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<boost::cgraph_graph_id_map, boost::graph_traits<Graph>::edge_descriptor>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<boost::cgraph_graph_id_map, boost::graph_traits<Graph>::vertex_descriptor>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<boost::cgraph_graph_name_map, Graph>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<boost::cgraph_graph_name_map, boost::graph_traits<Graph>::edge_descriptor>));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept<boost::cgraph_graph_name_map, boost::graph_traits<Graph>::vertex_descriptor>));
//    BOOST_CONCEPT_ASSERT((boost::WritablePropertyMapConcept<boost::cgraph_graph_id_map, void *>));
//    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept<boost::cgraph_graph_id_map, void *>));
//    BOOST_CONCEPT_ASSERT((boost::LvaluePropertyMapConcept<boost::cgraph_graph_id_map, void *>));
//    BOOST_CONCEPT_ASSERT((boost::Mutable_LvaluePropertyMapConcept<boost::cgraph_graph_id_map, void *>));

    GVC_t * gvc = ::gvContext();

    // declare a graph object
    Graph g = ::agopen("g", Agdirected, nullptr);

    // add nodes
    boost::graph_traits<Graph>::vertex_descriptor A = boost::add_vertex("A", g);
    boost::graph_traits<Graph>::vertex_descriptor B = boost::add_vertex("B", g);
    boost::graph_traits<Graph>::vertex_descriptor C = boost::add_vertex("C", g);
    boost::graph_traits<Graph>::vertex_descriptor D = boost::add_vertex("D", g);
    boost::graph_traits<Graph>::vertex_descriptor E = boost::add_vertex("E", g);

    // add edges
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> AB = boost::add_edge(A, B, "AB", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> AD = boost::add_edge(A, D, "AD", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> CA = boost::add_edge(C, A, "CA", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> DC = boost::add_edge(D, C, "DC", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> CE = boost::add_edge(C, E, "CE", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> BD = boost::add_edge(B, D, "BD", g);
    std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> DE = boost::add_edge(D, E, "DE", g);

    std::cout << "vertices(g) = ";
    typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
    VertexIndexMap vertexIndex = get(boost::vertex_index, g);
    VertexNameMap vertexName = get(boost::vertex_name, g);
    std::pair<vertex_iter, vertex_iter> vp;
    for (vp = vertices(g); vp.first != vp.second; ++vp.first) {
        const boost::graph_traits<Graph>::vertex_descriptor v = *vp.first;
        std::cout << vertexIndex[v];
        if(vertexName[v]) {
            std::cout << ':' << vertexName[v];
        }
        std::cout << " ";
    }
    std::cout << std::endl;

    std::cout << "edges(g) = ";
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    EdgeIndexMap edgeIndex = get(boost::edge_index, g);
    EdgeNameMap edgeName = get(boost::edge_name, g);
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        boost::graph_traits<Graph>::vertex_descriptor src = source(*ei, g);
        boost::graph_traits<Graph>::vertex_descriptor trg = target(*ei, g);
        std::cout << "(";
        std::cout << edgeIndex[*ei];
        if(edgeName[*ei]) {
            std::cout << ':' << edgeName[*ei];
        }
        std::cout << ") ";
    }
    std::cout << std::endl;

    std::cout << "out-edges of A: ";
    boost::graph_traits<Graph>::out_edge_iterator out_i, out_end;
    for (boost::tie(out_i, out_end) = out_edges(A, g); out_i != out_end; ++out_i) {
        boost::graph_traits<Graph>::vertex_descriptor src = source(*out_i, g);
        boost::graph_traits<Graph>::vertex_descriptor targ = target(*out_i, g);
        std::cout << "(";
        std::cout << edgeIndex[*out_i];
        if(edgeName[*out_i]) {
            std::cout << ':' << edgeName[*out_i];
        }
        std::cout << ") ";
    }
    std::cout << std::endl;

    std::cout << "in-edges of A: ";
    boost::graph_traits<Graph>::in_edge_iterator in_i, in_end;
    for (boost::tie(in_i, in_end) = in_edges(A, g); in_i != in_end; ++in_i) {
        boost::graph_traits<Graph>::vertex_descriptor src = source(*in_i, g);
        boost::graph_traits<Graph>::vertex_descriptor targ = target(*in_i, g);
        std::cout << "(";
        std::cout << edgeIndex[*in_i];
        if(edgeName[*in_i]) {
            std::cout << ':' << edgeName[*in_i];
        }
        std::cout << ") ";
    }
    std::cout << std::endl;

    std::cout << "adjacent vertices of C: ";
    boost::graph_traits<Graph>::adjacency_iterator ai, ai_end;
    for (boost::tie(ai, ai_end) = adjacent_vertices(C, g); ai != ai_end; ++ai) {
        std::cout << "(";
        std::cout << edgeIndex[*ai];
        if(edgeName[*ai]) {
            std::cout << ':' << edgeName[*ai];
        }
        std::cout << ") ";
    }
    std::cout << std::endl;

    // @todo the following terminates with
    // terminate called after throwing an instance of 'boost::wrapexcept<boost::not_a_dag>'
    //     what():  The graph must be a DAG.
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef std::list<Vertex> container;
    std::list<Vertex> c;
    std::map<Vertex, boost::default_color_type> vertex_colors;
    boost::topological_sort(g, std::back_inserter(c), boost::color_map(boost::make_assoc_property_map(vertex_colors)));
    std::cout << "A topological ordering: ";
    for (container::reverse_iterator ii = c.rbegin(); ii != c.rend(); ++ii) {
        std::cout << vertexIndex[*ii];
        if(vertexName[*ii]) {
            std::cout << ':' << vertexName[*ii];
        }
    }
    std::cout << std::endl;

    ::gvLayout(gvc, g, "dot");
    ::gvRender(gvc, g, "plain", stdout);

    ::gvFreeLayout(gvc, g);
    ::agclose(g);

    ::gvFreeContext(gvc);
}

int main(int argc, char *argv[])
{
    example_b();

    return EXIT_SUCCESS;
}

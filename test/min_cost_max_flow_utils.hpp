//=======================================================================
// Copyright 2013 University of Warsaw.
// Authors: Piotr Wygocki 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SAMPLE_GRAPH_UNDIRECTED_HPP
#define SAMPLE_GRAPH_UNDIRECTED_HPP 

#include <iostream>
#include <cstdlib>
#include <boost/graph/adjacency_list.hpp>


namespace boost {
struct SampleGraph {
    typedef adjacency_list_traits < vecS, vecS, directedS > Traits;

    typedef adjacency_list < listS, vecS, directedS, no_property,
            property < edge_capacity_t, long,
                property < edge_residual_capacity_t, long,
                    property < edge_reverse_t, Traits::edge_descriptor, 
                        property <edge_weight_t, long>
                             > 
                        > 
                     > > Graph;
    typedef property_map < Graph, edge_capacity_t >::type Capacity;
    typedef property_map < Graph, edge_residual_capacity_t >::type ResidualCapacity;
    typedef property_map < Graph, edge_weight_t >::type Weight;
    

    template <typename Graph, typename Weight, typename Capacity, typename Reversed, typename ResidualCapacity>
    class EdgeAdder {
    public:
        EdgeAdder(Graph & g, Weight & w, Capacity & c, Reversed & rev, ResidualCapacity & residualCapacity) 
            : m_g(g), m_w(w), m_cap(c), m_resCap(residualCapacity), m_rev(rev) {}
        void addEdge(int v, int w, int weight, int capacity) {
            Traits::edge_descriptor e,f;
            e = add(v, w, weight, capacity);
            f = add(w, v, -weight, 0);
            m_rev[e] = f; 
            m_rev[f] = e; 
        }
    private:
        Traits::edge_descriptor add(int v, int w, int weight, int capacity) {
            bool b;
            Traits::edge_descriptor e;
            boost::tie(e, b) = add_edge(vertex(v, m_g), vertex(w, m_g), m_g);
            if (!b) {
              std::cerr << "Edge between " << v << " and " << w << " already exists." << std::endl;
              std::abort();
            }
            m_cap[e] = capacity;
            m_w[e] = weight;
            return e;
        }
        Graph & m_g;
        Weight & m_w;
        Capacity & m_cap;
        ResidualCapacity & m_resCap;
        Reversed & m_rev;
    };


    static Graph getSampleGraph(unsigned & s, unsigned & t) {
        const boost::graph_traits<Graph>::vertices_size_type N(6);
        typedef property_map < Graph, edge_reverse_t >::type Reversed;

        Graph g(N);
        Capacity  capacity = get(edge_capacity, g);
        Reversed rev = get(edge_reverse, g);
        ResidualCapacity residual_capacity = get(edge_residual_capacity, g); 
        Weight weight = get(edge_weight, g);

        s = 0;
        t = 5;

        EdgeAdder<Graph, Weight, Capacity, Reversed, ResidualCapacity> 
            ea(g, weight, capacity, rev, residual_capacity);

        ea.addEdge(0, 1, 4 ,2);
        ea.addEdge(0, 2, 2 ,2);

        ea.addEdge(1, 3, 2 ,2);
        ea.addEdge(1, 4, 1 ,1);
        ea.addEdge(2, 3, 1 ,1);
        ea.addEdge(2, 4, 1 ,1);

        ea.addEdge(3, 5, 4 ,20);
        ea.addEdge(4, 5, 2 ,20);

        return g;
    }       
    
    static Graph getSampleGraph2(unsigned & s, unsigned & t) {
        const boost::graph_traits<Graph>::vertices_size_type N(5);
        typedef property_map < Graph, edge_reverse_t >::type Reversed;

        Graph g(N);
        Capacity  capacity = get(edge_capacity, g);
        Reversed rev = get(edge_reverse, g);
        ResidualCapacity residual_capacity = get(edge_residual_capacity, g); 
        Weight weight = get(edge_weight, g);

        s = 0;
        t = 4;

        EdgeAdder<Graph, Weight, Capacity, Reversed, ResidualCapacity> 
            ea(g, weight, capacity, rev, residual_capacity);

        ea.addEdge(0, 1, 4 ,2);
        ea.addEdge(0, 2, 2 ,2);
        ea.addEdge(1, 2, 2 ,2);
        ea.addEdge(2, 3, 1 ,1);
        ea.addEdge(2, 4, 1 ,1);
        ea.addEdge(3, 4, 1 ,1);
        

        ea.addEdge(1, 0, 2 ,2);
        ea.addEdge(2, 0, 1 ,1);
        ea.addEdge(2, 1, 5 ,2);
        ea.addEdge(3, 2, 1 ,1);
        ea.addEdge(4, 2, 2 ,2);
        ea.addEdge(4, 3, 1 ,3);

        return g;
    }       
};
} //boost

#endif /* SAMPLE_GRAPH_UNDIRECTED_HPP */


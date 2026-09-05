#ifndef BOOST_GRAPH_KARP_MINIMUM_CYCLE_MEAN_HPP
#define BOOST_GRAPH_KARP_MINIMUM_CYCLE_MEAN_HPP

#include <iostream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace boost
{
    template<typename Graph>
    double karp_minimum_cycle_mean(Graph g)
    {
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        typename property_map<Graph, edge_weight_t>::type weight = get(edge_weight,g);
        typedef typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator;
        BOOST_CONCEPT_ASSERT((GraphConcept< Graph >));

        int n  = num_vertices(g);
        double distance_matrix[n+1][n];
        std::fill(distance_matrix[0],distance_matrix[0]+(n+1)*(n), -1.0);
        distance_matrix[0][0] = 0;

        std::pair <vertex_iterator ,vertex_iterator> vertex_iterator_pair= vertices(g);
        std::vector<vertex_descriptor> vertices_vector;
        for(vertex_iterator a=vertex_iterator_pair.first; a!=vertex_iterator_pair.second;a++)
        {
            vertices_vector.push_back(*a);;
        }

        for (int i=1; i<=n; i++)
        {
            for (int j=0; j<n; j++)
            {
                std::pair <out_edge_iterator ,out_edge_iterator> edge_iterator_pair= boost::out_edges(vertices_vector[j],g);
                std::vector<typename boost::graph_traits<Graph>::edge_descriptor> incident_edges;
                for(out_edge_iterator b=edge_iterator_pair.first;b!=edge_iterator_pair.second;b++)
                {
                    incident_edges.push_back(*b);
                }
                for (int k=0; k< out_degree(vertices_vector[j],g); k++)
                {
                    vertex_descriptor t=target(incident_edges[k],g);
                    auto it = find(vertices_vector.begin(), vertices_vector.end(), t);
                    int index = it - vertices_vector.begin();
                    if (distance_matrix[i-1][index] != -1)
                    {
                        double curr_wt = distance_matrix[i-1][index] +
                                      get(weight,incident_edges[k]);
                        if (distance_matrix[i][j] == -1)
                            distance_matrix[i][j] = curr_wt;
                        else
                            distance_matrix[i][j] = std::min(distance_matrix[i][j], curr_wt);
                    }
                }
            }
        }
        double avg[n];
        std::fill(avg,avg+n,-1.0);

        for (int i=0; i<n; i++)
        {
            if (distance_matrix[n][i] != -1)
            {
                for (int j=0; j<n; j++)
                    if (distance_matrix[j][i] != -1)
                        avg[i] = std::max(avg[i],(double)(distance_matrix[n][i]-distance_matrix[j][i])/(n-j));
            }
        }
        double minimum_mean_weight = avg[0];
        for (int i=0; i<n; i++)
            if (avg[i] != -1 && avg[i] < minimum_mean_weight)
                minimum_mean_weight = avg[i];

        return minimum_mean_weight;
    }

}
#endif
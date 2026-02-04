//=======================================================================
// Copyright 2026 Becheler Code Labs for C++ Alliance
// Authors: Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
//
// Revision History:
//

#ifndef BOOST_GRAPH_LOUVAIN_CLUSTERING_HPP
#define BOOST_GRAPH_LOUVAIN_CLUSTERING_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_quality_functions.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <map>
#include <set>
#include <random>
#include <algorithm>
#include <iostream>

namespace boost
{
namespace louvain_detail 
{

/// @brief Result of graph aggregation operation.
template <typename Graph, typename PartitionMap, typename VertexDescriptor, typename WeightType>
struct aggregation_result
{
    Graph graph;
    PartitionMap partition;
    std::map<VertexDescriptor, WeightType> internal_weights;
    std::map<VertexDescriptor, std::set<VertexDescriptor>> vertex_mapping;
};

// Aggregate graph by collapsing communities into super-nodes.
// Edges between communities are preserved with accumulated weights.
template <typename Graph, typename CommunityMap, typename WeightMap>
auto aggregate(
    const Graph& g, 
    const CommunityMap& communities, 
    const WeightMap& weight
){
    using vertex_descriptor = typename graph_traits<Graph>::vertex_descriptor;
    using edge_descriptor = typename graph_traits<Graph>::edge_descriptor;
    using vertex_iterator = typename graph_traits<Graph>::vertex_iterator;
    using edge_iterator = typename graph_traits<Graph>::edge_iterator;
    using community_type = typename property_traits<CommunityMap>::value_type;
    using weight_type = typename property_traits<WeightMap>::value_type;
    using edge_property_t = property<edge_weight_t, weight_type>;
    using aggregated_graph_t = adjacency_list<vecS, vecS, undirectedS, no_property, edge_property_t>;
    using result_t = aggregation_result<aggregated_graph_t, vector_property_map<vertex_descriptor>, vertex_descriptor, weight_type>;
    
    aggregated_graph_t new_g;
    vector_property_map<vertex_descriptor> new_community_map;
    
    std::set<community_type> unique_communities;
    std::map<community_type, vertex_descriptor> comm_to_vertex;
    std::map<vertex_descriptor, std::set<vertex_descriptor>> vertex_to_originals;

    // collect unique communities
    vertex_iterator vi, vi_end;
    for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        unique_communities.insert(get(communities, *vi));
    }
    
    // create super-nodes with each their own community
    for (const community_type& comm : unique_communities) {
        vertex_descriptor new_v = add_vertex(new_g);
        comm_to_vertex[comm] = new_v;
        vertex_to_originals[new_v] = std::set<vertex_descriptor>();
        put(new_community_map, new_v, new_v);
    }
    
    // records which original vertices belong to which super-node
    for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        community_type c = get(communities, *vi);
        vertex_descriptor new_v = comm_to_vertex[c];
        vertex_to_originals[new_v].insert(*vi);
    } 
    
    // Build edges with accumulated weights
    std::map<std::pair<vertex_descriptor, vertex_descriptor>, weight_type> temp_edge_weights;
    std::map<vertex_descriptor, weight_type> temp_internal_weights;
    
    edge_iterator edge_it, edge_end;
    for (tie(edge_it, edge_end) = edges(g); edge_it != edge_end; ++edge_it) {
        vertex_descriptor u = source(*edge_it, g);
        vertex_descriptor v = target(*edge_it, g);
        
        community_type c_u = get(communities, u);
        community_type c_v = get(communities, v);
        
        vertex_descriptor new_u = comm_to_vertex[c_u];
        vertex_descriptor new_v = comm_to_vertex[c_v];
        
        weight_type w = get(weight, *edge_it);
        
        // vertices in same community = self loop on super node
        if (new_u == new_v) {
            auto edge_key = std::make_pair(new_u, new_u);
            temp_edge_weights[edge_key] += w;
            temp_internal_weights[new_u] += w;
        } else {
            auto edge_key = std::make_pair(std::min(new_u, new_v), std::max(new_u, new_v));
            temp_edge_weights[edge_key] += w;
        }
    }
    
    // add edges to connect super nodes
    for (const auto& kv : temp_edge_weights) {
        edge_descriptor e;
        bool inserted;
        tie(e, inserted) = add_edge(kv.first.first, kv.first.second, kv.second, new_g);
    }
    
    return result_t{std::move(new_g), std::move(new_community_map), std::move(temp_internal_weights), std::move(vertex_to_originals)};
}

// Track hierarchy of aggregation levels for unfolding partitions.
template <typename VertexDescriptor>
struct hierarchy_t
{
    // Each level maps super-nodes to their constituent vertices from the previous level.
    using level_t = std::map<VertexDescriptor, std::set<VertexDescriptor>>;
    std::vector<level_t> levels;
    
    void push_level(const level_t& mapping) {
        levels.push_back(mapping);
    }
    
    void push_level(level_t&& mapping) {
        levels.push_back(std::move(mapping));
    }
    
    std::size_t size() const {
        return levels.size();
    }
    
    bool empty() const {
        return levels.empty();
    }
    
    const level_t& operator[](std::size_t i) const {
        return levels[i];
    }
    
    template <typename CommunityMap>
    auto unfold(const CommunityMap& final_partition) const
    {
        assert(!empty());

        std::map<VertexDescriptor, VertexDescriptor> original_partition;
        
        for (const auto& kv : final_partition) {
            std::set<VertexDescriptor> current_nodes;
            current_nodes.insert(kv.first);
            
            // From coarse to fine
            for (int level = size() - 1; level >= 0; --level) {
                std::set<VertexDescriptor> next_nodes;
                
                for (VertexDescriptor node : current_nodes) {
                    auto it = levels[level].find(node);
                    assert(it != levels[level].end());
                    next_nodes.insert(it->second.begin(), it->second.end());
                }
                
                current_nodes = std::move(next_nodes);
            }
            
            // Assign all original vertices to community
            for (VertexDescriptor original_v : current_nodes) {
                original_partition[original_v] = kv.second;
            }
        }
        
        return original_partition;
    }
};

// Create vector of all vertices.
template <typename Graph>
auto get_vertex_vector(const Graph& g)
{
    using vertex_descriptor = typename graph_traits<Graph>::vertex_descriptor;
    using vertex_iterator = typename graph_traits<Graph>::vertex_iterator;
    
    std::vector<vertex_descriptor> vertices_vec;
    vertices_vec.reserve(num_vertices(g));
    vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        vertices_vec.push_back(*vi);
    }
    return vertices_vec;
}

}} // namespace boost::louvain_detail

namespace boost 
{

template <typename Graph, typename CommunityMap, typename WeightMap, typename QualityFunction = newman_and_girvan>
typename property_traits<WeightMap>::value_type
louvain_local_optimization(
    const Graph& g, 
    CommunityMap& communities, 
    const WeightMap& w,
    typename property_traits<WeightMap>::value_type min_improvement_inner = typename property_traits<WeightMap>::value_type(0.0),
    unsigned int seed = 0
)
{
    using community_type = typename property_traits<CommunityMap>::value_type;
    using weight_type = typename property_traits<WeightMap>::value_type;
    using vertex_descriptor = typename graph_traits<Graph>::vertex_descriptor;
    using vertex_iterator = typename graph_traits<Graph>::vertex_iterator;
    using out_edge_iterator = typename graph_traits<Graph>::out_edge_iterator;
    
    std::size_t n = num_vertices(g);
    
    // Use vector_property_map for O(1) community access
    vector_property_map<weight_type> k;
    vector_property_map<weight_type> in;
    vector_property_map<weight_type> tot;
    weight_type m;
    
    // populates k[c], in[c], tot[c]
    weight_type Q = QualityFunction::quality(g, communities, w, k, in, tot, m);
    weight_type Q_new = Q;
    std::size_t num_moves = 0;
    std::size_t pass_number = 0;
    bool has_rolled_back = false;
    
    // Randomize vertex order once
    std::mt19937 gen(seed);
    std::vector<vertex_descriptor> vertex_order = louvain_detail::get_vertex_vector(g);
    std::shuffle(vertex_order.begin(), vertex_order.end(), gen);
    
    // Pre-allocate neighbor buffers
    vector_property_map<weight_type> neigh_weight;
    std::vector<community_type> neigh_comm;
    neigh_comm.reserve(100);
    
    // Pre-allocate rollback buffers (only used if needed)
    std::vector<community_type> saved_partition;
    vector_property_map<weight_type> saved_in;
    vector_property_map<weight_type> saved_tot;
    
    do
    {
        Q = Q_new;
        num_moves = 0;        
        pass_number++;
        
        // Swap 2 random vertices per pass to escape local minima
        if (pass_number > 1 && vertex_order.size() >= 2) {
            std::uniform_int_distribution<std::size_t> dist(0, vertex_order.size() - 1);
            std::size_t idx1 = dist(gen);
            std::size_t idx2 = dist(gen);
            std::swap(vertex_order[idx1], vertex_order[idx2]);
        }
        
        // Lazy save: only save state if we've previously needed to rollback
        if (has_rolled_back) {
            saved_partition.resize(num_vertices(g));
            saved_in = in;
            saved_tot = tot;
            vertex_iterator vi_save, vi_save_end;
            for (boost::tie(vi_save, vi_save_end) = vertices(g); vi_save != vi_save_end; ++vi_save) {
                saved_partition[get(vertex_index, g, *vi_save)] = get(communities, *vi_save);
            }
        }
        
        for(auto v : vertex_order)
        {
            community_type c_old = get(communities, v);
            weight_type k_v = get(k, v);
            weight_type w_selfloop = 0;
            
            // Single-pass neighbor aggregation using pre-allocated vector
            neigh_comm.clear();
            out_edge_iterator ei, ei_end;
            for (boost::tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
                vertex_descriptor neighbor = target(*ei, g);
                weight_type edge_w = get(w, *ei);
                if (neighbor == v) {
                    w_selfloop += edge_w;
                } else {
                    community_type c_neighbor = get(communities, neighbor);
                    if (get(neigh_weight, c_neighbor) == weight_type(0)) {
                        neigh_comm.push_back(c_neighbor);
                    }
                    put(neigh_weight, c_neighbor, get(neigh_weight, c_neighbor) + edge_w);
                }
            }
            
            // Remove v from community
            weight_type k_v_in_old = get(neigh_weight, c_old);
            QualityFunction::remove(in, tot, c_old, k_v, k_v_in_old, w_selfloop);
            
            // Find best community
            community_type c_best = c_old;
            weight_type best_gain = 0;
            
            for(community_type c_neighbor : neigh_comm)
            {
                weight_type k_v_in_neighbor = get(neigh_weight, c_neighbor);
                weight_type gain = QualityFunction::gain(tot, m, c_neighbor, k_v_in_neighbor, k_v);
                
                if (gain > best_gain) {
                    best_gain = gain;
                    c_best = c_neighbor;
                }
            }
            
            // Insert v into best community
            if (c_best != c_old) {
                put(communities, v, c_best);
                num_moves++;
            }
            
            weight_type k_v_in_best = get(neigh_weight, c_best);
            QualityFunction::insert(in, tot, c_best, k_v, k_v_in_best, w_selfloop);
            
            // Clear neighbor weights for next vertex
            for(community_type c : neigh_comm) {
                put(neigh_weight, c, weight_type(0));
            }
        }

        // Compute quality from incremental in/tot (no graph traversal)
        Q_new = QualityFunction::quality_from_incremental(in, tot, m, n);
        
        // Rollback if quality didn't improve after moving nodes
        // Prevent endless oscillations of vertices: algo gets one extra pass before giving up
        if (num_moves > 0 && Q_new <= Q) {
            if (has_rolled_back) {
                vertex_iterator vi_restore, vi_restore_end;
                for (boost::tie(vi_restore, vi_restore_end) = vertices(g); vi_restore != vi_restore_end; ++vi_restore) {
                    put(communities, *vi_restore, saved_partition[get(vertex_index, g, *vi_restore)]);
                }
                in = saved_in;
                tot = saved_tot;
                Q_new = Q;
                break;
            } else {
                has_rolled_back = true;
            }
        }
        
    } while (num_moves > 0 && (Q_new - Q) > min_improvement_inner);
    
    return Q_new;
}

template <typename Graph, typename ComponentMap, typename WeightMap, typename QualityFunction = newman_and_girvan>
typename property_traits<WeightMap>::value_type
louvain_clustering(
    const Graph& g0,
    ComponentMap components,
    const WeightMap& w0,
    typename property_traits<WeightMap>::value_type min_improvement_inner = typename property_traits<WeightMap>::value_type(0.0),
    typename property_traits<WeightMap>::value_type min_improvement_outer = typename property_traits<WeightMap>::value_type(0.0),
    unsigned int seed = 0
){
    using vertex_descriptor = typename graph_traits<Graph>::vertex_descriptor;
    using weight_type = typename property_traits<WeightMap>::value_type;
    using vertex_iterator = typename graph_traits<Graph>::vertex_iterator;
    
    // Initialize each vertex to its own community
    vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g0); vi != vi_end; ++vi) {
        put(components, *vi, *vi);
    }
    
    // Run local optimization
    weight_type Q = louvain_local_optimization<Graph, ComponentMap, WeightMap, QualityFunction>(
        g0, components, w0, min_improvement_inner, seed);
    
    // Build partition vector from current component map
    std::vector<vertex_descriptor> partition_vec(num_vertices(g0));
    for (boost::tie(vi, vi_end) = vertices(g0); vi != vi_end; ++vi) {
        partition_vec[get(vertex_index, g0, *vi)] = get(components, *vi);
    }
    
    louvain_detail::hierarchy_t<vertex_descriptor> hierarchy;    
    auto partition_map_g0 = make_iterator_property_map(partition_vec.begin(), get(vertex_index, g0));    
    auto agg_result = louvain_detail::aggregate(g0, partition_map_g0, w0);
    
    std::size_t prev_n_vertices = num_vertices(g0);
    std::size_t iteration = 0;
    
    // Track best partition across all levels
    std::vector<vertex_descriptor> best_partition = partition_vec;
    weight_type best_Q = Q;
    std::size_t best_level = 0;
    
    while (true) {
        iteration++;
        // Check convergence: graph didn't get smaller (no communities merged)
        std::size_t n_communities = num_vertices(agg_result.graph);
        
        if (n_communities >= prev_n_vertices || n_communities == 1) {
            break;
        }
        
        hierarchy.push_level(std::move(agg_result.vertex_mapping));
        weight_type Q_old = Q;
        weight_type Q_agg = louvain_local_optimization(
            agg_result.graph, 
            agg_result.partition, 
            get(edge_weight, agg_result.graph), 
            min_improvement_inner,
            seed  // Same seed across levels
        );
        
        // Unfold partition to original graph to compute actual Q
        std::map<vertex_descriptor, vertex_descriptor> agg_partition_map;
        vertex_iterator vi_agg, vi_agg_end;
        for (boost::tie(vi_agg, vi_agg_end) = vertices(agg_result.graph); vi_agg != vi_agg_end; ++vi_agg) {
            agg_partition_map[*vi_agg] = get(agg_result.partition, *vi_agg);
        }
        
        auto unfolded_map = hierarchy.unfold(agg_partition_map);
        vertex_iterator vi_orig, vi_orig_end;
        for (boost::tie(vi_orig, vi_orig_end) = vertices(g0); vi_orig != vi_orig_end; ++vi_orig) {
            partition_vec[get(vertex_index, g0, *vi_orig)] = unfolded_map[*vi_orig];
        }
        
        // Compute Q on original graph
        auto partition_map_check = make_iterator_property_map(partition_vec.begin(), get(vertex_index, g0));
        Q = QualityFunction::quality(g0, partition_map_check, w0);
        
        // Track best partition
        if (Q > best_Q) {
            best_Q = Q;
            best_partition = partition_vec;
            best_level = iteration;
        }
        
        // Stop if quality did not improve
        if (Q - Q_old <= min_improvement_outer) {
            break;
        }
        
        prev_n_vertices = n_communities;        
        agg_result = louvain_detail::aggregate(agg_result.graph, agg_result.partition, get(edge_weight, agg_result.graph));
    }
    
    // Write best partition to output ComponentMap
    partition_vec = best_partition;
    Q = best_Q;
    
    for (boost::tie(vi, vi_end) = vertices(g0); vi != vi_end; ++vi) {
        put(components, *vi, partition_vec[get(vertex_index, g0, *vi)]);
    }
    
    return Q;
}

} // namespace boost

#endif

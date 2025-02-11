# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # When we're part of the Boost super project.
    cmake_minimum_required(VERSION 3.5...3.16)
    
    project(boost_graph VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)
    
    add_library(boost_graph
      src/graphml.cpp
      src/read_graphviz_new.cpp
    )
    
    target_include_directories(boost_graph PUBLIC include)
    
    target_link_libraries(boost_graph
      PUBLIC
        Boost::algorithm
        Boost::any
        Boost::array
        Boost::assert
        Boost::bimap
        Boost::bind
        Boost::concept_check
        Boost::config
        Boost::container_hash
        Boost::conversion
        Boost::core
        Boost::detail
        Boost::foreach
        Boost::function
        Boost::integer
        Boost::iterator
        Boost::lexical_cast
        Boost::math
        Boost::move
        Boost::mpl
        Boost::multi_index
        Boost::optional
        Boost::parameter
        Boost::preprocessor
        Boost::property_map
        Boost::property_tree
        Boost::random
        Boost::range
        Boost::serialization
        Boost::smart_ptr
        Boost::spirit
        Boost::static_assert
        Boost::throw_exception
        Boost::tti
        Boost::tuple
        Boost::type_traits
        Boost::typeof
        Boost::unordered
        Boost::utility
        Boost::xpressive
      PRIVATE
        Boost::regex
    )
    
    target_compile_definitions(boost_graph
      PUBLIC BOOST_GRAPH_NO_LIB
      # Source files already define BOOST_GRAPH_SOURCE
      # PRIVATE BOOST_GRAPH_SOURCE
    )
else()
    # When we're worked on by Boost.Graph developers.
    # Using 3.29 for now until I figure out the differences with the new Boost
    # config cmake introduced in 3.30.
    cmake_minimum_required(VERSION 3.29)
    project(boost_graph VERSION 1.0.0 LANGUAGES CXX)
    
    set(CMAKE_CXX_STANDARD 14)
    
    # TODO: The super-project version is set explicitly just like this, but we 
    # should parse it from boost/version.hpp.
    find_package(Boost 1.87.0 REQUIRED COMPONENTS regex)

    add_library(boost_graph
        include/boost/graph/adj_list_serialize.hpp
        include/boost/graph/adjacency_iterator.hpp
        include/boost/graph/adjacency_list.hpp
        include/boost/graph/adjacency_list_io.hpp
        include/boost/graph/adjacency_matrix.hpp
        include/boost/graph/astar_search.hpp
        include/boost/graph/bandwidth.hpp
        include/boost/graph/bc_clustering.hpp
        include/boost/graph/bellman_ford_shortest_paths.hpp
        include/boost/graph/betweenness_centrality.hpp
        include/boost/graph/biconnected_components.hpp
        include/boost/graph/bipartite.hpp
        include/boost/graph/boyer_myrvold_planar_test.hpp
        include/boost/graph/boykov_kolmogorov_max_flow.hpp
        include/boost/graph/breadth_first_search.hpp
        include/boost/graph/bron_kerbosch_all_cliques.hpp
        include/boost/graph/buffer_concepts.hpp
        include/boost/graph/chrobak_payne_drawing.hpp
        include/boost/graph/circle_layout.hpp
        include/boost/graph/closeness_centrality.hpp
        include/boost/graph/clustering_coefficient.hpp
        include/boost/graph/compressed_sparse_row_graph.hpp
        include/boost/graph/connected_components.hpp
        include/boost/graph/copy.hpp
        include/boost/graph/core_numbers.hpp
        include/boost/graph/create_condensation_graph.hpp
        include/boost/graph/cuthill_mckee_ordering.hpp
        include/boost/graph/cycle_canceling.hpp
        include/boost/graph/dag_shortest_paths.hpp
        include/boost/graph/degree_centrality.hpp
        include/boost/graph/depth_first_search.hpp
        include/boost/graph/dijkstra_shortest_paths.hpp
        include/boost/graph/dijkstra_shortest_paths_no_color_map.hpp
        include/boost/graph/dimacs.hpp
        include/boost/graph/directed_graph.hpp
        include/boost/graph/dll_import_export.hpp
        include/boost/graph/dominator_tree.hpp
        include/boost/graph/eccentricity.hpp
        include/boost/graph/edge_coloring.hpp
        include/boost/graph/edge_connectivity.hpp
        include/boost/graph/edge_list.hpp
        include/boost/graph/edmonds_karp_max_flow.hpp
        include/boost/graph/edmunds_karp_max_flow.hpp
        include/boost/graph/erdos_renyi_generator.hpp
        include/boost/graph/exception.hpp
        include/boost/graph/exterior_property.hpp
        include/boost/graph/filtered_graph.hpp
        include/boost/graph/find_flow_cost.hpp
        include/boost/graph/floyd_warshall_shortest.hpp
        include/boost/graph/fruchterman_reingold.hpp
        include/boost/graph/geodesic_distance.hpp
        include/boost/graph/graph_archetypes.hpp
        include/boost/graph/graph_as_tree.hpp
        include/boost/graph/graph_concepts.hpp
        include/boost/graph/graph_mutability_traits.hpp
        include/boost/graph/graph_selectors.hpp
        include/boost/graph/graph_stats.hpp
        include/boost/graph/graph_traits.hpp
        include/boost/graph/graph_utility.hpp
        include/boost/graph/graphml.hpp
        include/boost/graph/graphviz.hpp
        include/boost/graph/grid_graph.hpp
        include/boost/graph/gursoy_atun_layout.hpp
        include/boost/graph/hawick_circuits.hpp
        include/boost/graph/howard_cycle_ratio.hpp
        include/boost/graph/incremental_components.hpp
        include/boost/graph/is_kuratowski_subgraph.hpp
        include/boost/graph/is_straight_line_drawing.hpp
        include/boost/graph/isomorphism.hpp
        include/boost/graph/iteration_macros.hpp
        include/boost/graph/iteration_macros_undef.hpp
        include/boost/graph/johnson_all_pairs_shortest.hpp
        include/boost/graph/kamada_kawai_spring_layout.hpp
        include/boost/graph/king_ordering.hpp
        include/boost/graph/kruskal_min_spanning_tree.hpp
        include/boost/graph/labeled_graph.hpp
        include/boost/graph/leda_graph.hpp
        include/boost/graph/lookup_edge.hpp
        include/boost/graph/loop_erased_random_walk.hpp
        include/boost/graph/make_biconnected_planar.hpp
        include/boost/graph/make_connected.hpp
        include/boost/graph/make_maximal_planar.hpp
        include/boost/graph/matrix_as_graph.hpp
        include/boost/graph/max_cardinality_matching.hpp
        include/boost/graph/maximum_adjacency_search.hpp
        include/boost/graph/maximum_weighted_matching.hpp
        include/boost/graph/mcgregor_common_subgraphs.hpp
        include/boost/graph/mesh_graph_generator.hpp
        include/boost/graph/metis.hpp
        include/boost/graph/metric_tsp_approx.hpp
        include/boost/graph/minimum_degree_ordering.hpp
        include/boost/graph/named_function_params.hpp
        include/boost/graph/named_graph.hpp
        include/boost/graph/neighbor_bfs.hpp
        include/boost/graph/numeric_values.hpp
        include/boost/graph/one_bit_color_map.hpp
        include/boost/graph/overloading.hpp
        include/boost/graph/page_rank.hpp
        include/boost/graph/planar_canonical_ordering.hpp
        include/boost/graph/planar_face_traversal.hpp
        include/boost/graph/plod_generator.hpp
        include/boost/graph/point_traits.hpp
        include/boost/graph/prim_minimum_spanning_tree.hpp
        include/boost/graph/profile.hpp
        include/boost/graph/properties.hpp
        include/boost/graph/property_iter_range.hpp
        include/boost/graph/push_relabel_max_flow.hpp
        include/boost/graph/r_c_shortest_paths.hpp
        include/boost/graph/random.hpp
        include/boost/graph/random_layout.hpp
        include/boost/graph/random_spanning_tree.hpp
        include/boost/graph/read_dimacs.hpp
        include/boost/graph/relax.hpp
        include/boost/graph/reverse_graph.hpp
        include/boost/graph/rmat_graph_generator.hpp
        include/boost/graph/sequential_vertex_coloring.hpp
        include/boost/graph/simple_point.hpp
        include/boost/graph/sloan_ordering.hpp
        include/boost/graph/small_world_generator.hpp
        include/boost/graph/smallest_last_ordering.hpp
        include/boost/graph/ssca_graph_generator.hpp
        include/boost/graph/st_connected.hpp
        include/boost/graph/stanford_graph.hpp
        include/boost/graph/stoer_wagner_min_cut.hpp
        include/boost/graph/strong_components.hpp
        include/boost/graph/subgraph.hpp
        include/boost/graph/successive_shortest_path_nonnegative_weights.hpp
        include/boost/graph/tiernan_all_cycles.hpp
        include/boost/graph/topological_sort.hpp
        include/boost/graph/topology.hpp
        include/boost/graph/transitive_closure.hpp
        include/boost/graph/transitive_reduction.hpp
        include/boost/graph/transpose_graph.hpp
        include/boost/graph/tree_traits.hpp
        include/boost/graph/two_bit_color_map.hpp
        include/boost/graph/two_graphs_common_spanning_trees.hpp
        include/boost/graph/undirected_dfs.hpp
        include/boost/graph/undirected_graph.hpp
        include/boost/graph/use_mpi.hpp
        include/boost/graph/vector_as_graph.hpp
        include/boost/graph/vertex_and_edge_range.hpp
        include/boost/graph/vf2_sub_graph_iso.hpp
        include/boost/graph/visitors.hpp
        include/boost/graph/wavefront.hpp
        include/boost/graph/write_dimacs.hpp
        src/graphml.cpp
        src/read_graphviz_new.cpp
    )
    target_link_libraries(boost_graph PUBLIC Boost::headers PRIVATE Boost::regex)
    include(CTest)
endif()

add_library(Boost::graph ALIAS boost_graph)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(boost_graph PUBLIC BOOST_GRAPH_DYN_LINK)
else()
  target_compile_definitions(boost_graph PUBLIC BOOST_GRAPH_STATIC_LINK)
endif()

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")

  add_subdirectory(test)

endif()

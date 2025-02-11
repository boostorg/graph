find_package(Boost 1.87.0 REQUIRED COMPONENTS filesystem serialization)

# These use Boost.Core's lightweight_test, one main() function per file.

set(TEST_SOURCES
    adj_list_cc.cpp
    adj_list_edge_list_set.cpp
    # adj_list_invalidation.cpp # ?
    adj_list_loops.cpp
    adj_matrix_cc.cpp
    adjacency_matrix_test.cpp
    astar_search_test.cpp
    basic_planarity_test.cpp
    bellman-test.cpp
    betweenness_centrality_test.cpp
    bfs.cpp
    bfs_cc.cpp
    biconnected_components_test.cpp
    bidir_remove_edge.cpp
    # bidir_vec_remove_edge.cpp # Not tested in Jamfile, fails to compile.
    bipartite_test.cpp
    boykov_kolmogorov_max_flow_test.cpp
    bron_kerbosch_all_cliques.cpp
    bundled_properties.cpp
    closeness_centrality.cpp
    clustering_coefficient.cpp
    copy.cpp
    core_numbers_test.cpp
    csr_graph_test.cpp
    cuthill_mckee_ordering.cpp
    cycle_canceling_test.cpp
    cycle_ratio_tests.cpp
    dag_longest_paths.cpp
    degree_centrality.cpp
    delete_edge.cpp
    dfs.cpp
    dfs_cc.cpp
    dijkstra_cc.cpp
    dijkstra_no_color_map_compare.cpp
    dimacs.cpp
    disjoint_set_test.cpp
    dominator_tree_test.cpp
    eccentricity.cpp
    edge_list_cc.cpp
    filter_graph_vp_test.cpp
    filtered_graph_cc.cpp
    # filtered_graph_properties_dijkstra.cpp # Compile-only, no link test??
    find_flow_cost_bundled_properties_and_named_params_test.cpp
    finish_edge_bug.cpp
    floyd_warshall_test.cpp
    generator_test.cpp
    graph_concepts.cpp
    graphml_test.cpp
    graphviz_test.cpp
    grid_graph_cc.cpp
    grid_graph_test.cpp
    gursoy_atun_layout_test.cpp
    hawick_circuits.cpp
    incremental_components_test.cpp
    index_graph.cpp
    is_straight_line_draw_test.cpp
    isomorphism.cpp
    johnson-test.cpp
    king_ordering.cpp
    labeled_graph.cpp
    layout_test.cpp
    lvalue_pmap.cpp
    make_bicon_planar_test.cpp
    make_connected_test.cpp
    make_maximal_planar_test.cpp
    mas_test.cpp
    matching_test.cpp
    max_flow_algorithms_bundled_properties_and_named_params.cpp
    max_flow_test.cpp
    mcgregor_subgraphs_test.cpp
    mean_geodesic.cpp
    metis_test.cpp
    min_degree_empty.cpp
    named_vertices_test.cpp
    r_c_shortest_paths_test.cpp
    random_matching_test.cpp
    random_spanning_tree_test.cpp
    rcsp_custom_vertex_id.cpp
    rcsp_single_solution.cpp
    read_propmap.cpp
    # reverse_graph_cc.cpp # compile-only
    sequential_vertex_coloring.cpp
    # serialize.cpp # Fails to link, but why?
    stoer_wagner_test.cpp
    strong_components_test.cpp
    subgraph.cpp
    subgraph_add.cpp
    subgraph_bundled.cpp
    subgraph_props.cpp
    successive_shortest_path_nonnegative_weights_test.cpp
    swap.cpp
    test_graphs.cpp
    tiernan_all_cycles.cpp
    # transitive_closure_test.cpp # Benchmark and test, need to separate.
    transitive_closure_test2.cpp
    two_graphs_common_spanning_trees_test.cpp
    undirected_dfs.cpp
    undirected_dfs_visitor.cpp
    # vector_graph_cc.cpp # compile-only
    vf2_sub_graph_iso_test.cpp
    vf2_sub_graph_iso_test_2.cpp
    weighted_matching_test.cpp
)

foreach(SOURCE ${TEST_SOURCES})
    cmake_path(GET SOURCE STEM LAST_ONLY TEST)
    add_executable(${TEST} ${SOURCE})
    target_link_libraries(${TEST} Boost::graph)
    add_test(${TEST} ${TEST})
endforeach()

# graph and property_iter are parameterized.

set(PARAMETERIZED_TEST_SOURCES graph.cpp property_iter.cpp)
set(TEST_PARAMETERS 1 2 3 4 5 6 7 8 9)

foreach(SOURCE ${PARAMETERIZED_TEST_SOURCES})
    foreach(PARAMETER ${TEST_PARAMETERS})
        cmake_path(GET SOURCE STEM LAST_ONLY TEST)
        set(TEST_NAME ${TEST}_${PARAMETER})
        add_executable(${TEST_NAME} ${SOURCE})
        target_compile_definitions(${TEST_NAME} PUBLIC TEST=${PARAMETER})
        target_link_libraries(${TEST_NAME} Boost::graph Boost::serialization)
        add_test(${TEST_NAME} ${TEST_NAME})
    endforeach()
endforeach()

# NOTE: Following note copied verbatim from Jamfile.v2.
# The tests below started failing to compile for xcode with cxxstd=11
# due to issues with constexpr ctors in Boost.Filesystem

# all_planar_input_files_test.cpp     
# parallel_edges_loops_test.cpp

# I don't know if these commercial packages have or will ever have a CMake config.

find_package(LEDA QUIET)
if(LEDA_FOUND)
    add_executable(leda_graph_cc leda_graph_cc.cpp)
    target_compile_definitions(leda_graph_cc)
    target_link_libraries(leda_graph_cc Boost::graph LEDA)
    add_test(leda_graph_cc leda_graph_cc)
endif()

find_package(StanfordGraphBase QUIET)
if(StanfordGraphBase_FOUND)
    add_executable(stanford_graph_cc stanford_graph_cc.cpp)
    target_compile_definitions(stanford_graph_cc)
    target_link_libraries(stanford_graph_cc Boost::graph StanfordGraphBase)
    add_test(stanford_graph_cc stanford_graph_cc)
endif()

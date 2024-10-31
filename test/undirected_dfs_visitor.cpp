//=======================================================================
// Copyright 2024
// Author: Daniel Yang
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <string>

#include <boost/config.hpp>
#include <boost/core/lightweight_test.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>

using Graph = boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_color_t, boost::default_color_type>>;

using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
using Edge = boost::graph_traits<Graph>::edge_descriptor;

struct DFSVisitorLogger : boost::default_dfs_visitor {
    std::vector<std::string> &log;

    DFSVisitorLogger(std::vector<std::string> &log) : log(log) {}

    void log_vertex(const Vertex v, const std::string &event) {
        log.push_back("vertex " + std::to_string(v) + " " + event);
    }
    void log_edge(const Edge e, const std::string &event, const Graph &g) {
        log.push_back("edge (" + std::to_string(boost::source(e, g)) + "," + std::to_string(boost::target(e, g)) + ") " + event);
    }

    void discover_vertex(Vertex v, const Graph &g) {
        log_vertex(v, "discovered");
    }
    void finish_vertex(Vertex v, const Graph &g) {
        log_vertex(v, "finished");
    }
    void examine_edge(Edge e, const Graph &g) {
        log_edge(e, "examined", g);
    }
    void tree_edge(Edge e, const Graph &g) {
        log_edge(e, "tree", g);
    }
    void back_edge(Edge e, const Graph &g) {
        log_edge(e, "back", g);
    }
    void forward_or_cross_edge(Edge e, const Graph &g) {
        log_edge(e, "forward_cross", g);
    }
    void finish_edge(Edge e, const Graph &g) {
        log_edge(e, "finished", g);
    }
};

int main() {
    Graph g(3);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);

    std::vector<std::string> expected_answer = {
        "vertex 0 discovered",
        "edge (0,1) examined",
        "edge (0,1) tree",
        "vertex 1 discovered",
        "edge (1,0) examined",
        "edge (1,0) finished",
        "edge (1,2) examined",
        "edge (1,2) tree",
        "vertex 2 discovered",
        "edge (2,1) examined",
        "edge (2,1) finished",
        "vertex 2 finished",
        "edge (1,2) finished",
        "vertex 1 finished",
        "edge (0,1) finished",
        "vertex 0 finished",
    };
    std::vector<std::string> actual_answer;

    // run undirected_dfs
    DFSVisitorLogger dfs_visitor_logger(actual_answer);
    boost::undirected_dfs(g,
        boost::visitor(dfs_visitor_logger)
        .edge_color_map(boost::get(boost::edge_color, g)));

    // check if all vertices and edges have been visited in the correct order
    BOOST_TEST(expected_answer.size() == actual_answer.size());
    if (expected_answer.size() == actual_answer.size()) {
        for (int i = 0; i < expected_answer.size(); ++i) {
            BOOST_TEST(expected_answer[i] == actual_answer[i]);
        }
    }

    return boost::report_errors();
}

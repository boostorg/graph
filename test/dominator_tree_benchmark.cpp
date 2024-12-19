//=======================================================================
// Copyright (C) 2024 Pavel Samolysov <samolisov -at- gmail.com>
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * To manually build the benchmark, please follow the instructions.
 *
 * prerequisites:
 * ~/dev/boost - the directory where the boost library has been built. See the
 *               following instruction to build boost:
 *              https://github.com/boostorg/graph?tab=readme-ov-file#development
 *
 * ~/dev/benchmark - the directory with the sources of the google benchmark
 *                   library.
 * ~/dev/benchmark/build - the directory where the google benchmark library has
 *                         been built in accordance with the following
 *                         instruction:
 *           https://github.com/google/benchmark?tab=readme-ov-file#installation
 *
 * compilation:
 * $ clang++ -O2 -o dominator_tree_benchmark -I ~/dev/boost/libs/graph/include/
 * \
 *   -I ~/dev/boost -I ~/dev/benchmark/include/ dominator_tree_benchmark.cpp \
 *   -L ~/dev/benchmark/build/src/ -lbenchmark -lpthread
 * run:
 * $ ./dominator_tree_benchmark
 */

#include "benchmark/benchmark.h"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/dominator_tree.hpp"
#include "boost/graph/graph_traits.hpp"
#include "boost/property_map/property_map.hpp"

#include <limits>
#include <utility>
#include <vector>

using namespace boost;

namespace {

using Glist =
    adjacency_list<listS, listS, bidirectionalS,
                   property<vertex_index_t, std::size_t>, no_property>;

using Gvec = adjacency_list<listS, vecS, bidirectionalS>;

struct GraphEdgeSet {
  using edge = std::pair<int, int>;
  int numOfVertices;
  std::vector<edge> edges;
};

struct GraphEdgeSetBuilder {
  int numOfVertices;
  std::vector<GraphEdgeSet::edge> edges;

  GraphEdgeSetBuilder &&vx(int numOfVertices) && {
    this->numOfVertices = numOfVertices;
    return std::move(*this);
  }

  GraphEdgeSetBuilder &&edge(int s, int e) && {
    edges.emplace_back(s, e);
    return std::move(*this);
  }

  GraphEdgeSet build() && {
    GraphEdgeSet set;
    set.numOfVertices = numOfVertices;
    set.edges = std::move(edges);
    return set;
  }
};

GraphEdgeSetBuilder make_graph() { return {}; }

} // namespace

template <typename Graph>
void BM_DominatorTree(benchmark::State &state, GraphEdgeSet testSet) {
  Graph g(testSet.edges.begin(), testSet.edges.end(), testSet.numOfVertices);

  using IndexMap = property_map<Graph, vertex_index_t>::type;
  IndexMap indexMap(get(vertex_index, g));

  using Traits = adjacency_list_traits<typename Graph::out_edge_list_selector,
                                       typename Graph::vertex_list_selector,
                                       typename Graph::directed_selector,
                                       typename Graph::edge_list_selector>;
  if constexpr (!Traits::is_rand_access::value) {
    typename graph_traits<Graph>::vertex_iterator uItr, uEnd;
    int j = 0;
    for (boost::tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr, ++j) {
      put(indexMap, *uItr, j);
    }
  }

  using Vertex = typename graph_traits<Graph>::vertex_descriptor;
  using PredMap =
      iterator_property_map<typename std::vector<Vertex>::iterator, IndexMap>;

  for (auto _ : state) {
    // Lengauer-Tarjan dominator tree algorithm
    std::vector<Vertex> domTreePredVector(num_vertices(g),
                                          graph_traits<Graph>::null_vertex());
    PredMap domTreePredMap =
        make_iterator_property_map(domTreePredVector.begin(), indexMap);

    lengauer_tarjan_dominator_tree(g, vertex(0, g), domTreePredMap);
    benchmark::DoNotOptimize(domTreePredVector.data());
  }
}

auto TarjanGraph = make_graph()
                       .vx(13)
                       .edge(0, 1)
                       .edge(0, 2)
                       .edge(0, 3)
                       .edge(1, 4)
                       .edge(2, 1)
                       .edge(2, 4)
                       .edge(2, 5)
                       .edge(3, 6)
                       .edge(3, 7)
                       .edge(4, 12)
                       .edge(5, 8)
                       .edge(6, 9)
                       .edge(7, 9)
                       .edge(7, 10)
                       .edge(8, 5)
                       .edge(8, 11)
                       .edge(9, 11)
                       .edge(10, 9)
                       .edge(11, 0)
                       .edge(11, 9)
                       .edge(12, 8)
                       .build();

BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Glist, VertexList, TarjanGraph)
    ->Name("Tarjan's paper (vertex list)")
    ->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Gvec, VertexVector, TarjanGraph)
    ->Name("Tarjan's paper  (vertex vector)")
    ->Unit(benchmark::kNanosecond);

// Appel. p449. figure 19.8
auto AppelGraph = make_graph()
                      .vx(13)
                      .edge(0, 1)
                      .edge(0, 2)
                      .edge(1, 3)
                      .edge(1, 6)
                      .edge(2, 4)
                      .edge(2, 7)
                      .edge(3, 5)
                      .edge(3, 6)
                      .edge(4, 7)
                      .edge(4, 2)
                      .edge(5, 8)
                      .edge(5, 10)
                      .edge(6, 9)
                      .edge(7, 12)
                      .edge(8, 11)
                      .edge(9, 8)
                      .edge(10, 11)
                      .edge(11, 1)
                      .edge(11, 12)
                      .build();

BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Glist, VertexList, AppelGraph)
    ->Name("Appel. fig. 19.8 (vertex list)")
    ->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Gvec, VertexVector, AppelGraph)
    ->Name("Appel. fig. 19.8  (vertex vector)")
    ->Unit(benchmark::kNanosecond);

// Muchnick. p253. figure 8.18
auto MuchnickGraph = make_graph()
                         .vx(8)
                         .edge(0, 1)
                         .edge(0, 2)
                         .edge(1, 6)
                         .edge(2, 3)
                         .edge(2, 4)
                         .edge(3, 7)
                         .edge(5, 7)
                         .edge(6, 7)
                         .build();

BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Glist, VertexList, MuchnickGraph)
    ->Name("Muchnick. fig. 8.18 (vertex list)")
    ->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Gvec, VertexVector, MuchnickGraph)
    ->Name("Muchnick. fig. 8.18  (vertex vector)")
    ->Unit(benchmark::kNanosecond);

auto CytronGraph = make_graph()
                       .vx(14)
                       .edge(0, 1)
                       .edge(0, 13)
                       .edge(1, 2)
                       .edge(2, 3)
                       .edge(2, 7)
                       .edge(3, 4)
                       .edge(3, 5)
                       .edge(4, 6)
                       .edge(5, 6)
                       .edge(6, 8)
                       .edge(7, 8)
                       .edge(8, 9)
                       .edge(9, 10)
                       .edge(9, 11)
                       .edge(10, 11)
                       .edge(11, 9)
                       .edge(11, 12)
                       .edge(12, 2)
                       .edge(12, 13)
                       .build();
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Glist, VertexList, CytronGraph)
    ->Name("Cytron's paper, fig. 9 (vertex list)")
    ->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Gvec, VertexVector, CytronGraph)
    ->Name("Cytron's paper, fig. 9  (vertex vector)")
    ->Unit(benchmark::kNanosecond);

auto RealCodeGraph = make_graph()
                         .vx(186)
                         .edge(0, 1)
                         .edge(0, 43)
                         .edge(1, 2)
                         .edge(2, 3)
                         .edge(2, 4)
                         .edge(3, 16)
                         .edge(4, 5)
                         .edge(4, 8)
                         .edge(5, 6)
                         .edge(5, 8)
                         .edge(6, 15)
                         .edge(9, 15)
                         .edge(10, 11)
                         .edge(10, 12)
                         .edge(11, 16)
                         .edge(12, 13)
                         .edge(12, 17)
                         .edge(13, 16)
                         .edge(15, 17)
                         .edge(16, 17)
                         .edge(17, 18)
                         .edge(17, 22)
                         .edge(18, 19)
                         .edge(18, 20)
                         .edge(19, 119)
                         .edge(20, 2)
                         .edge(22, 23)
                         .edge(22, 41)
                         .edge(23, 24)
                         .edge(23, 25)
                         .edge(24, 37)
                         .edge(25, 26)
                         .edge(25, 29)
                         .edge(26, 27)
                         .edge(27, 29)
                         .edge(27, 36)
                         .edge(29, 30)
                         .edge(29, 31)
                         .edge(30, 36)
                         .edge(31, 32)
                         .edge(31, 33)
                         .edge(32, 37)
                         .edge(33, 34)
                         .edge(33, 36)
                         .edge(34, 37)
                         .edge(36, 38)
                         .edge(37, 38)
                         .edge(38, 39)
                         .edge(38, 41)
                         .edge(39, 22)
                         .edge(41, 119)
                         .edge(43, 44)
                         .edge(43, 45)
                         .edge(44, 75)
                         .edge(45, 46)
                         .edge(45, 49)
                         .edge(46, 47)
                         .edge(46, 49)
                         .edge(47, 74)
                         .edge(49, 50)
                         .edge(49, 51)
                         .edge(50, 74)
                         .edge(51, 52)
                         .edge(51, 53)
                         .edge(52, 75)
                         .edge(53, 54)
                         .edge(53, 55)
                         .edge(54, 75)
                         .edge(55, 56)
                         .edge(55, 57)
                         .edge(57, 58)
                         .edge(57, 59)
                         .edge(58, 74)
                         .edge(59, 60)
                         .edge(59, 61)
                         .edge(60, 75)
                         .edge(61, 62)
                         .edge(61, 63)
                         .edge(62, 75)
                         .edge(63, 64)
                         .edge(63, 65)
                         .edge(64, 75)
                         .edge(65, 66)
                         .edge(65, 67)
                         .edge(66, 75)
                         .edge(67, 68)
                         .edge(67, 69)
                         .edge(68, 75)
                         .edge(69, 70)
                         .edge(69, 71)
                         .edge(70, 75)
                         .edge(71, 72)
                         .edge(71, 74)
                         .edge(72, 75)
                         .edge(74, 76)
                         .edge(75, 76)
                         .edge(76, 77)
                         .edge(76, 81)
                         .edge(77, 78)
                         .edge(77, 79)
                         .edge(78, 119)
                         .edge(79, 43)
                         .edge(81, 82)
                         .edge(81, 118)
                         .edge(82, 83)
                         .edge(82, 84)
                         .edge(83, 114)
                         .edge(84, 85)
                         .edge(84, 88)
                         .edge(85, 86)
                         .edge(85, 88)
                         .edge(86, 113)
                         .edge(88, 89)
                         .edge(88, 90)
                         .edge(89, 113)
                         .edge(90, 91)
                         .edge(90, 92)
                         .edge(91, 114)
                         .edge(92, 93)
                         .edge(92, 94)
                         .edge(93, 114)
                         .edge(94, 95)
                         .edge(94, 96)
                         .edge(95, 114)
                         .edge(96, 97)
                         .edge(96, 98)
                         .edge(97, 113)
                         .edge(98, 99)
                         .edge(98, 100)
                         .edge(99, 114)
                         .edge(100, 101)
                         .edge(100, 102)
                         .edge(101, 114)
                         .edge(102, 103)
                         .edge(102, 104)
                         .edge(103, 114)
                         .edge(104, 105)
                         .edge(104, 106)
                         .edge(105, 114)
                         .edge(106, 107)
                         .edge(106, 108)
                         .edge(107, 114)
                         .edge(108, 109)
                         .edge(108, 110)
                         .edge(109, 114)
                         .edge(110, 111)
                         .edge(110, 113)
                         .edge(111, 114)
                         .edge(113, 115)
                         .edge(114, 115)
                         .edge(115, 116)
                         .edge(115, 118)
                         .edge(116, 81)
                         .edge(118, 119)
                         .edge(119, 120)
                         .edge(119, 121)
                         .edge(120, 183)
                         .edge(121, 122)
                         .edge(121, 125)
                         .edge(122, 123)
                         .edge(122, 124)
                         .edge(123, 184)
                         .edge(124, 182)
                         .edge(125, 126)
                         .edge(125, 171)
                         .edge(126, 127)
                         .edge(126, 135)
                         .edge(127, 128)
                         .edge(128, 129)
                         .edge(128, 130)
                         .edge(129, 143)
                         .edge(130, 131)
                         .edge(130, 132)
                         .edge(131, 143)
                         .edge(132, 133)
                         .edge(132, 135)
                         .edge(133, 128)
                         .edge(135, 136)
                         .edge(135, 142)
                         .edge(136, 137)
                         .edge(137, 138)
                         .edge(137, 139)
                         .edge(138, 143)
                         .edge(139, 140)
                         .edge(139, 142)
                         .edge(140, 143)
                         .edge(143, 144)
                         .edge(143, 145)
                         .edge(144, 146)
                         .edge(145, 146)
                         .edge(146, 147)
                         .edge(146, 148)
                         .edge(147, 184)
                         .edge(148, 149)
                         .edge(148, 162)
                         .edge(149, 150)
                         .edge(149, 154)
                         .edge(150, 151)
                         .edge(151, 152)
                         .edge(151, 153)
                         .edge(152, 151)
                         .edge(153, 154)
                         .edge(154, 155)
                         .edge(154, 156)
                         .edge(155, 156)
                         .edge(156, 157)
                         .edge(156, 162)
                         .edge(157, 158)
                         .edge(158, 159)
                         .edge(158, 161)
                         .edge(159, 158)
                         .edge(161, 182)
                         .edge(162, 163)
                         .edge(163, 164)
                         .edge(163, 165)
                         .edge(164, 166)
                         .edge(165, 163)
                         .edge(166, 167)
                         .edge(164, 168)
                         .edge(167, 169)
                         .edge(168, 170)
                         .edge(169, 167)
                         .edge(170, 182)
                         .edge(171, 172)
                         .edge(171, 173)
                         .edge(172, 184)
                         .edge(173, 174)
                         .edge(174, 175)
                         .edge(174, 174)
                         .edge(175, 177)
                         .edge(176, 174)
                         .edge(177, 178)
                         .edge(178, 179)
                         .edge(178, 180)
                         .edge(179, 181)
                         .edge(180, 178)
                         .edge(181, 182)
                         .edge(184, 183)
                         .edge(183, 185)
                         .edge(184, 185)
                         .build();

BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Glist, VertexList, RealCodeGraph)
    ->Name("From a code, 186 BBs (vertex list)")
    ->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE1_CAPTURE(BM_DominatorTree, Gvec, VertexVector, RealCodeGraph)
    ->Name("From a code, 186 BBs (vertex vector)")
    ->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();

//=======================================================================
// Copyright (C) 2005 Jong Soo Park <jongsoo.park -at- gmail.com>
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/test/minimal.hpp>
#include <iostream>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dominator_tree.hpp>

using namespace std;

struct DominatorCorrectnessTestSet
{
  typedef pair<int, int> edge;

  int numOfVertices;
  int numOfEdges;
  edge edges[100];
  int correctIdoms[100];
};

using namespace boost;

typedef adjacency_list<
    listS,
    listS,
    bidirectionalS,
    property<vertex_index_t, std::size_t>, no_property> G;

int test_main(int, char*[])
{
  typedef DominatorCorrectnessTestSet::edge edge;
  DominatorCorrectnessTestSet testSet[] =
    {
      // tarjan's dissertation
      {
        13,
        21,
        {
          edge(0, 1),
          edge(0, 2),
          edge(0, 3),
          edge(1, 4),
          edge(2, 1),
          edge(2, 4),
          edge(2, 5),
          edge(3, 6),
          edge(3, 7),
          edge(4, 12),
          edge(5, 8),
          edge(6, 9),
          edge(7, 9),
          edge(7, 10),
          edge(8, 5),
          edge(8, 11),
          edge(9, 11),
          edge(10, 9),
          edge(11, 0),
          edge(11, 9),
          edge(12, 8),
        },
        {
          (numeric_limits<int>::max)(), 0, 0, 0, 0, 0, 3, 3, 0, 0, 7, 0, 4,
        },
      },
      // appel. p441. figure 19.4
      {
        7,
        8,
        {
          edge(0, 1),
          edge(1, 2),
          edge(1, 3),
          edge(2, 4),
          edge(2, 5),
          edge(4, 6),
          edge(5, 6),
          edge(6, 1),
        },
        {
          (numeric_limits<int>::max)(), 0, 1, 1, 2, 2, 2,
        },
      },
      // appel. p449. figure 19.8
      {
        13,
        19,
        {
          edge(0, 1),
          edge(0, 2),
          edge(1, 3),
          edge(1, 6),
          edge(2, 4),
          edge(2, 7),
          edge(3, 5),
          edge(3, 6),
          edge(4, 7),
          edge(4, 2),
          edge(5, 8),
          edge(5, 10),
          edge(6, 9),
          edge(7, 12),
          edge(8, 11),
          edge(9, 8),
          edge(10, 11),
          edge(11, 1),
          edge(11, 12),
        },
        {
          (numeric_limits<int>::max)(), 0, 0, 1, 2, 3, 1, 2, 1, 6, 5, 1, 0,
        },
      },
      {
        8,
        9,
        {
          edge(0, 1),
          edge(1, 2),
          edge(1, 3),
          edge(2, 7),
          edge(3, 4),
          edge(4, 5),
          edge(4, 6),
          edge(5, 7),
          edge(6, 4),
        },
        {
          (numeric_limits<int>::max)(), 0, 1, 1, 3, 4, 4, 1,
        },
      },
      // muchnick. p256. figure 8.21
      {
        8,
        9,
        {
          edge(0, 1),
          edge(1, 2),
          edge(2, 3),
          edge(2, 4),
          edge(3, 2),
          edge(4, 5),
          edge(4, 6),
          edge(5, 7),
          edge(6, 7),
        },
        {
          (numeric_limits<int>::max)(), 0, 1, 2, 2, 4, 4, 4,
        },
      },
      // muchnick. p253. figure 8.18
      {
        8,
        8,
        {
          edge(0, 1),
          edge(0, 2),
          edge(1, 6),
          edge(2, 3),
          edge(2, 4),
          edge(3, 7),
          edge(5, 7),
          edge(6, 7),
        },
        {
          (numeric_limits<int>::max)(), 0, 0, 2, 2, (numeric_limits<int>::max)(), 1, 0,
        },
      },
      // cytron's thesis, fig. 9
      {
        14,
        19,
        {
          edge(0, 1),
          edge(0, 13),
          edge(1, 2),
          edge(2, 3),
          edge(2, 7),
          edge(3, 4),
          edge(3, 5),
          edge(4, 6),
          edge(5, 6),
          edge(6, 8),
          edge(7, 8),
          edge(8, 9),
          edge(9, 10),
          edge(9, 11),
          edge(10, 11),
          edge(11, 9),
          edge(11, 12),
          edge(12, 2),
          edge(12, 13),
        },
        {
          (numeric_limits<int>::max)(), 0, 1, 2, 3,
          3, 3, 2, 2, 8,
          9, 9, 11, 0,
        },
      },
    };

  for (size_t i = 0; i < sizeof(testSet)/sizeof(testSet[0]); ++i)
    {
      const int numOfVertices = testSet[i].numOfVertices;

      G g(
          testSet[i].edges, testSet[i].edges + testSet[i].numOfEdges,
          numOfVertices);

      typedef graph_traits<G>::vertex_descriptor Vertex;
      typedef property_map<G, vertex_index_t>::type IndexMap;
      typedef
        iterator_property_map<vector<Vertex>::iterator, IndexMap>
        PredMap;

      vector<Vertex> domTreePredVector, domTreePredVector2;
      IndexMap indexMap(get(vertex_index, g));
      graph_traits<G>::vertex_iterator uItr, uEnd;
      int j = 0;
      for (tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr, ++j)
        {
          put(indexMap, *uItr, j);
        }

      // lengauer-tarjan dominator tree algorithm
      domTreePredVector =
        vector<Vertex>(num_vertices(g), graph_traits<G>::null_vertex());
      PredMap domTreePredMap =
        make_iterator_property_map(domTreePredVector.begin(), indexMap);

      lengauer_tarjan_dominator_tree(g, vertex(0, g), domTreePredMap);

      vector<int> idom(num_vertices(g));
      for (tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr)
        {
          if (get(domTreePredMap, *uItr) != graph_traits<G>::null_vertex())
            idom[get(indexMap, *uItr)] =
              get(indexMap, get(domTreePredMap, *uItr));
          else
            idom[get(indexMap, *uItr)] = (numeric_limits<int>::max)();
        }

      copy(idom.begin(), idom.end(), ostream_iterator<int>(cout, " "));
      cout << endl;

      // dominator tree correctness test
      BOOST_CHECK(equal(idom.begin(), idom.end(), testSet[i].correctIdoms));

      // compare results of fast version and slow version of dominator tree
      domTreePredVector2 =
        vector<Vertex>(num_vertices(g), graph_traits<G>::null_vertex());
      domTreePredMap =
        make_iterator_property_map(domTreePredVector2.begin(), indexMap);

      iterative_bit_vector_dominator_tree(g, vertex(0, g), domTreePredMap);

      vector<int> idom2(num_vertices(g));
      for (tie(uItr, uEnd) = vertices(g); uItr != uEnd; ++uItr)
        {
          if (get(domTreePredMap, *uItr) != graph_traits<G>::null_vertex())
            idom2[get(indexMap, *uItr)] =
              get(indexMap, get(domTreePredMap, *uItr));
          else
            idom2[get(indexMap, *uItr)] = (numeric_limits<int>::max)();
        }

      copy(idom2.begin(), idom2.end(), ostream_iterator<int>(cout, " "));
      cout << endl;

      size_t k;
      for (k = 0; k < num_vertices(g); ++k)
        BOOST_CHECK(domTreePredVector[k] == domTreePredVector2[k]);
    }

  return 0;
};

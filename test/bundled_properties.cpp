// Boost Graph library

//  Copyright Doug Gregor 2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/minimal.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <vector>

using namespace std;

struct City
{
  string name;
  int population;
  vector<int> zipcodes;
};

struct Highway
{
  string name;
  double miles;
  int speed_limit;
  int lanes;
  bool divided;
};

typedef boost::adjacency_list<
    boost::listS, boost::vecS, boost::bidirectionalS,
    City, Highway> Map;

int test_main(int, char*[])
{
  Map map(2);

  Map::vertex_iterator vi = vertices(map).first;
  Map::vertex_descriptor v = *vi;
  map[v].name = "Troy";
  map[v].population = 49170;
  map[v].zipcodes.push_back(12180);

  ++vi;
  Map::vertex_descriptor u = *vi;
  map[u].name = "Albany";
  map[u].population = 95658;
  map[u].zipcodes.push_back(12201);

  Map::edge_descriptor e = add_edge(v, u, map).first;
  map[e].name = "I-87";
  map[e].miles = 10;
  map[e].speed_limit = 65;
  map[e].lanes = 4;
  map[e].divided = true;

  BOOST_TEST(get(map->*&City::name, v) == "Troy");
  BOOST_TEST(get(map->*&Highway::name, e) == "I-87");

  return 0;
}

// Copyright (C) 2001 Vladimir Prus <ghost@cs.msu.su>
// Permission to copy, use, modify, sell and distribute this software is
// granted, provided this copyright notice appears in all copies and 
// modified version are clearly marked as such. This software is provided
// "as is" without express or implied warranty, and with no claim as to its
// suitability for any purpose.


#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/transitive_closure.hpp>

#include <iostream>

#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <cstdlib>
#include <ctime>
#include <boost/progress.hpp>
using namespace std;
using namespace boost;

void generate_graph(int n, double p, vector< vector<int> >& r1,
          vector<vector<bool> >& r2)
{
  static class {
  public:
    double operator()() {
      return double(rand())/RAND_MAX;
    }
  } gen;  

  r1.clear();
  r2.clear();

  r1.resize(n);
  r2.resize(n, vector<bool>(n));

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) 
      if (gen() < p) {
        r1[i].push_back(j);
        r2[i][j] = true;
      }   
}

bool compare_graphs(const vector< vector<int> >& g1,
            const vector< vector<bool> >& g2)
{
  int ne1(0);
  {
    for (size_t i = 0; i < g1.size(); ++i)
      ne1 += g1[i].size();      
  }
  int ne2(0);
  {
    for (size_t i = 0; i < g2.size(); ++i)
      for (size_t j = 0; j < g2.size(); ++j)
        if (g2[i][j])
          ++ne2;
  }
  if (ne1 == ne2) {

    for (size_t i = 0; i < g1.size(); ++i)
      for (size_t j = 0; j < g1[i].size(); ++j)
        if (!g2[i][g1[i][j]])
          return false;

  } else {
    return false;
  }
  return true;    
}

void warshall_transitive_closure(vector< vector<bool> >& g)
{
  for (size_t k = 0; k < g.size(); ++k)
    for (size_t i = 0; i < g.size(); ++i)
      for (size_t j = 0; j < g.size(); ++j) 
        g[i][j] = g[i][j] || (g[i][k] && g[k][j]);        
}

bool test(int n, double p)
{
  vector< vector<int> > g1;
  vector< vector<bool> > g2;


  generate_graph(n, p, g1, g2);
  cout << "Created graph with " << n << " vertices.\n";

  vector< vector<int> > g1_c(g1);

  {
    progress_timer t;
    cout << "transitive_closure" << endl;
    transitive_closure(g1);
    cout << "warshall_transitive_closure" << endl;
    warshall_transitive_closure(g2);
  }

  if(compare_graphs(g1, g2))
    return true;
  else {
    //cout << "Original graph was " << multiline << g1_c << endl;
    //cout << "Result is " << multiline << g1 << endl;
    return false;
  }
}


int main()
{
  srand(time(0));
  static class {
  public:
    double operator()() {
      return double(rand())/RAND_MAX;
    }
  } gen;  


  for (size_t i = 0; i < 100; ++i) {
    int n = 0 + int(20*gen());
    double p = gen();
    if (!test(n, p)) {
      cout << "Failed." << endl;
      return 1; 
    }
  }
  cout << "Passed." << endl;
}


# Copyright 2005 The Trustees of Indiana University.

# Use, modification and distribution is subject to the Boost Software
# License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

#  Authors: Douglas Gregor
#           Andrew Lumsdaine

# Note: this code requires the Boost Graph Library bindings for Python
# and the priodict module. The latter is part of the Python NMS
# library (http://pynms.sourceforge.net/).

import bgl
import priodict

class pqueue(bgl.Digraph.VertexQueue):
  def __init__(self, distance):
    bgl.Digraph.VertexQueue.__init__(self)
    self.Q = priodict.priorityDictionary()
    self.distance = distance
  def empty(self):
    return self.Q == {}
  def top(self):
    return self.Q.smallest();
  def pop(self):
    del self.Q[self.Q.smallest()]
  def push(self, x):
    self.Q[x] = self.distance[x]
  def update(self, x, v):
    self.distance[x] = v
    self.Q[x] = v

class dijkstra_bfs_visitor(bgl.Digraph.BFSVisitor):
  def __init__(self, Q, weight, distance, predecessor):
    bgl.Digraph.BFSVisitor.__init__(self)
    self.Q = Q
    self.weight = weight
    self.distance = distance
    self.predecessor = predecessor
   
  def tree_edge(self, e, g):
    (u, v) = (g.source(e), g.target(e))
    self.distance[v] = self.distance[u] + self.weight[e]   
    self.predecessor[v] = u

  def gray_target(self, e, g):
    (u, v) = (g.source(e), g.target(e))
    if self.distance[u] + self.weight[e] < self.distance[v]:
      self.Q.update(v, self.distance[u] + self.weight[e])
      self.predecessor[v] = u;
    

g = bgl.Digraph()

# Create vertices in the graph
name = g.get_vertex_string_map("node_id")
A = g.add_vertex()
name[A] = "A"
B = g.add_vertex()
name[B] = "B"
C = g.add_vertex()
name[C] = "C"
D = g.add_vertex()
name[D] = "D"
E = g.add_vertex()
name[E] = "E"

# Create (weighted) edges in the graph
weight = g.get_edge_double_map("label")
weight[g.add_edge(A, C)] = 1
weight[g.add_edge(B, B)] = 2
weight[g.add_edge(B, D)] = 1
weight[g.add_edge(B, E)] = 2.5
weight[g.add_edge(C, B)] = 7
weight[g.add_edge(C, D)] = 3
weight[g.add_edge(D, E)] = 1
weight[g.add_edge(E, A)] = 1
weight[g.add_edge(E, B)] = 1

# Initialize property maps
predecessor = {}
distance = g.get_vertex_double_map("distance_from_A")
for v in g.vertices:
  predecessor[v] = v
  distance[v] = 1e100

# Run breadth-first search to compute shortest paths
distance[A] = 0
buf = pqueue(distance)

bgl.breadth_first_search(g, A, buf, 
                         dijkstra_bfs_visitor(buf,weight,distance,predecessor),
                         color_map = g.get_vertex_color_map("color"))

class show_relaxed_edges(bgl.Digraph.DijkstraVisitor):
  def edge_relaxed(self, e, g):
    text = "Relaxed edge (" + name[g.source(e)] + ", " + name[g.target(e)] + ")"
    print text

# Run Dijkstra's algorithm to compute shortest paths
distance2 = g.get_vertex_double_map("distance_from_A_also");
bgl.dijkstra_shortest_paths(g, A, distance_map = distance2, 
                            visitor=show_relaxed_edges(),
                            weight_map = g.get_edge_double_map("label"));
                               
# Emit graph
g.write_graphviz("dijkstra-example.dot")



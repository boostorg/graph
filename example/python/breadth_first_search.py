from bgl import *

class bfs_print_discover_visitor(Graph.BFSVisitor):
  def bfs_print_discover_visitor(self, dtime_map):
    Graph.BFSVisitor.__init__(self)

  def discover_vertex(self, u, g):
    print "Discovered vertex ",
    print u

g = Graph((("r", "s"), ("r", "v"), ("s", "w"), ("w", "r"), ("w", "t"), ("w", "x"), ("x", "t"), ("t", "u"), ("x", "y"), ("u", "y")), "label")


breadth_first_search(g, s, visitor=bfs_print_discover_visitor())


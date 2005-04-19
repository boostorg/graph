from bgl import *

class bfs_print_discover_visitor(Graph.BFSVisitor):
  def discover_vertex(self, u, g):
    print "Discovered vertex ",
    print u

g = Graph((("r", "s"), ("r", "v"), ("s", "w"), ("w", "r"), ("w", "t"), ("w", "x"), ("x", "t"), ("t", "u"), ("x", "y"), ("u", "y")))

iter = g.vertices.__iter__()
iter.next()
s = iter.next()
breadth_first_search(g, s, visitor=bfs_print_discover_visitor())


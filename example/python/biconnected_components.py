# Copyright 2005 The Trustees of Indiana University.

# Use, modification and distribution is subject to the Boost Software
# License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

#  Authors: Douglas Gregor
#           Andrew Lumsdaine

import bgl

g = bgl.Graph("biconnected_components.dot", bgl.file_kind.graphviz)
art_points = bgl.biconnected_components(g, g.get_edge_int_map("label"));
g.write_graphviz("biconnected_components_out.dot")

print "Articulation points: ",
node_id = g.get_vertex_string_map("node_id")
for v in art_points:
  print node_id[v],
  print " ",
print ""

# Copyright 2005 The Trustees of Indiana University.

# Use, modification and distribution is subject to the Boost Software
# License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

#  Authors: Douglas Gregor
#           Andrew Lumsdaine

import bgl

g = bgl.Graph("biconnected_components.dot", bgl.file_kind.graphviz)
bgl.kamada_kawai_spring_layout(g, side_length=400)
g.write_graphviz("kamada_kawai_spring_layout_out.dot")

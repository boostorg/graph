#!/bin/sh

# Copyright (C) 2009 The Trustees of Indiana University.
# Use, modification and distribution is subject to the Boost Software
# License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Authors: Jeremiah Willcock, Andrew Lumsdaine

for i in read_graphml read_graphviz write_graphml; do
  rst2html.py -gdt --link-stylesheet --traceback --trim-footnote-reference-space --footnote-references=superscript --stylesheet=../../../rst.css $i.rst > $i.html
done
# Also see grid_graph_export_png.sh for figure conversions

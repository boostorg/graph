# -*- makefile -*-

.SUFFIXES:  .fig .gif

.fig.gif:
	fig2dev -L gif $*.fig > $*.gif



FIG	= \
analogy.fig       dfs_example.fig             quick_start.fig \
back_edges.fig    dfs_family.fig              stl_iter.fig    \
bfs_example.fig   dfs_visitor.fig             tree_edges.fig  \
bfs_family.fig    disjoint_set_family.fig     visitor.fig     \
bfs_visitor.fig   file_dep.fig					    \
concepts.fig      forward_or_cross_edges.fig  digraph.fig     \
undigraph.fig     adj_matrix.fig              adj_list.fig    \
edge_list.fig     dfs.fig                     knights_tour.fig \
search_states.fig graph_search.fig


GIF	= $(FIG:.fig=.gif)

gifs: $(GIF)

clean:
	/bin/rm -f *.gif




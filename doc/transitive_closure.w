\documentclass[11pt]{report}

\input{defs}


\setlength\overfullrule{5pt}
\tolerance=10000
\sloppy
\hfuzz=10pt

\makeindex

\begin{document}

\title{An Implementation of Transitive Closure}
\author{Jeremy G. Siek}

\maketitle

\section{Introduction}

This paper documents the implementation of the
\code{transitive\_closure()} function of the Boost Graph Library.  The
function was implemented by Vladimir Prus.

The algorithm used to implement the \code{transitive\_closure()}
function is based on the detection of strong components
\cite{nuutila95, purdom70}. The following discussion describes the
main ideas of the algorithm and some relevant background theory.

The \keyword{transitive closure} of a graph $G = (V,E)$ is a graph $G^+
= (V,E*)$ such that $E^+$ contains an edge $(u,v)$ if and only if $G$
contains a path (of at least one edge) from $u$ to $v$.  A
\keyword{successor set} of a vertex $v$, denoted by $Succ(v)$, is the
set of vertices that are reachable from vertex $v$. The set of
vertices adjacent to $v$ in the transitive closure $G^+$ is the same as
the successor set of $v$ in the original graph $G$. Computing the
transitive closure is equivalent to computing the successor set for
every vertex in $G$.

All vertices in the same strong component have the same successor set
(because every vertex is reachable from all the other vertices in the
component). Therefore, it is redundant to compute the successor set
for every vertex in a strong component; it suffices to compute it for
just one vertex per component.

A \keyword{condensation graph} is a a graph $G'=(V',E')$ based on the
graph $G=(V,E)$ where each vertex in $V'$ corresponds to a strongly
connected component in $G$ and the edge $(s,t)$ is in $E'$ if and only
if there exists an edge in $E$ connecting any of the vertices in the
component of $s$ to any of the vertices in the component of $t$.

\section{The Implementation}

The following is the interface and outline of the function:

@d Transitive Closure Function
@{
template <typename Graph, typename GraphTC, typename VertexIndexMap>
void transitive_closure(const Graph& g, GraphTC& tc, VertexIndexMap index_map)
{
  @<Some type definitions@>
  @<Compute strongly connected components of the graph@>
  @<Construct the condensation graph (version 2)@>
  @<Compute transitive closure on the condensation graph@>
  @<Build transitive closure of the original graph@>
}
// Alternate interface with default vertex index map
template <typename Graph, typename GraphTC>
void transitive_closure(const Graph& g, GraphTC& tc)
{
  transitive_closure(g, tc, get(vertex_index, g));
}
@}

\noindent To simplify the code in the rest of the function we make the
following typedefs.

@d Some type definitions
@{
typedef typename graph_traits<Graph>::vertex_descriptor vertex;
typedef typename graph_traits<Graph>::edge_descriptor edge;
typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
typedef typename property_traits<VertexIndexMap>::value_type size_type;
typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;
@}

The first step of the algorithm is to compute which vertices are in
each strongly connected component (SCC) of the graph. This is done
with the \code{strong\_components()} function. The result of this
function is stored in the \code{component\_num} array which maps each
vertex to the number of the SCC to which it belongs (the components
are numbered zero through \code{num\_scc}). Later we will need
efficient access to all vertices in the same SCC so we create a
\code{std::vector} of vertices for each SCC. Later we will use the SCC
numbers for vertices in the condensation graph (CG), so we use the
same integer type \code{cg\_vertex} for both.

@d Compute strongly connected components of the graph
@{
typedef size_type cg_vertex;
std::vector<cg_vertex> component_number(num_vertices(g));
int num_scc = strong_components(g, 
  make_iterator_property_map(&component_number[0], index_map),
  vertex_index_map(index_map));
std::vector< std::vector<vertex> > components;
components.resize(num_scc);
vertex_iterator vi, vi_end;
for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
  components[component_number[get(index_map, *vi)]].push_back(*vi);
@}

The next step is to construct the condensation graph.  There will be
one vertex in the CG for every strongly connected component in the
original graph. We will add an edge to the CG whenever there is one or
more edges in the original graph that has its source in one SCC and
its target in another SCC.  The data structure we will use for the CG
is an adjacency-list with a \code{std::set} for each out-edge list. We
use \code{std::set} because it will automatically discard parallel
edges. This makes the code simpler since we can just call
\code{insert()} every time there is an edge connecting two SCCs in the
original graph.

@d Construct the condensation graph (version 1)
@{
typedef std::vector< std::set<cg_vertex> > CG_t;
CG_t CG(num_scc);
for (cg_vertex s = 0; s < components.size(); ++s) {
  for (size_type i = 0; i < components[s].size(); ++i) {
    vertex u = components[s][i];
    adjacency_iterator vi, vi_end;
    for (tie(vi, vi_end) = adjacent_vertices(u, g); vi != vi_end; ++vi) {
      cg_vertex t = component_number[get(index_map, *vi)];
      if (s != t) // Avoid loops in the condensation graph
        CG[s].insert(t); // add edge (s,t) to the condensation graph
    }
  }
}
@}

Inserting into a \code{std::set} and iterator traversal for
\code{std::set} is a bit slow. We can get better performance if we use
\code{std::vector} and then explicitly remove duplicated vertices from
the out-edge lists. Here is the construction of the condensation graph
rewritten to use \code{std::vector}.

@d Construct the condensation graph (version 2)
@{
typedef std::vector< std::vector<cg_vertex> > CG_t;
CG_t CG(num_scc);
for (cg_vertex s = 0; s < components.size(); ++s) {
  std::vector<cg_vertex> adj;
  for (size_type i = 0; i < components[s].size(); ++i) {
    vertex u = components[s][i];
    adjacency_iterator v, v_end;
    for (tie(v, v_end) = adjacent_vertices(u, g); v != v_end; ++v) {
      cg_vertex t = component_number[get(index_map, *v)];
      if (s != t) // Avoid loops in the condensation graph
        adj.push_back(t);
    }
  }
  std::sort(adj.begin(), adj.end());
  std::vector<cg_vertex>::iterator di = std::unique(adj.begin(), adj.end());
  if (di != adj.end())
    adj.erase(di, adj.end());        
  CG[s] = adj;
}
@}

Now we compute the transitive closure of the condensation graph.  The
basic outline of the algorithm is below. The vertices are considered
in reverse topological order to ensure that the when computing the
successor set for a vertex $u$, the successor set for each vertex in
$Adj[u]$ has already been computed. The successor set for a vertex $u$
can then be constructing by taking the union of the successor sets for
all of its adjacent vertices together with the adjacent vertices
themselves.

\begin{tabbing}
\textbf{for} \= ea\=ch \= vertex $u$ in $G'$ in reverse topological order \\
\>\textbf{for} each vertex $v$ in $Adj[u]$ \\
\>\>if ($v \notin Succ(u)$) \\
\>\>\>$Succ(u)$ := $Succ(u) \cup \{ v \} \cup Succ(v)$
\end{tabbing}

An optimized implementation of the set union operation improves the
performance of the algorithm. Therefore this implementation uses
\keyword{chain decomposition}\cite{goral79,simon86}. The vertices of
$G$ are partitioned into chains $Z_1, ..., Z_k$, where each chain
$Z_i$ is a path in $G$ and the vertices in a chain have increasing
topological number.  A successor set $S$ is then represented by a
collection of intersections with the chains, i.e., $S =
\bigcup_{i=1 \ldots k} (Z_i \cap S)$. Each intersection can be represented
by the first vertex in the path $Z_i$ that is also in $S$, since the
rest of the path is guaranteed to also be in $S$. The collection of
intersections is therefore represented by a vector of length $k$ where
the ith element of the vector stores the first vertex in the
intersection of $S$ with $Z_i$.

Computing the union of two successor sets, $S_3 = S_1 \cup S_2$, can
then be computed in $O(k)$ time with the below operation. We will
represent the successor sets by vectors of integers where the integers
are the topological numbers for the vertices in the set.

@d Union of successor sets
@{
namespace detail {
  void union_successor_sets(const std::vector<std::size_t>& s1,
			  const std::vector<std::size_t>& s2,
			  std::vector<std::size_t>& s3)
  {
    for (std::size_t k = 0; k < s1.size(); ++k)
      s3[k] = std::min(s1[k], s2[k]);
  }
} // namespace detail
@}

So to compute the transitive closure we must first sort the graph by
topological number and then decompose the graph into chains. Once
that is accomplished we can enter the main loop and begin computing
the successor sets.

@d Compute transitive closure on the condensation graph
@{
  @<Compute topological number for each vertex@>
  @<Sort the out-edge lists by topological number@>
  @<Decompose the condensation graph into chains@>
  @<Compute successor sets@>
  @<Build the transitive closure of the condensation graph@>
@}

The \code{topological\_sort()} function is called to obtain a list of
vertices in topological order and then we use this ordering to assign
topological numbers to the vertices.

@d Compute topological number for each vertex
@{
std::vector<cg_vertex> topo_order;
std::vector<cg_vertex> topo_number(num_vertices(CG));
topological_sort(CG, std::back_inserter(topo_order), 
  vertex_index_map(identity_property_map()));
std::reverse(topo_order.begin(), topo_order.end());
size_type n = 0;
for (std::vector<cg_vertex>::iterator i = topo_order.begin();
     i != topo_order.end(); ++i)
  topo_number[*i] = n++;
@}

Next we sort the out-edge lists of the condensation graph by
topological number. This is needed for computing the chain
decomposition, for each the vertices in a chain must be in topological
order and we will be adding vertices to the chains from the out-edge
lists. The \code{subscript()} function creates a function object that
returns the topological number of its input argument.

@d Sort the out-edge lists by topological number
@{
for (size_type i = 0; i < num_vertices(CG); ++i)
  std::sort(CG[i].begin(), CG[i].end(), 
            compose_f_gx_hy(std::less<cg_vertex>(),
                            detail::subscript(topo_number),
                            detail::subscript(topo_number)));   
@}

Here is the code that defines the \code{subscript\_t} function object
and its associated helper object generation function.

@d Subscript function object
@{
namespace detail {
  template <typename Container, typename ST = std::size_t, 
    typename VT = typename Container::value_type>
  struct subscript_t : std::unary_function<ST, VT> {
    subscript_t(Container& c) : container(&c) {};
    VT& operator()(const ST& i) const { return (*container)[i]; }
  protected:
    Container *container;
  };
  template <typename Container>
  subscript_t<Container> subscript(Container& c)
  { return subscript_t<Container>(c); }
} // namespace detail
@}


Now we are ready to decompose the condensation graph into chains.  The
idea is that we want to form lists of vertices that are in a path and
that the vertices in the list should be ordered by topological number.
These lists will be stored in the \code{chains} vector below.  To
create the chains we consider each vertex in the graph in topological
order. If the vertex is not already in a chain then it will be the
start of a new chain. We then follow a path from this vertex to extend
the chain.

@d Decompose the condensation graph into chains
@{
std::vector< std::vector<cg_vertex> > chains;
{
  std::vector<cg_vertex> in_a_chain(num_vertices(CG));
  for (std::vector<cg_vertex>::iterator i = topo_order.begin();
       i != topo_order.end(); ++i) {
    cg_vertex v = *i;
    if (!in_a_chain[v]) {
      chains.resize(chains.size() + 1);
      std::vector<cg_vertex>& chain = chains.back();
      for (;;) {
        @<Extend the chain until the path dead-ends@>
      }
    }
  }
}
@<Record the chain number and chain position for each vertex@>
@}

\noindent To extend the chain we pick an adjacent vertex that is not
already in a chain. Also, the adjacent vertex chosen will be the one
with lowest topological number since the out-edges of \code{CG} are in
topological order.

@d Extend the chain until the path dead-ends
@{
chain.push_back(v);
in_a_chain[v] = true;
graph_traits<CG_t>::adjacency_iterator adj_first, adj_last;
tie(adj_first, adj_last) = adjacent_vertices(v, CG);
graph_traits<CG_t>::adjacency_iterator next
  = std::find_if(adj_first, adj_last, not1(detail::subscript(in_a_chain)));
if (next != adj_last)
  v = *next;
else
  break; // end of chain, dead-end
@}

In the next steps of the algorithm we will need to efficiently find
the chain for a vertex and the position in the chain for a vertex, so
here we compute this information and store it in two vectors:
\code{chain\_number} and \code{pos\_in\_chain}.

@d Record the chain number and chain position for each vertex
@{
std::vector<size_type> chain_number(num_vertices(CG));
std::vector<size_type> pos_in_chain(num_vertices(CG));
for (size_type i = 0; i < chains.size(); ++i)
  for (size_type j = 0; j < chains[i].size(); ++j) {
    cg_vertex v = chains[i][j];
    chain_number[v] = i;
    pos_in_chain[v] = j;
  }             
@}

Now that we have completed the chain decomposition we are ready to
write the main loop for computing the transitive closure of the
condensation graph. The output of this will be a successor set for
each vertex. Remember that the successor set is stored as a collection
of intersections with the chains. Each successor set is represented by
a vector where the ith element is the representative vertex for the
intersection of the set with the ith chain. We compute the successor
sets for every vertex in decreasing topological order. The successor
set for each vertex is the union of the successor sets of the adjacent
vertex plus the adjacent vertices themselves.

@d Compute successor sets
@{
cg_vertex inf = std::numeric_limits<cg_vertex>::max();
std::vector< std::vector<cg_vertex> > successors(num_vertices(CG),
  std::vector<cg_vertex>(chains.size(), inf));
for (std::vector<cg_vertex>::reverse_iterator i = topo_order.rbegin();
     i != topo_order.rend(); ++i) {
  cg_vertex u = *i;
  graph_traits<CG_t>::adjacency_iterator adj, adj_last;
  for (tie(adj, adj_last) = adjacent_vertices(u, CG);
       adj != adj_last; ++adj) {
    cg_vertex v = *adj;
    if (topo_number[v] < successors[v][chain_number[v]]) {
      // Succ(u) = Succ(u) U Succ(v)
      detail::union_successor_sets(successors[u], successors[v], 
        successors[u]);
      // Succ(u) = Succ(u) U {v}
      successors[u][chain_number[v]] = topo_number[v];
    }
  }
}
@}

We now rebuild the condensation graph, adding in edges to connect each
vertex to every vertex in its successor set, thereby obtaining the
transitive closure. The successor set vectors contain topological
numbers, so we map back to vertices using the \code{topo\_order}
vector.

@d Build the transitive closure of the condensation graph
@{
for (size_type i = 0; i < CG.size(); ++i)
  CG[i].clear();
for (size_type i = 0; i < CG.size(); ++i) 
  for (size_type j = 0; j < chains.size(); ++j) {
    size_type topo_num = successors[i][j];
    if (topo_num < inf) {
      cg_vertex v = topo_order[topo_num];
      for (size_type k = pos_in_chain[v]; k < chains[j].size(); ++k)
        CG[i].push_back(chains[j][k]);
    }
  }
@}

The last stage is to create the transitive closure graph $G^+$ based on
the transitive closure of the condensation graph $G'^+$. We do this in
two steps. First we add edges between all the vertices in one SCC to
all the vertices in another SCC when the two SCCs are adjacent in the
condensation graph. Second we add edges to connect each vertex in a
SCC to every other vertex in the SCC (other than itself).

@d Build transitive closure of the original graph
@{
// Add vertices to the transitive closure graph
typedef typename graph_traits<GraphTC>::vertex_descriptor tc_vertex;
std::vector<tc_vertex> to_tc_vec(num_vertices(g));
iterator_property_map<tc_vertex*, VertexIndexMap> 
  to_tc(&to_tc_vec[0], index_map);

{
  vertex_iterator i, i_end;
  for (tie(i, i_end) = vertices(g); i != i_end; ++i)
    to_tc[*i] = add_vertex(tc);
}
// Add edges between all the vertices in two adjacent SCCs
graph_traits<CG_t>::vertex_iterator si, si_end;
for (tie(si, si_end) = vertices(CG); si != si_end; ++si) {
  cg_vertex s = *si;
  graph_traits<CG_t>::adjacency_iterator i, i_end;
  for (tie(i, i_end) = adjacent_vertices(s, CG); i != i_end; ++i) {
    cg_vertex t = *i;
    for (size_type k = 0; k < components[s].size(); ++k)
      for (size_type l = 0; l < components[t].size(); ++l)
        add_edge(to_tc[components[s][k]], to_tc[components[t][l]], tc);
  }
}
// Add edges connecting all vertices in a SCC
for (size_type i = 0; i < components.size(); ++i)
  if (components[i].size() > 1)
    for (size_type k = 0; k < components[i].size(); ++k)
      for (size_type l = 0; l < components[i].size(); ++l) {
        vertex u = components[i][k], v = components[i][l];
        if (u != v)
          add_edge(to_tc[u], to_tc[v], tc);
      }
@}


@c transitive_closure.hpp
@{
// Copyright (C) 2001 Vladimir Prus <ghost@@cs.msu.su>
// Copyright (C) 2001 Jeremy Siek <jsiek@@cs.indiana.edu>
// Permission to copy, use, modify, sell and distribute this software is
// granted, provided this copyright notice appears in all copies and 
// modified version are clearly marked as such. This software is provided
// "as is" without express or implied warranty, and with no claim as to its
// suitability for any purpose.

#ifndef BOOST_GRAPH_TRANSITIVE_CLOSURE_HPP
#define BOOST_GRAPH_TRANSITIVE_CLOSURE_HPP

#include <vector>
#include <functional>
#include <boost/compose.hpp>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

namespace boost {
  @<Union of successor sets@>
  @<Subscript function object@>
  @<Transitive Closure Function@>
} // namespace boost

#endif // BOOST_GRAPH_TRANSITIVE_CLOSURE_HPP
@}

\bibliographystyle{abbrv}
\bibliography{jtran,ggcl,optimization,generic-programming,cad}

\end{document}
% LocalWords:  Siek Prus Succ typename GraphTC VertexIndexMap const tc typedefs
% LocalWords:  typedef iterator adjacency SCC num scc CG cg resize SCCs di ch
% LocalWords:  traversal ith namespace topo inserter  gx hy struct pos inf max
% LocalWords:  rbegin vec si hpp ifndef endif jtran ggcl

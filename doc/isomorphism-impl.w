\documentclass[11pt]{report}

%\input{defs}
\usepackage{math}
\usepackage{jweb}
\usepackage{lgrind}
%\usepackage{times}
\usepackage{fullpage}

\newcommand{\myhyperref}[2]{#2}
\newcommand{\code}[1]{{\small{\em \textbf{#1}}}}

\setlength\overfullrule{5pt}
\tolerance=10000
\sloppy
\hfuzz=10pt

\makeindex

\newcommand{\isomorphic}{\cong}

\begin{document}

\title{An Implementation of Isomorphism Testing}
\author{Jeremy G. Siek}

\maketitle

\section{Introduction}

This paper documents the implementation of the
\code{simple\-\_isomorphism()} function of the Boost Graph Library.
This function answers the question, ``are these two graphs equal?''
By \emph{equal}, we mean the two graphs have the same structure---the
vertices and edges are connected in the same way. The mathematical
name for this kind of equality is \emph{isomorphic}.

An \emph{isomorphism} is a one-to-one mapping of the vertices in one
graph to the vertices of another graph such that adjacency is
preserved. Another words, given graphs $G_{1} = (V_{1},E_{1})$ and
$G_{2} = (V_{2},E_{2})$, an isomorphism is a function $f$ such that
for all pairs of vertices $a,b$ in $V_{1}$, edge $(a,b)$ is in $E_{1}$
if and only if edge $(f(a),f(b))$ is in $E_{2}$.

Both graphs must be the same size, so let $N = |V_1| = |V_2|$. The
graph $G_1$ is \emph{isomorphic} to $G_2$ if an isomorphism exists
between the two graphs, which we denote by $G_1 \isomorphic G_2$.

In the following discussion we will need to use several notions from
graph theory. The graph $G_s=(V_s,E_s)$ is a \emph{subgraph} of graph
$G=(V,E)$ if $V_s \subseteq V$ and $E_s \subseteq E$.  An
\emph{induced subgraph}, denoted by $G[V_s]$, of a graph $G=(V,E)$
consists of the vertices in $V_s$, which is a subset of $V$, and every
edge $(u,v)$ in $E$ such that both $u$ and $v$ are in $V_s$.  We use
the notation $E[V_s]$ to mean the edges in $G[V_s]$.

In some places we express a function as a set of pairs, so the set $f
= \{ \pair{a_1}{b_1}, \pair{a_2}{b_2}, \ldots, \pair{a_n}{b_n} \}$
means $f(a_i) = b_i$ for $i=1,\ldots,n$.

\section{Exhaustive Backtracking Search}

The algorithm used by the \code{simple\_isomorphism()} function is, at
first approximation, an exhaustive search implemented via
backtracking.  The backtracking algorithm is a recursive function. At
each stage we will try to extend the match that we have found so far.
So suppose that we have already determined that some subgraph of $G_1$
is isomorphic to a subgraph of $G_2$.  We then try to add a vertex to
each subgraph such that the new subgraphs are still isomorphic to one
another. At some point we may hit a dead end---there are no vertices
that can be added to extend the isomorphic subgraphs. We then
backtrack to previous smaller matching subgraphs, and try extending
with a different vertex choice. The process ends by either finding a
complete mapping between $G_1$ and $G_2$ and return true, or by
exhausting all possibilities and returning false.

We are going to consider the vertices of $G_1$ in a specific order
(more about this later), so assume that the vertices of $G_1$ are
labeled $1,\ldots,N$ according to the order that we plan to add them
to the subgraph.  Let $G_1[k]$ denote the subgraph of $G_1$ induced by
the first $k$ vertices, with $G_1[0]$ being an empty graph. At each
stage of the recursion we start with an isomorphism $f_k$ between
$G_1[k]$ and a subgraph of $G_2$, which we denote by $G_2[S]$, so
$G_1[k] \isomorphic G_2[S]$. The vertex set $S$ is the subset of $V_2$
that corresponds via $f_k$ to the first $k$ vertices in $G_1$. We try
to extend the isomorphism by finding a vertex $v \in V_2 - S$ that
matches with vertex $k+1$. If a matching vertex is found, we have a
new isomorphism $f_{k+1}$ with $G_1[k+1] \isomorphic G_2[S \union \{ v
\}]$.

\begin{tabbing}
IS\=O\=M\=O\=RPH($k$, $S$, $f_k$) $\equiv$ \\
\>\textbf{if} ($S = V_2$) \\
\>\>\textbf{return} true \\
\>\textbf{for} each vertex $v \in V_2 - S$ \\
\>\>\textbf{if} (MATCH($k+1$, $v$)) \\
\>\>\>$f_{k+1} = f_k \union \pair{k+1}{v}$ \\
\>\>\>ISOMORPH($k+1$, $S \union \{ v \}$, $f_{k+1}$)\\
\>\>\textbf{else}\\
\>\>\>\textbf{return} false \\
\end{tabbing}

%ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)

The basic idea of the match operation is to check whether $G_1[k+1]$
is isomorphic to $G_2[S \union \{ v \}]$. We already know that $G_1[k]
\isomorphic G_2[S]$ with the mapping $f_k$, so all we need to do is
verify that the edges in $E_1[k+1] - E_1[k]$ connect vertices that
correspond to the vertices connected by the edges in $E_2[S \union \{
v \}] - E_2[S]$. The edges in $E_1[k+1] - E_1[k]$ of course are all
the out-edges and in-edges of $k+1$, and $E_2[S \union \{ v \}] -
E_2[S]$ consists of all the out-edges and in-edges of $v$.  We denote
the out-edges of a given vertex $u$ by $Out[u]$ and the in-edges of a
vertex $u$ by $In[u]$. Then we state the MATCH operation more
precisely as follows.

\begin{tabbing}
M\=ATCH($i$, $u$) $\equiv$ \\
\>$out \leftarrow \forall (i,j) \in Out[i] \Big( (u,f(j)) \in Out[u] \Big)$ \\
\>$in \leftarrow \forall (j,i) \in In[i] \Big( (f(j),u) \in In[u]) \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}



% \begin{tabbing}
% MA\=TC\=H(\=$i$\=, $v$) $\equiv$ \\
% \>\textbf{for} each edge $(i, j) \in E_1[i] - E_1[k]$ \\
% \>\>\textbf{if} ($k=i$) \\
% \>\>\>\textbf{if} ( $(v,f(j)) \notin E_2[S \union \{ v \}] - E_2[S]$ ) \\
% \>\>\>\>\textbf{return} false\\
% \>\>\textbf{else if} ($k=j$) \\
% \>\>\>\textbf{if} ( $(f(i),v) \notin E_2[S \union \{ v \}] - E_2[S]$ ) \\
% \>\>\>\>\textbf{return} false\\
% \end{tabbing}


The problem with the exhaustive backtracking algorithm is that there
are $N!$ possible vertex mappings, and $N!$ gets very large as $N$
increases, so we need to prune the search space. We use the pruning
techniques described in
\cite{fortin96:_isomorph,reingold77:_combin_algo} that originated in
\cite{sussenguth65:_isomorphism,unger64:_isomorphism}.

\section{Vertex Invariants}

One way to reduce the search space is through the use of \emph{vertex
invariants}. The idea is to compute a number for each vertex $invar(v)$
such that $invar(v) = invar(v')$ if there exists some isomorphism $f$ where
$f(v) = v'$. Then when we look for a match to some vertex $v$, we only
need to consider those vertices that have the same vertex invariant
number. The number of vertices in a graph with the same vertex
invariant number $i$ is called the \emph{invariant multiplicity} for
$i$.  In this implementation, by default we use the out-degree of the
vertex as the vertex invariant, though the user can also supply there
own invariant function. The ability of the invariant function to prune
the search space varies widely with the type of graph.

As a first check to rule out graphs that have no possibility of
matching, one can create a list of computed vertex invariant numbers
for the vertices in each graph, sort the two lists, and then compare
them.  If the two lists are different then the two graphs are not
isomorphic.  If the two lists are the same then the two graphs may be
isomorphic.

Also, we extend the MATCH operation to use the vertex invariants to
help rule out vertices.

\begin{tabbing}
M\=A\=T\=C\=H-INVAR($i$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (i,j) \in Out[i] \Big( (u,f(j)) \in Out[u] \Land invar(j) = invar(f(j)) \Big)$ \\
\>$in \leftarrow \forall (j,i) \in In[i] \Big( (f(j),u) \in In[u]) \Land invar(j) = invar(f(j)) \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}

\section{Vertex Order}

A good choice of the labeling for the vertices (which determines the
order in which the subgraph $G_1[k]$ is grown) can also reduce the
search space. In the following we discuss two labeling heuristics.

\subsection{Most Constrained First}

Consider the most constrained vertices first.  That is, examine
lower-degree vertices before higher-degree vertices. This reduces the
search space because it chops off a trunk before the trunk has a
chance to blossom out. We can generalize this to use vertex
invariants. We examine vertices with low invariant multiplicity
before examining vertices with high invariant multiplicity.

\subsection{Adjacent First}

The MATCH operation only considers edges when the other vertex already
has a mapping defined. This means that the MATCH operation can only
weed out vertices that are adjacent to vertices that have already been
matched. Therefore, when choosing the next vertex to examine, it is
desirable to choose one that is adjacent a vertex already in $S_1$.

\subsection{DFS Order, Starting with Lowest Multiplicity}

For this implementation, we combine the above two heuristics in the
following way. To implement the ``adjacent first'' heuristic we apply
DFS to the graph, and use the DFS discovery order as our vertex
order. To comply with the ``most constrained first'' heuristic we
start the DFS at the vertex with the lowest invariant multiplicity,
and each time we restart the DFS, we choose the undiscovered vertex
with lowest invariant multiplicity.

\section{Implementation}


@d Isomorphism Function Interface
@{
template <typename Graph1, typename Graph2, 
          typename IndexMapping, typename VertexInvariant,
          typename VertexIndexMap1, typename VertexIndexMap2>
bool simple_isomorphism(const Graph1& g1, const Graph2& g2,
    IndexMapping f, VertexInvariant invariant,
    VertexIndexMap1 v1_index_map, VertexIndexMap2 v2_index_map)
@}


@d Simple Isomorphism Function Outline
@{
{
  @<Concept checking@>
  @<Some type definitions and iterator declarations@>
  @<Quick return with false if $|V_1| \neq |V_2|$@>
  @<Compute vertex invariants@>
  @<Quick return if the graph's invariants do not match@>
  @<Compute invariant multiplicity@>
  @<Sort vertices by invariant multiplicity@>
  @<Order the vertices by DFS discover time@>
  @<Order the edge set by DFS discover time@>
  @<Perform backtracking searches, trying each vertex in $G_2$ as the start@>
}
@}


@d Concept checking
@{

@}

@d Some type definitions and iterator declarations
@{
typedef typename graph_traits<Graph2>::vertex_descriptor VertexG2;
typedef typename property_traits<VertexIndexMap2>::value_type V2Idx;
typedef typename graph_traits<Graph1>::vertices_size_type size_type;
typedef typename VertexInvariant::template result<Graph1>::type InvarValue1;
typedef typename VertexInvariant::template result<Graph2>::type InvarValue2;

typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
typename graph_traits<Graph2>::vertex_iterator i2, i2_end;
@}




@d Degree vertex invariant
@{
struct degree_vertex_invariant {
  template <typename Graph> struct result {
    typedef typename graph_traits<Graph>::degree_size_type type;
  };
  template <typename Graph>
  typename graph_traits<Graph>::degree_size_type
  operator()(typename graph_traits<Graph>::vertex_descriptor v, const Graph& g)
  {
    return out_degree(v, g);
  }
};
@}



@d Quick return with false if $|V_1| \neq |V_2|$
@{
if (num_vertices(g1) != num_vertices(g2))
  return false;
@}



@d Compute vertex invariants
@{
@<Setup storage for vertex invariants@>
for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
  invar1[*i1] = invariant(*i1, g1);
for (tie(i2, i2_end) = vertices(g2); i2 != i2_end; ++i2)
  invar2[*i2] = invariant(*i2, g2);
@}


@d Setup storage for vertex invariants
@{
typedef std::vector<InvarValue1> invar_vec1_t;
typedef std::vector<InvarValue2> invar_vec2_t;
typedef typename invar_vec1_t::iterator vec1_iter;
typedef typename invar_vec2_t::iterator vec2_iter;
invar_vec1_t invar1_vec(num_vertices(g1));
invar_vec2_t invar2_vec(num_vertices(g2));

// Provide Property Map interface for invariants
iterator_property_map<vec1_iter, V1Map, InvarValue1, InvarValue1&>
  invar1(invar1_vec.begin(), v1_index_map);
iterator_property_map<vec2_iter, V2Map, InvarValue2, InvarValue2&>
  invar2(invar2_vec.begin(), v2_index_map);
@}


@d Quick return if the graph's invariants do not match
@{
{
  invar_vec1_t invar1_tmp(invar1_vec);
  invar_vec2_t invar2_tmp(invar2_vec);
  std::sort(invar1_tmp.begin(), invar1_tmp.end());
  std::sort(invar2_tmp.begin(), invar2_tmp.end());
  if (! std::equal(invar1_tmp.begin(), invar1_tmp.end(), invar2_tmp.begin()))
    return false;
}
@}


@d Compute invariant multiplicity
@{
std::vector<std::size_t> invar_mult(num_vertices(g1), 0);
for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)      
  ++invar_mult[invar1[*i1]];
@}

@d Sort vertices by invariant multiplicity
@{
std::vector<size_type> perm;
integer_range<size_type> range(0, num_vertices(g1));
std::copy(range.begin(), range.end(), std::back_inserter(perm));
std::sort(perm.begin(), perm.end(),
          detail::compare_invariant_multiplicity(invar1_vec.begin(),
                                                 invar_mult.begin()));

typedef typename graph_traits<Graph1>::vertex_descriptor VertexG1;
std::vector<VertexG1> g1_vertices;
for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
  g1_vertices.push_back(*i1);
permute(g1_vertices.begin(), g1_vertices.end(), perm.begin());
@}


@d Order the vertices by DFS discover time
@{
{
  perm.clear();
  @<Compute DFS discover times@>
  g1_vertices.clear();
  for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
    g1_vertices.push_back(*i1);
  permute(g1_vertices.begin(), g1_vertices.end(), perm.begin());
}
@}


@d Compute DFS discover times
@{
std::vector<default_color_type> color_vec(num_vertices(g1));
for (typename std::vector<VertexG1>::iterator ui = g1_vertices.begin();
     ui != g1_vertices.end(); ++ui) {
  if (color_vec[get(v1_index_map, *ui)] 
      == color_traits<default_color_type>::white()) {
    depth_first_visit
      (g1, *ui, detail::record_dfs_order<Graph1, V1Map>(perm, 
						       v1_index_map), 
       make_iterator_property_map(&color_vec[0], v1_index_map, 
				  color_vec[0]));
  }
}
@}


@d Order the edge set by DFS discover time
@{
typedef typename graph_traits<Graph1>::edge_descriptor edge1_t;
std::vector<edge1_t> edge_set;
std::copy(edges(g1).first, edges(g1).second, 
          std::back_inserter(edge_set));

std::sort(edge_set.begin(), edge_set.end(), 
          detail::isomorph_edge_ordering
          (make_iterator_property_map(perm.begin(), v1_index_map, 
                                      perm[0]), g1));
@}



@d Perform backtracking searches, trying each vertex in $G_2$ as the start
@{
typename graph_traits<Graph2>::vertex_iterator vi, vi_end;
for (tie(vi, vi_end) = vertices(g2); vi != vi_end; ++vi) {
  f[*first] = *vi; // S = { *vi }
  @<Construct $V_2 - S$, calling it \code{not\_in\_S}@>
  @<Attempt to extend $S$ to the whole of $V_2$@>
}
@}


@d Construct $V_2 - S$, calling it \code{not\_in\_S}
@{
typedef indirect_cmp<V2Map, std::less<V2Idx> >  Cmp;
Cmp cmp(v2_index_map);
std::set<VertexG2, Cmp> not_in_S(cmp);
for (tie(i2, i2_end) = vertices(g2); i2 != i2_end; ++i2)
  set_insert(not_in_S, *i2);
set_remove(not_in_S, *vi);
@}



@d Attempt to extend $S$ to the whole of $V_2$
@{
if(detail::isomorph(boost::next(first), g1_vertices.end(), 
    edge_set.begin(), edge_set.end(), g1, g2,
    make_iterator_property_map(perm.begin(), v1_index_map, perm[0]),
    v2_index_map, f, invar1, invar2, not_in_S))
  return true;
@}


@d Signature for the recursive isomorph function
@{
template <class VertexIter, class EdgeIter, class Graph1, class Graph2,
  class V1IndexMap, class V2IndexMap, class IndexMapping, 
  class Invar1, class Invar2, class Set>
bool isomorph(VertexIter k, VertexIter last,
	      EdgeIter edge_iter, EdgeIter edge_iter_end,
	      const Graph1& g1, const Graph2& g2,
	      V1IndexMap v1_index_map,
	      V2IndexMap v2_index_map,
	      IndexMapping f, Invar1 invar1, Invar2 invar2,
	      const Set& not_in_S)
@}



@d Outline for the isomorph function
@{
@<Return true if matching is complete@>
@<Create a local copy of the mapping $f_k$@>
@<Find potential matches for $k$ in $V_2 - S$@>
@}

@d Return true if matching is complete
@{
if (k == last) 
  return true;
@}

@d Create a local copy of the mapping $f_k$
@{
typedef typename graph_traits<Graph2>::vertex_descriptor v2_desc_t;
std::vector<v2_desc_t> my_f_vec(num_vertices(g1));
typedef typename std::vector<v2_desc_t>::iterator vec_iter;
iterator_property_map<vec_iter,  V1IndexMap, v2_desc_t, v2_desc_t&>
  my_f(my_f_vec.begin(), v1_index_map);

typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
  my_f[*i1] = f[*i1];
@}

In the psuedo-code for ISOMORPH, we iterate through each vertex in $v
\in V_2 - S$ and check if $k+1$ and $v$ can match.  A more efficient
approach is to directly iterate through all the potential matches for
$k+1$. Let $M$ be the set of vertices that can be potentially matched
to $k+1$. Then we can define $M$ as follows:

\begin{align*}
M &= \{ v \st out \lor in \} \\
out &= (k+1,j) \in E_1[k+1] \land (v,f(j)) \in E_2[V_2 - S] \\
in &= (j,k) \in E_1[k+1] \land (f(j),v) \in E_2[V_2 - S])
\end{align*}




%(k,j)
%(j,k)
%(v,f(j))
%(f(j), v)

Let $M$ be the set of vertices that have the same connectivity
structure as $k$ with respect to the vertices that have already been
mapped, and which have the same vertex invariant. It is the
intersection of the vertices in $C$ with the vertices in $V_2 - S_2$
that are potential matches for $k$, which we denote $P$.

introduce notion of partial isomorphism?

@d foo
@{
for (tie(kj, kj_end) = out_edges(k, g1); kj != kj_end; ++kj) {
  j = target(*kj, g1);
  vertex2_t v = get(f, j);
  for (tie(uv, uv_end) = in_edges(v, g2); uv != uv_end; ++uv) {
    u = source(*uv, g2);
    if (get(not_in_S, u) && (get(invar1, j) == get(invar2, v))) {
      isomorph();
    }
  }
}
@}

@d bar
@{
for (tie(jk, jk_end) = in_edges(k, g1); jk != jk_end; ++jk) {
  j = source(*jk, g1);
  vertex2_t v = get(f, j);
  for (tie(vu, vu_end) = out_edges(v, g2); vu != vu_end; ++vu) {
    u = target(*vu, g2);
    if (get(not_in_S, u) && (get(invar1, j) == get(invar2, u))) {
      isomorph();
    }
  }
}
@}





@d Let $P = V_2 - S$
@{
std::vector<v2_desc_t> vertex_set;
std::copy(not_in_S.begin(), not_in_S.end(), std::back_inserter(vertex_set));
@}

@d Construct $C$, the vertices that are partially isomorphic to $k$
@{

@}

@d Perform $P = P \intersect C$
@{

@}

Let $P_2$ be the vertices from $V_2 - S$ that have the same
connectivity with respect to vertex $k$ and the same vertex invariant
as $k$.


@d MATCH-INVAR implementation
@{
@}

% P_2 = V_2 - S_2

% (k, v_1)
% v_2 = f(v_1)
% A = {}
% for all (u,v_2) \in E_2
%   if (i(k) = i(u))
%     A = A U { u }

% P_2 <- P_2 \intersect A

% same connectivity and same vertex invariant


\end{document}
% LocalWords:  Isomorphism Siek isomorphism adjacency subgraph subgraphs OM DFS
% LocalWords:  ISOMORPH Invariants invariants typename IndexMapping bool const
% LocalWords:  VertexInvariant VertexIndexMap iterator typedef VertexG Idx num
% LocalWords:  InvarValue struct invar vec iter tmp mult inserter permute ui
% LocalWords:  dfs cmp isomorph VertexIter EdgeIter IndexMap desc

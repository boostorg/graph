\documentclass[11pt]{report}

%\input{defs}
\usepackage{math}
\usepackage{jweb}
\usepackage{lgrind}
%\usepackage{times}
\usepackage{fullpage}
\usepackage{graphicx}

\newcommand{\myhyperref}[2]{#2}
\newcommand{\code}[1]{{\small{\em \textbf{#1}}}}

%\newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.pdf}}\caption{#2}\label{fig:#1}\end{figure}}
\newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.eps}}\caption{#2}\label{fig:#1}\end{figure}}

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
stage of the recursion we start with an isomorphism $f_{k-1}$ between
$G_1[k-1]$ and a subgraph of $G_2$, which we denote by $G_2[S]$, so
$G_1[k-1] \isomorphic G_2[S]$. The vertex set $S$ is the subset of
$V_2$ that corresponds via $f_{k-1}$ to the first $k-1$ vertices in
$G_1$. We try to extend the isomorphism by finding a vertex $v \in V_2
- S$ that matches with vertex $k$. If a matching vertex is found, we
have a new isomorphism $f_k$ with $G_1[k] \isomorphic G_2[S \union \{
v \}]$.

\begin{tabbing}
IS\=O\=M\=O\=RPH($k$, $S$, $f_{k-1}$) $\equiv$ \\
\>\textbf{if} ($k = |V_1|+1$) \\
\>\>\textbf{return} true \\
\>\textbf{for} each vertex $v \in V_2 - S$ \\
\>\>\textbf{if} (MATCH($k$, $v$)) \\
\>\>\>$f_k = f_{k-1} \union \pair{k}{v}$ \\
\>\>\>ISOMORPH($k+1$, $S \union \{ v \}$, $f_k$)\\
\>\>\textbf{else}\\
\>\>\>\textbf{return} false \\
\end{tabbing}

%ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)

The basic idea of the match operation is to check whether $G_1[k]$ is
isomorphic to $G_2[S \union \{ v \}]$. We already know that $G_1[k-1]
\isomorphic G_2[S]$ with the mapping $f_{k-1}$, so all we need to do
is verify that the edges in $E_1[k] - E_1[k-1]$ connect vertices that
correspond to the vertices connected by the edges in $E_2[S \union \{
v \}] - E_2[S]$. The edges in $E_1[k] - E_1[k-1]$ of course are all
the out-edges and in-edges of $k$, and $E_2[S \union \{ v \}] -
E_2[S]$ consists of all the out-edges and in-edges of $v$.  We denote
the out-edges of a given vertex $u$ by $Out[u]$ and the in-edges of a
vertex $u$ by $In[u]$. Then we state the MATCH operation more
precisely as follows.

\begin{tabbing}
M\=ATCH($k$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (k,j) \in Out[k] \Big( (v,f(j)) \in Out[v] \Big)$ \\
\>$in \leftarrow \forall (j,k) \in In[k] \Big( (f(j),v) \in In[v]) \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}

% For MATCH, do we have check going from G_2 to G_1 as well?

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
invariants}. The idea is to compute a number for each vertex $i(v)$
such that $i(v) = i(v')$ if there exists some isomorphism $f$ where
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
M\=A\=T\=C\=H-INVAR($k$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (k,j) \in Out[k] \Big( (v,f(j)) \in Out[v] \Land i(j) = i(f(j)) \Big)$ \\
\>$in \leftarrow \forall (j,k) \in In[k] \Big( (f(j),v) \in In[v]) \Land i(j) = i(f(j)) \Big)$ \\
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
    VertexIndexMap1 index_map1, VertexIndexMap2 index_map2)
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
  @<Invoke recursive \code{isomorph} function@>
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


@d Quick return with false if $|V_1| \neq |V_2|$
@{
if (num_vertices(g1) != num_vertices(g2))
  return false;
@}


\subsection{Ordering by Vertex Invariant Multiplicity}


@d Degree vertex invariant
@{
struct degree_vertex_invariant {
  template <typename Graph> struct result {
    typedef typename graph_traits<Graph>::degree_size_type type;
  };
  template <typename Graph>
  typename graph_traits<Graph>::degree_size_type
  operator()(typename graph_traits<Graph>::vertex_descriptor v, const Graph& g)
    { return out_degree(v, g); }
};
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
  invar1(invar1_vec.begin(), index_map1);
iterator_property_map<vec2_iter, V2Map, InvarValue2, InvarValue2&>
  invar2(invar2_vec.begin(), index_map2);
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

\subsection{Ordering by DFS Discover Time}


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
  if (color_vec[get(index_map1, *ui)] 
      == color_traits<default_color_type>::white()) {
    depth_first_visit
      (g1, *ui, detail::record_dfs_order<Graph1, V1Map>(perm, index_map1), 
       make_iterator_property_map(&color_vec[0], index_map1, color_vec[0]));
  }
}
@}

The set $V_2 - S$ is represented with a bitset named
\code{not\_in\_S}. 



@d Invoke recursive \code{isomorph} function
@{
std::vector<char> not_in_S(num_vertices(g2), true);
return detail::isomorph(g1_vertices.begin(), g1_vertices.end(), g1, g2,
    make_iterator_property_map(perm.begin(), index_map1, perm[0]),
    index_map2, f, invar1, invar2, not_in_S));
@}


\subsection{Implementation of ISOMORPH}


@d Signature for the recursive isomorph function
@{
template <typename VertexIter, typename Graph1, typename Graph2,
    typename IndexMap2, typename IndexMap2, typename IndexMapping, 
    typename Invar1, typename Invar2, typename NotInSMap>
bool isomorph(VertexIter k, VertexIter last,
              const Graph1& g1, const Graph2& g2,
              IndexMap2 index_map2,
              IndexMap2 index_map2,
              IndexMapping f, 
              Invar1 invar1, Invar2 invar2,
              const NotInSMap& not_in_S)
@}



@d Outline for the isomorph function
@{
@<Return true if matching is complete@>
@<Compute $M$, the potential matches for $k$@>
@<Create a local copy of the mapping $f_k$@>
@<Invoke isomorph for each vertex in $M$@>
@}

@d Return true if matching is complete
@{
if (k == last) 
  return true;
@}


In the psuedo-code for ISOMORPH, we iterate through each vertex in $v
\in V_2 - S$ and check if $k$ and $v$ can match.  A more efficient
approach is to directly iterate through all the potential matches for
$k$, for this often is many fewer vertices than $V_2 - S$.  Let $M$
denote the set of vertices that can be potentially matched to $k$. We
define $M$ as follows:
%
\begin{align*}
M &= out \intersect in \\ 
out &= \Big\{ v \st \forall (k,j) \in Out[k] \Big( (v,f(j)) \in In[f(j)] \Land i(j) = i(f(j)) \Land v \in V_2 - S \Big) \Big\} \\
in &= \Big\{ v \st \forall (j,k) \in In[k] \Big( (f(j),v) \in Out[f(j)] \Land i(j) = i(f(j)) \Land v \in V_2 - S \Big) \Big\}
\end{align*}


@d Compute $M$, the potential matches for $k$
@{
 std::vector<vertex2_t> M, out, in;
@<Compute the $out$ set@>
@<Compute the $in$ set@>
 std::set_intersection(out.begin(), out.end(), in.begin(), in.end(),
     std::back_inserter(M), cmp);
@}

To compute the $out$ set, we iterate through the out-edges $(k,j)$ of
$k$, and for each $j$ we iterate through the in-edges $(v,f(j))$ of
$f(j)$, putting all of the $v$'s in $out$ that have the same vertex
invariant as $k$, and which are in $V_2 - S$. Figure~\ref{fig:out}
depicts the computation of the $out$ set. The implementation is as
follows.

@d Compute the $out$ set
@{
 for (tie(k_j, k_j_end) = out_edges(k, g1); k_j != k_j_end; ++k_j) {
   vertex1_t j = target(*k_j, g1);
   for (tie(v_fj, v_fj_end) = in_edges(get(f, j), g2); v_fj != v_fj_end; ++v_fj) {
     vertex2_t v = source(*v_fj, g2);
     if ((get(invar1, j) == get(invar2, get(f, j))) && not_in_S[get(index_map2, v)])
       out.push_back(v);
   }
 }
@}


\vizfig{out}{Computing the $out$ set.}

@c out.dot
@{
digraph G {
  node[shape=circle]
  size="4,2"
  ratio="fill"

  subgraph cluster0 { label="G_1"
    k -> j_1 
    k -> j_2 
    k -> j_3
  }

  subgraph cluster1 { label="G_2"

    subgraph cluster2 { label="out" v_1 v_2 v_3 v_4 v_5 v_6 }

    v_1 -> fj_1
    v_2 -> fj_1
    v_3 -> fj_1

    v_4 -> fj_2

    v_5 -> fj_3
    v_6 -> fj_3

    fj_1[label="f(j_1)"]
    fj_2[label="f(j_2)"]
    fj_3[label="f(j_3)"]
  }

  j_1 -> fj_1[style=dotted]
  j_2 -> fj_2[style=dotted]
  j_3 -> fj_3[style=dotted]
}
@}

The $in$ set is is constructed by iterating through the in-edges
$(j,k)$ of $k$, and for each $j$ we iterate through the out-edges
$(f(j),v)$ of $f(j)$. We put all of the $v$'s in $in$ that have the
same vertex invariant as $k$, and which are in $V_2 -
S$. Figure~\ref{fig:in} depicts the computation of the $in$ set.  The
following code computes the $in$ set.

@d Compute the $in$ set
@{
 for (tie(j_k, j_k_end) = in_edges(k, g1); j_k != j_k_end; ++j_k) {
   j = source(*j_k, g1);
   for (tie(fj_v, fj_v_end) = out_edges(get(f, j), g2); fj_v != fj_v_end; ++fj_v) {
     v = target(*fj_v, g2);
     if ((get(invar1, j) == get(invar2, get(f, j))) && not_in_S[get(index_map2, v)])
       in.push_back(v);
   }
 }
@}

\vizfig{in}{Computing the $in$ set.}

@c in.dot
@{
digraph G {
  node[shape=circle]
  size="3,2"
  ratio="fill"
  subgraph cluster0 { label="G1"
    j_1 -> k
    j_2 -> k
  }

  subgraph cluster1 { label="G2"

    subgraph cluster2 { label="in" v_1 v_2 v_3 }

    v_1 -> fj_1
    v_2 -> fj_1

    v_3 -> fj_2

    fj_1[label="f(j_1)"]
    fj_2[label="f(j_2)"]
  }

  j_1 -> fj_1[style=dotted]
  j_2 -> fj_2[style=dotted]

}
@}

For each vertex $v$ in the potential matches $M$, we will create an
extended isomorphism $f_k = f_{k-1} \union \pair{k}{v}$. First
we create a local copy of $f_{k-1}$.

@d Create a copy of $f_{k-1}$ which will become $f_k$
@{
typedef typename graph_traits<Graph2>::vertex_descriptor v2_desc_t;
std::vector<vertex2_t> my_f_vec(num_vertices(g1));

// Construct property map interface to my_f_vec
typedef typename std::vector<v2_desc_t>::iterator vec_iter;
iterator_property_map<vec_iter,  IndexMap2, v2_desc_t, v2_desc_t&>
  my_f(my_f_vec.begin(), index_map2);

typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
  my_f[*i1] = f[*i1];
@}

Next we enter the loop through every vertex $v$ in $M$, and extend the
isomorphism with $\pair{k}{v}$. We then update the set $S$ (by
removing $v$ from $V_2 - S$) and make the recursive call to
\code{isomorph}. If \code{isomorph} returns successfully, we have
found an isomorphism for the complete graph, so we copy our local
mapping into the mapping from the previous calling function.

@d Invoke isomorph for each vertex in $M$
@{
for (iterator v_iter = M.begin(); v_iter != M.end(); ++v_iter) {
  vertex2_t v = *v_iter;
  my_f[k] = v;
  std::vector<char> my_not_in_S(not_in_S);
  my_not_in_S[get(index_map2, v)] = false;
  if (isomorph(next(k), last, g1, g2, index_map2, index_map2, my_f, invar1, invar2, my_not_in_S)) {
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
      f[*i1] = my_f[*i1];
    return true;
  }
}
@}


\end{document}
% LocalWords:  Isomorphism Siek isomorphism adjacency subgraph subgraphs OM DFS
% LocalWords:  ISOMORPH Invariants invariants typename IndexMapping bool const
% LocalWords:  VertexInvariant VertexIndexMap iterator typedef VertexG Idx num
% LocalWords:  InvarValue struct invar vec iter tmp mult inserter permute ui
% LocalWords:  dfs cmp isomorph VertexIter EdgeIter IndexMap desc

\documentclass[11pt]{report}


\usepackage[leqno]{amsmath}
\usepackage{amsfonts}
\usepackage{amssymb}
\usepackage{amsthm}
\usepackage{latexsym}     

\newcommand{\path}{\overset{G}{\rightsquigarrow}}
\newcommand{\ancestor}{\overset{T}{\rightsquigarrow}}
\newcommand{\descendant}{\ancestor^{-1}}
\newcommand{\backedge}{\overset{B}{\rightarrow}}
\newcommand{\edge}{\rightarrow}
\DeclareMathOperator{\suchthat}{s.t.}
\begin{document}

\section{Facts about DFS on an connected undirected graph}

Let $T$ be the DFS-tree formed by applying DFS to a connected
undirected graph $G = (V,E)$. Let $B$ be the set of back edges that
connect a vertex in $T$ to one of its ancestors in $T$.  Let $d[u]$ be
the discover time of $u$ in the DFS.  Let $T_u$ denote the subtree of
$T$ rooted at $u$. Let $\path$ denote a path in $G$ and $\ancestor$ a
path in $T$. If $\exists v \ancestor w$ then $v$ is an ancestor of
$w$, and $w$ is a descendant of $v$. The subtree $T_u$ contains all
the descendants of $u$.

\begin{eqnarray}
T \cup B = E \\
(u,v) \in T \implies d[u] < d[v] \\
(u,v) \in B \implies d[u] > d[v] \\
v \in T_u \implies d[u] < d[v] \\
v \notin T_u \implies d[v] < d[u] \\
\exists a \in v \path w \suchthat \exists a \ancestor v \land \exists a \ancestor w
\end{eqnarray}

There are three cases for the relationship between two vertices
$u$ and $v$ in $G$.
\begin{enumerate}
\item $v \ancestor w$
\item $w \ancestor v$
\item Neither $v \ancestor w$ or $w \ancestor v$, and the
only paths $v \path w$ go through some ancestor $a$ of both $v$ and $w$.
If there was a path $v \path w$ that did not go through a
common ancestor of $v$ and $w$, then either $v \ancestor w$ or
$w \ancestor v$ would have to be true.
\end{enumerate}


\paragraph{Definition.} An undirected graph $G = (V,E)$ is \emph{biconnected}
if for each triple of distinct vertices $v, w, a \in V$ there is a
path $p : v \path w$ such that $a$ is not on the path $p$.

\paragraph{Definition.} An \emph{articulation point} of $G = (V,E)$
is a vertex $a \in V$ where there are two other distinct vertices $v,w
\in V$ such that $a$ is on any path $p:v \path w$ and there is at
least one such path.

\paragraph{Lemma.} Let $G = (V,E)$ be an undirected graph. We define
an equivalence relation on the set of edges as follows: two edges are
equivalent iff they belong to a common cycle. Let the distinct
equivalence classes under this relation be $\mathcal{E}_i, 1 \leq i
\leq n$ and let $G_i = (V_i, E_i)$ where $V_i$ is the set of vertices
incident to the edges of $\mathcal{E}_i$; $V_i = \{v|\exists w ((v,w)
\in \mathcal{E}_i)\}$.  Then
\begin{enumerate}
\item $G_i$ is biconnected, for each $1 \leq i \leq n$.
\item No $G_i$ is a proper subgraph of a biconnected subgraph of $G$.
\item Each articulation point of $G$ occurs more than once among the $V_i$.
\item Each nonarticulation point of $G$ occurs exactly once among the $V_i$.
item The set $V_i \cap V_j$ contains at most one point, for any $1 \leq i, j \leq n$. Such a point of intersection is an articulation point of the graph.
\end{enumerate}
The subgraphs $G_i$ are called the \emph{biconnected components} of $G$.

If $u$ is an ancestor or $v$ in the spanning tree $T$ of $P$, then
$d[u] < d[v]$. 

\paragraph{Definition.} $\lambda(v)$ is the vertex with the smallest
DFS number in the set $\{v\} \cup \{w|v \ancestor \backedge w\}$. That
is, $\lambda(v)$ is the smallest vertex reachable from $v$ by
traversing zero or more DFS-tree edges followed by at most one back
edge.


\paragraph{Lemma.}
If $(a, v) \in T$ and $d[\lambda(v)] \geq d[a]$ we know that any path
from $v$ not passing through $a$ remains in the subtree $T_v$.

\paragraph{Proof by contradiction.} Assume $\exists w \notin
T_v$ and there is a path $p: v \ancestor\backedge w$ that does not
include $a$. Since $w \notin T_v$ and $\exists p$ we know that
$v \in T_w$.  We also know that $d[w] < d[a]$ because otherwise
$(a,v)$ would not be in $T$, some other edge on $p$ would be the tree
edge for $v$.  Now since $d[\lambda(v)] \leq d[w]$ this means
$d[\lambda(v)] < d[a]$ which is a contradiction.


\paragraph{Lemma.} Suppose $a,v,w$ are distinct vertices in $G$ such that
$(a,v) \in T$ and $\not\exists v \ancestor w$.  If $d[\lambda(v)] \geq
d[a]$, then $a$ is an articulation point of $P$ and removal of $a$
disconnects $v$ and $w$. Conversely, if $a$ is an articulation point
of $G$ then $\exists a,v,w \in G$ such that $(a,v) \in T$,
$\not\exists v \ancestor w$, and $d[\lambda(v)] \geq d[a]$.

\paragraph{Proof.} Since $d[\lambda(v)] \geq d[a]$ we know by
the previous lemma that all paths from $v$ either stay in $T_v$ or go
through $a$. Therefore $a$ must be an articulation point.

Now to prove the converse. Let $a$ be an articulation point of $G$ and
show that $\not\exists v \ancestor w$ and $d[\lambda(v)] \geq d[a]$.
\begin{enumerate}
\item If $a$ is the root of $T$ then then there must be at least two
out-edges of $a$, let them be $(a,v)$ and $(a,w)$. Then $(a,v) \in T$
and $d[\lambda(v)] \geq d[a]$ and $\not\exists v \ancestor w$ .

\item If $a$ is not the root of $T$, consider the connected components
formed by deleting $a$ from $G$. One component is $T_a$. Let $v$ be a
child of $a$ and $w \ancestor a$. Then $(a,v) \in T$, $\lambda(v) \geq
a$, and $\not\exists v \ancestor w$.
\end{enumerate}


@d Recursive Biconnect Function
@{
template <typename Graph, typename DiscoverTimeMap, typename LowPointMap,
  typename Stack>
void biconnect(typename graph_traits<Graph>::vertex_descriptor v,
               typename graph_traits<Graph>::vertex_descriptor u,
               const Graph& g,
               ComponentMap comp,
               std::size_t& c,
               DiscoverTimeMap d,
	       std::size_t& dfs_time,
               LowPointMap lowpt,
               Stack& S)
{
  typedef typename property_traits<DiscoverTime>::value_type D;
  d[v] = ++dfs_time;
  lowpt[v] = d[v];
  for (tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
    vertex_t w = target(*ei, g);
    if (d[w] == std::numeric_limits<D>::max())
      S.push(*ei);
      biconnect(w, v);
      lowpt[v] = std::min(lowpt[v], lowpt[w]);
      if (d[v] < lowpt[w]) {
        ++c;
        while (d[w] < d[source(S.top(), g)]) {
          comp[source(S.top(), g)] = c;
          comp[target(S.top(), g)] = c;
          S.pop();
        }
        comp[source(S.top(), g)] = c;
        comp[target(S.top(), g)] = c;
        S.pop();
      }
    } else if (d[w] < d[v] && w != u) {
      S.push(*ei);
      lowpt[v] = std::min(lowpt[v], d[w]);
    }
  }
}
@}

@d Biconnected Components Algorithm
@{
namespace detail {
  @<Recursive Biconnect Function@>
}

template <typename Graph, typename DiscoverTimeMap, typename LowPointMap>
void biconnected_components(typename graph_traits<Graph>::vertex_descriptor v,
               typename graph_traits<Graph>::vertex_descriptor u,
               const Graph& g,
               ComponentMap comp,
               std::size_t& c,
               DiscoverTimeMap d,
               LowPointMap lowpt)
{
  typedef typename property_traits<DiscoverTime>::value_type D;
  typedef graph_traits<graph_t>::edge_descriptor edge_t;
  std::size_t dfs_time = 0;
  std::stack<edge_t> S;
  for (tie(wi, wi_end) = vertices(g); wi != wi_end; ++wi)
    if (d[*wi] == std::numeric_limits<D>::max())
      detail::biconnect(*wi, *vertices(g).first,
                        g, comp, c, d, dfs_time, lowpt, S);
}
@}


@o biconnected_components.hpp
@{
#ifndef BOOST_GRAPH_BICONNECTED_COMPONENTS_HPP
#define BOOST_GRAPH_BICONNECTED_COMPONENTS_HPP

#include <boost/graph/graph_traits.hpp>

namespace boost {
  @<Biconnected Components Algorithm@>
} // namespace boost

#endif BOOST_GRAPH_BICONNECTED_COMPONENTS_HPP
@}

@o biconnected_components.cpp
@{
#include <vector>
#include <list>
#include "biconnected_components.hpp"
#include <boost/graph/vertex_as_graph.hpp>

int main()
{
  using namespace boost;
  typedef adjacency_list<vecS, vecS, undirectedS> graph_t;
  typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
  graph_t g(9);
  add_edge(0, 5, g); add_edge(0, 1, g); add_edge(0, 6, g);
  add_edge(1, 2, g); add_edge(1, 3, g); add_edge(1, 4, g);
  add_edge(2, 3, g);
  add_edge(4, 5, g);
  add_edge(6, 8, g); add_edge(6, 7, g);
  add_edge(7, 8, g);

  std::vector<std::size_t> component(num_vertices(g));
  std::size_t c = 0;
  std::vector<std::size_t> discover_time(vertices(g),
    std::numeric_limits<std:size_t>::max()));
  std::vector<vertex_t> lowpt(vertices(g));
  biconnect(0, 8, g, &component[0], c, &discover_time[0],
            &lowpt[0], S);
  std::cout << "vertex -> biconnected component number" << std::endl;
  for (std::size_t i = 0; i < num_vertices(g); ++i)
    std::cout << i << ": " << component[i] << std::endl;

  return 0;
}
@}

% \paragraph{Definition.} A \emph{palm tree} $P$ is a directed graph that 
% consists of two disjoint sets of edges, denoted by $v \rightarrow w$
% and $v \backedge w$ respectively, with the following properties:

% \begin{enumerate}

% \item The subgraph $T$ containing the edges $v \rightarrow w$ is a
%   spanning tree of $P$.

% \item $\backedge \; \subseteq \descendant$. That is, each edge of $P$
% that is not in $T$ connects a vertex to one of its ancestors in $T$.
% \end{enumerate}


\end{document}


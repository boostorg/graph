\documentclass[11pt]{report}


\usepackage[leqno]{amsmath}
\usepackage{amsfonts}
\usepackage{amssymb}
\usepackage{amsthm}
\usepackage{latexsym}     

\newcommand{\path}{\rightsquigarrow}
\newcommand{\ancestor}{\overset{T}{\rightsquigarrow}}
\newcommand{\descendant}{\ancestor^{-1}}
\newcommand{\backedge}{\overset{B}{\rightarrow}}
\newcommand{\edge}{\rightarrow}
\DeclareMathOperator{\suchthat}{s.t.}

\begin{document}

\title{An Implementation of Biconnected Components}
\author{Jeremy Siek}

\maketitle

\section{Introduction}

This paper documents the implementation of the
\code{biconnected\_components()} function of the Boost Graph
Library. The function was implemented by Jeremy Siek.

The algorithm used to implement the \code{biconnected\_components()}
function is the one based on depth-first search described
by Tarjan~\cite{tarjan72:dfs_and_linear_algo}.

An undirected graph $G = (V,E)$ is \emph{biconnected} if for each
triple of distinct vertices $v, w, a \in V$ there is a path $p : v
\path w$ such that $a$ is not on the path $p$.  An \emph{articulation
point} of $G = (V,E)$ is a vertex $a \in V$ where there are two other
distinct vertices $v,w \in V$ such that $a$ is on any path $p:v \path
w$ and there is at least one such path. So articulation points act as
bridges between biconnected components; the only path from one
biconnected component to another is through an articulation point.

The algorithm finds articulation points in the graph based on
information provided by depth-first search. During a DFS, we label
each vertex $v \in G$ with its discover time, denoted $d[v]$.  During
the DFS we also compute the $lowpt(v)$, which is the smallest (in
terms of discover time) vertex reachable from $v$ by traversing zero
or more DFS-tree edges followed by at most one back edge. Tree edges
and back edges can be identified based on discover time because for
tree edge $(u,v)$ we have $d[u] < d[v]$ and for back edge $(u,v)$ we
have $d[u] > d[v]$. The $lowpt(v)$ is computed for $v$ by taking the
vertex with the minimum $lowpt(w)$ for all $(v,w) \in E$. The
$lowpt(v)$ is computed after the recursive call so $lowpt(w)$ for all
$(v,w \in E)$ has already been computed.

Now it turns out that $lowpt$ can be used to identify articulation
points. Suppose $a,v,w$ are distinct vertices in $G$ such that $(a,v)$
is a tree edge and $w$ is not a descendant of $v$. If $d[lowpt(v)]
\geq d[a]$, then $a$ is an articulation point and removal of $a$
disconnects $v$ and $w$. The reason this works is that if $d[lowpt(v)]
\geq d[a]$, then we know all paths starting from $v$ stay within the
subtree $T_v$ rooted at $v$. If there was a path that escaped from the
subtree, then the first vertex $w$ in that path outside of $T_v$ would
cause $d[lowpt(v)] < d[a]$, for $d[w] < d[a]$ due the structure of the
DFS.

\section{The Implementation}



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
      if (lowpt[w] >= d[v]) {
        @<Record the biconnected component@>
      }
    } else if (d[w] < d[v] && w != u) {
      S.push(*ei);
      lowpt[v] = std::min(lowpt[v], d[w]);
    }
  }
}
@}

@d Record the biconnected component
@{
++c;
while (d[source(S.top(), g)] >= d[w]) {
  comp[source(S.top(), g)] = c;
  comp[target(S.top(), g)] = c;
  S.pop();
}
comp[source(S.top(), g)] = c;
comp[target(S.top(), g)] = c;
S.pop();
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


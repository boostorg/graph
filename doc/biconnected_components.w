\documentclass[11pt]{report}

\begin{document}

\paragraph{Definition.} A \emph{palm tree} $P$ is a directed graph that 
consists of two disjoint sets of edges, denoted by $v \rightarrow w$
and $v \hookrightarrow w$ respectively, with the following properties:

\begin{enumerate}

\item The subgraph $T$ containing the edges $v \rightarrow w$ is a
  spanning tree of $P$.

\item $\hookrightarrow \; \subseteq (\stackrel{*}{\rightarrow})^{-1}$. That is, each edge of $P$ that is
not in $T$ connects a vertex to one of its ancestors in $T$.
\end{enumerate}


\paragraph{Facts about DFS}
Let $T$ by the DFS-tree formed by applying DFS to a connected undirected
graph $G$.
\begin{enumerate}
\item If $(u,v) \in T$ then $dfsnum[u] < dfsnum[v]$.
\item If $(u,v)$ is a back edge then $dfsnum[v] < dfsnum[u]$.
\item Let $T_u \subset T$ be the subtree rooted at $u$. Then
  for all $v \in T_u$ we have $dfsnum[u] < dfsnum[v]$
  and for all $w \notin T_u$ we haven $dfsnum[w] < dfsnum[u]$.
\item If $(u,v)$ is a cross edge then $dfsnum[v] < dfsnum[u]$
 (cross edges do not occur in a connected graph).
\item Any path $p: v \stackrel{*}{\rightarrow} w$ contains a vertex that is
an ancestor of both $v$ and $w$ in $T$.
\end{enumerate}


\paragraph{Definition.} An undirected graph $G = (V,E)$ is \emph{biconnected}
if for each triple of distinct vertices $v, w, a \in V$ there is a 
path $p : v \stackrel{*}{\rightarrow} w$ such that $a$ is not on the
path $p$. 

\paragraph{Definition.} An \emph{articulation point} of $G = (V,E)$
is a vertex $a \in V$ where there are two other distinct vertices $v,w
\in V$ such that $a$ is on any path $p:v \stackrel{*}{\rightarrow} w$
and there is at least one such path.

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
$dfsnum[u] < dfsnum[v]$. Define $LOWPT(v)$ to be the vertex with the
smallest DFS number in the set $\{v\} \cup \{w|v
\stackrel{*}\rightarrow \hookrightarrow w\}$. That is, $LOWPT(V)$ is
the smallest vertex reachable from $v$ by traversing zero or more
DFS-tree arcs followed by at most one back edge.

\paragraph{Lemma.} Given a DFS-tree $T$ of a connected undirected graph
$G$, suppose $a,v,w$ are distinct vertices in $G$ such that $(a,v)$ is
in of $G$ and $w$ is not a descendant of $v$ in $T$.  If
$dfsnum[LOWPT(v)] \geq dfsnum[a]$, then $a$ is an articulation point
of $P$ and removal of $a$ disconnects $v$ and $w$. Conversely, if $a$
is an articulation point of $G$ then there exists vertices $a,v,w$
with $(a,v) \in T$, $w$ is not a descendant of $v$, and
$dfsnum[LOWPT(v)] \geq dfsnum[a]$.

Lemma. If $(a, v) \in G$ and $dfsnum[LOWPT(v)] \geq dfsnum[a]$
we know that any path from $v$ not passing through $a$ remains in
the subtree $T_v$.

Proof by contradiction.  Because $u \notin T_v$ we know
that $dfsnum[u] < dfsnum[v]$.

Assume that $u \notin T_v$ and $p : v
\rightsquigarrow u$ with $a$ not on the path $p$. 




Also the subtree does not contain vertex $w$.

\end{document}


\documentclass[11pt]{report}

%\input{defs}


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

This paper documents the implementation of the \texttt{simple\_isomorphism()}
function of the Boost Graph Library.

An \emph{isomorphism} is a one-to-one mapping of the vertices in one
graph to the vertices of another graph such that adjacency is
preserved. Another words, given graphs $G_{1} = (V_{1},E_{1})$ and
$G_{2} = (V_{2},E_{2})$, an isomorphism is a function $f$ such that
for all pairs of vertices $a,b$ in $V_{1}$, edge $(a,b)$ is in $E_{1}$
if and only if edge $(f(a),f(b))$ is in $E_{2}$. Both graphs must
be the same size, so let $N = |V_1| = |V_2|$. The graph $G_1$
is \emph{isomorphic} to $G_2$ if an isomorphism exists between
the to graphs, which we denote by $G_1 \isomorphic G_2$.

\section{Exhaustive Backtracking Search}

The algorithm used by the \texttt{simple\_isomorphism()} function is,
at first approximation, an exhaustive search implemented via
backtracking.  The backtracking algorithm is a recursive function. At
each stage we will try to extend the match that we have found so far.
So we have two subgraphs $S_1 \subseteq G_1$ and $S_2 \subseteq G_2$,
and suppose we have already determined that $S_1 \isomorphic S_2$. We
then try to add a vertex to $S_1$ and $S_2$ such that the new
subgraphs are still isomorphic to one another.

The vertices of $G_1$ are labelled $\{1,\ldots,N\}$ and the subgraphs
$S_1$ that we will be considering will consist of increasing numbers
of vertices from $G_1$ in order of their label.  Let $S_1(k)$ denote
the subgraph of $G_1$ induced by the first $k$ vertices, and $S_1(0)$
is an empty graph.  At each stage we will try to extend the match from
$S_1(k)$ to $S_1(k+1)$ by finding a vertex in $G_2 - S_2$ that matches
(has the same adjacency structure as) the $k+1$st vertex of $G_1$.

\begin{tabbing}
IS\=OM\=OR\=PH\=($k$, $G_1$, $S_2$, $G_2$, $f$) $\equiv$ \\
\>\textbf{if} ($S_2 = G_2$) \\
\>\>\textbf{return} true \\
\>\textbf{for} each vertex $v \in G_2 - S_2$ \\
\>\>\textbf{if} (MATCH($k+1$, $v$)) \\
\>\>\>$f(k+1) \leftarrow v$ \\
\>\>\>ISOMORPH($k+1$, $G_1$, $S_2 \cup \{ v \}$, $G_2$)\\
\>\>\textbf{else}\\
\>\>\>\textbf{return} false
\\
ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)
\end{tabbing}

The MATCH operation is as follows. The basic idea is to check to see
if the adjacency structure of $v_1$ is the same as the adjacency
structure of $v_2$. For each edge $(v_1,w) \in G_1$ we check to see if
$(v_2, f(w)) \in G_2$ and for each edge $(w,v_1) \in G_1$ we check to
see if $(f(w), v_2) \in G_2$. Note that so far we have only defined
the mapping $f$ for vertices with labels equal to or smaller than
$v_1$, so we do not look at edges that are incident on vertices with
larger labels. Those edges will be examined in later stages.

\begin{tabbing}
MA\=TC\=H(\=$v_1$\=, $v_2$, $G_1$, $G_2$, $f$) $\equiv$ \\
\>\textbf{for} each out edge $(v_1, w) \in G_1$ \\
\>\>\textbf{if} ($w < v_1$) \\
\>\>\>\textbf{if} ( $(v_2,f(w)) \notin G_2$ ) \\
\>\>\>\>\textbf{return} false\\
\>\textbf{for} each in edge $(w, v_1) \in G_1$ \\
\>\>\textbf{if} ($w < v_1$) \\
\>\>\>\textbf{if} ($(f(w),v_2) \notin G_2$) \\
\>\>\>\>\textbf{return} false \\
\>\textbf{return} true
\end{tabbing}

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
$f(v) = v'$. Therefore, when looking for a match to some vertex $v$,
we only need to consider those vertices that have the same vertex
invariant number. The number of vertices in a graph $g$ with the same
vertex invariant number $i$ is called the \emph{invariant
multiplicity} of $i$ in $g$.  In this implementation, by default we
use the out-degree of the vertex as the vertex invariant, though the
user can also supply there own invariant function. The ability of the
invariant function to prune the search space varies widely with the
type of graph.

As a first check to see if there is no possibility of matching the two
graphs, one create a list of computed vertex invariants for the
vertices in each graph, sort the two lists, and then compare them.  If
the two lists are different then the two graphs are not isomorphic.
If the two lists are the same then the two graphs may be isomorphic.


\section{Vertex Order}

A good choice of the labeling for the vertices (which determines the
order in which the subgraph $S_1$ is grown) can also reduce the search
space. In the following we discuss two labeling heuristics.

\subsection{Most Constrained First}

Consider the most constrained vertices first.  That is, examine
lower-degree vertices before higher-degree vertices. This reduces the
search space because it chops off a trunk before the trunk has a
chance to blossom out. We can generalize this a bit to use vertex
invariants, so we examine vertices with low invariant multiplicilty
before examining vertices with high invariant multiplicity.

\subsection{Adjacent First}

The MATCH operation only considers edges when the other vertex already
has a mapping defined. This means that the MATCH operation can only
weed out vertices that are adjacent to vertices that have already been
matched. Therefore, when choosing the next vertex to examine, it is
desirable to choose one that is adjacent a vertex already in $S_1$.

\subsection{DFS Order, Starting with Lowest Degree}

For this implementation, we combine the above two heuristics in the
following way. To implement the ``adjacent first'' heuristic we apply
DFS to the graph, and use the DFS discovery order as our vertex
order. To comply with the ``most constrained first'' heuristic we
start the DFS at the vertex with the lowest degree, and each time we
restart the DFS, we choose the undiscovered vertex with lowest degree.




\end{document}

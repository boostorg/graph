\documentclass[11pt]{report}

%\input{defs}


\setlength\overfullrule{5pt}
\tolerance=10000
\sloppy
\hfuzz=10pt

\makeindex

\newcommand{\isomorphic}{\equiv}

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


The algorithm used by the \texttt{simple\_isomorphism()} function is,
at first approximation, an exhaustive search implemented via
backtracking.  The backtracking algorithm is a recursive function. At
each stage we will try to extend the match that we have found so far.
So we have two subgraphs $S_1 \subseteq G_1$ and $S_2 \subseteq G_2$,
and suppose we have already determined that $S_1 \isomorphic S_2$ . We
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
IS\=OM\=OR\=PH\=($k$, $G_1$, $S_2$, $G_2$) \\
\>\textbf{for} each vertex $v \in G_2 - S_2$ \\
\>\>\textbf{if} (MATCH($k+1$, $v$)) \\
\>\>\>$f(k+1) \leftarrow v$ \\
\>\>\>ISOMORPH($k+1$, $G_1$, $S_2 \cup \{ v \}$, $G_2$)\\
\\
ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)
\end{tabbing}

There are $N!$ possible vertex mappings, so the idea is to try each
mapping. Of course, $N!$ gets very large as $N$ increases, so
something has to be done to prune the search space. We use pruning
techniques described in \cite{reingold77:_combin_algo} and which
originated in \cite{sussenguth65:_isomorphism,unger64:_isomorphism}.




\end{document}

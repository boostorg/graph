\documentclass[11pt]{report}

%\input{defs}
\usepackage{math}
\usepackage{jweb}
\usepackage{lgrind}
\usepackage{times}
\usepackage{fullpage}
\usepackage{graphicx}

\newif\ifpdf
\ifx\pdfoutput\undefined
   \pdffalse
\else
   \pdfoutput=1
   \pdftrue
\fi

\ifpdf
  \usepackage[
              pdftex,
              colorlinks=true, %change to true for the electronic version
              linkcolor=blue,filecolor=blue,pagecolor=blue,urlcolor=blue
              ]{hyperref}
\fi

\ifpdf
  \newcommand{\stlconcept}[1]{\href{http://www.sgi.com/tech/stl/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\bglconcept}[1]{\href{http://www.boost.org/libs/graph/doc/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\pmconcept}[1]{\href{http://www.boost.org/libs/property_map/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\myhyperref}[2]{\hyperref[#1]{#2}}
  \newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.pdf}}\caption{#2}\label{fig:#1}\end{figure}}
\else
  \newcommand{\myhyperref}[2]{#2}
  \newcommand{\bglconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\pmconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\stlconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.eps}}\caption{#2}\label{fig:#1}\end{figure}}
\fi

\newcommand{\code}[1]{{\small{\em \textbf{#1}}}}


% jweb -np isomorphism-impl.w; dot -Tps out.dot -o out.eps; dot -Tps in.dot -o in.eps; latex isomorphism-impl.tex; dvips isomorphism-impl.dvi -o isomorphism-impl.ps

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

This paper documents the implementation of the \code{isomorphism()}
function of the Boost Graph Library.  The implementation was by Jeremy
Siek with algorithmic improvements and test code from Douglas Gregor.
The \code{isomorphism()} function answers the question, ``are these
two graphs equal?''  By \emph{equal}, we mean the two graphs have the
same structure---the vertices and edges are connected in the same
way. The mathematical name for this kind of equality is
\emph{isomorphic}.

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
= \{ \pair{a_1}{b_1}, \ldots, \pair{a_n}{b_n} \}$
means $f(a_i) = b_i$ for $i=1,\ldots,n$.

\section{Exhaustive Backtracking Search}

The algorithm used by the \code{isomorphism()} function is, at
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
\\
ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)
\end{tabbing}

The basic idea of the match operation is to check whether $G_1[k]$ is
isomorphic to $G_2[S \union \{ v \}]$. We already know that $G_1[k-1]
\isomorphic G_2[S]$ with the mapping $f_{k-1}$, so all we need to do
is verify that the edges in $E_1[k] - E_1[k-1]$ connect vertices that
correspond to the vertices connected by the edges in $E_2[S \union \{
v \}] - E_2[S]$. The edges in $E_1[k] - E_1[k-1]$ are all the
out-edges $(k,j)$ and in-edges $(j,k)$ of $k$ where $j$ is less than
or equal to $k$ according to the ordering.  The edges in $E_2[S \union
\{ v \}] - E_2[S]$ consists of all the out-edges $(v,u)$ and
in-edges $(u,v)$ of $v$ where $u \in S$.

\begin{tabbing}
M\=ATCH($k$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (k,j) \in E_1[k] - E_1[k-1] \Big( (v,f(j)) \in E_2[S \union \{ v \}] - E_2[S] \Big)$ \\
\>$in \leftarrow \forall (j,k) \in E_1[k] - E_1[k-1] \Big( (f(j),v) \in E_2[S \union \{ v \}] - E_2[S] \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}

The problem with the exhaustive backtracking algorithm is that there
are $N!$ possible vertex mappings, and $N!$ gets very large as $N$
increases, so we need to prune the search space. We use the pruning
techniques described in
\cite{deo77:_new_algo_digraph_isomorph,fortin96:_isomorph,reingold77:_combin_algo}
that originated in
\cite{sussenguth65:_isomorphism,unger64:_isomorphism}.

\section{Vertex Invariants}
\label{sec:vertex-invariants}

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
\>$out \leftarrow \forall (k,j) \in E_1[k] - E_1[k-1] \Big( (v,f(j)) \in E_2[S \union \{ v \}] - E_2[S] \Land i(v) = i(k) \Big)$ \\
\>$in \leftarrow \forall (j,k) \in E_1[k] - E_1[k-1] \Big( (f(j),v) \in E_2[S \union \{ v \}] - E_2[S] \Land i(v) = i(k) \Big)$ \\
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
order the roots of our DFS trees by invariant multiplicity.


\section{Implementation}



@d Invariant multiplicity comparison functor
@{
struct cmp_multiplicity
{
    cmp_multiplicity(self& algo, size_type* multiplicity)
        : algo(algo), multiplicity(multiplicity) { }
    bool operator()(const vertex_a& x, const vertex_a& y) const {
        return multiplicity[algo.ADEG(x)] < multiplicity[algo.ADEG(y)];
    }
    self& algo;
    size_type* multiplicity;
};
@}


Use \code{pair} instead of \code{edge_a} so that we can create ficticious
edges for the DFS tree roots.

@d State used inside the DFS Visitor
@{
struct dfs_order {
    dfs_order(std::vector<vertex_a>& v) : vertices(v) { }
    std::vector<vertex_a>& vertices;
    std::vector< std::pair<int,int> > edges;
};
@}

@d DFS visitor to record vertex and edge order
@{
struct record_dfs_order : default_dfs_visitor {
    record_dfs_order(dfs_order& order) : order(order) { }
    void start_vertex(vertex_a v, const GraphA&) const {
        order.edges.push_back(make_pair(-1, v));
    }
    void discover_vertex(vertex_a v, const GraphA&) const {
        order.vertices.push_back(v);
    }
    void examine_edge(edge_a e, const GraphA& Ga) const {
        order.edges.push_back(make_pair(source(e, Ga), target(e, Ga)));
    }
    dfs_order& order;
};
@}

ficticiuos edges for the DFS tree roots

@d Ordered edge class
@{
struct ordered_edge {
    ordered_edge(int f, int t) : from(f), to(t) { }

    bool operator<(const ordered_edge& e) const {
        using namespace std;
        int m1 = max(from, to);
        int m2 = max(e.from, e.to);
        // lexicographical comparison of (m1,from,to) and (m2,e.from,e.to)
        return make_pair(m1, make_pair(from, to)) < make_pair(m2, make_pair(e.from, e.to));
    }
    int from;
    int to;
    int order;
};
@}

@d Quick return if the vertex invariants do not match up
@{
{
    std::vector<size_type> ADEG_Ga;
    BGL_FORALL_VERTICES_T(v, Ga, GraphA)
    ADEG_Ga.push_back(ADEG(v));
    std::sort(ADEG_Ga.begin(), ADEG_Ga.end());

    std::vector<size_type> BDEG_Gb;
    BGL_FORALL_VERTICES_T(v, Gb, GraphB)
    BDEG_Gb.push_back(BDEG(v));
    std::sort(BDEG_Gb.begin(), BDEG_Gb.end());

    if (!std::equal(ADEG_Ga.begin(), ADEG_Ga.end(), BDEG_Gb.begin())) {
        std::cout << "invariants don't match" << std::endl;
        return false;
    }
}
@}

@d Sort vertices according to invariant multiplicity
@{
std::vector<vertex_a> Vmult;
BGL_FORALL_VERTICES_T(v, Ga, GraphA)
    Vmult.push_back(v);
{
    std::vector<size_type> multiplicity(this->max_DEG(), 0);
    BGL_FORALL_VERTICES_T(v, Ga, GraphA)
    ++multiplicity[ADEG(v)];

    std::sort(Vmult.begin(), Vmult.end(), cmp_multiplicity(*this, &multiplicity[0]));

    std::cout << "Vmult=";
    std::copy(Vmult.begin(), Vmult.end(),
            std::ostream_iterator<vertex_a>(std::cout, " "));
    std::cout << std::endl;
}
@}

@d Order vertices and edges by DFS
@{
{
    dfs_order order(dfs_vertices);
    std::vector<default_color_type> color_vec(num_vertices(Ga));
    record_dfs_order dfs_visitor(order);
    typedef color_traits<default_color_type> Color;
    for (vertex_iter u = Vmult.begin(); u != Vmult.end(); ++u) {
        if (color_vec[*u] == Color::white()) {
            dfs_visitor.start_vertex(*u, Ga);
            depth_first_visit(Ga, *u, dfs_visitor, &color_vec[0]);
        }
    }
    // Create the dfs_number array
    size_type n = 0;
    dfs_number.resize(num_vertices(Ga));
    for (vertex_iter v = dfs_vertices.begin(); v != dfs_vertices.end(); ++v)
        dfs_number[*v] = n++;
    
    // Construct the ordered_edges array
    for (std::vector<std::pair<int,int> >::iterator e = order.edges.begin();
         e != order.edges.end(); ++e) {
        int u = e->first < 0 ? e->first : dfs_number[e->first];
        int v = dfs_number[e->second];
        ordered_edges.push_back(ordered_edge(u, v));
    }
}
@}

Reorder the edges so that all edges belonging to $G_1[k]$
appear before any edges not in $G_1[k]$, for $k=1,...,n$.

The order field needs a better name. How about k?

@d Sort edges according to vertex DFS order
@{
{
    std::stable_sort(ordered_edges.begin(), ordered_edges.end());
    // Fill in i->order field
    ordered_edges[0].order = 0;
    for (edge_iter i = next(ordered_edges.begin()); i != ordered_edges.end(); ++i)
        i->order = std::max(prior(i)->from, prior(i)->to);
}
@}


@d $v$ is a DFS tree root
@{
std::cout << "** case 1" << std::endl;
// Try all possible mappings
BGL_FORALL_VERTICES_T(y, Gb, GraphB) {
    std::cout << "y: " << b_names[y] << std::endl;
    if (ADEG(v) == BDEG(y) && f_inv_assigned[y] == false) {
        std::cout << "f(" << a_names[v] << ")=" <<b_names[y] << std::endl;
        f[v] = y; 
        f_assigned[v] = true;
        f_inv[y] = v; f_inv_assigned[y] = true;
        mc = 0;
        std::cout << "mc = 0" << std::endl;
        if (match(next(iter)))
            return true;
        f_assigned[v] = false;
        f_inv_assigned[y] = false;
    }
    std::cout << "xxx" << std::endl;
}
@}

Growing the subgraph.

@d $v$ is an unmatched vertex, $(u,v)$ is a tree edge
@{
std::cout << "** case 2" << std::endl;
vertex_a k = dfs_vertices[edge_order_num];
std::cout << "k=" << a_names[k] << std::endl;
assert(f_assigned[k] == true);
std::cout << "f[k]: " << b_names[f[k]] << std::endl;

@<Count out-edges of $f(k)$ in $G_2[S]$@>
@<Count in-edges of $f(k)$ in $G_2[S]$@>

std::cout << "mc: " << mc << std::endl;
if (mc != 0) // make sure out/in edges for k and f(k) add up
    return false;
@<Assign $v$ to some vertex in $V_2 - S$@>
@}

@d Count out-edges of $f(k)$ in $G_2[S]$
@{
BGL_FORALL_ADJACENT_T(f[k], w, Gb, GraphB) {
    if (f_inv_assigned[w] == true) {
        --mc;
        std::cout << "--mc: " << mc << std::endl;
        std::cout << "(" << b_names[f[k]] << "," << b_names[w] << ")\n";
    }
}
@}

@d Count in-edges of $f(k)$ in $G_2[S]$
@{
for (std::size_t ji = 0; ji < edge_order_num; ++ji) {
    vertex_a j = dfs_vertices[ji];
    BGL_FORALL_ADJACENT_T(f[j], w, Gb, GraphB) {
        if (w == f[k]) {
            --mc;
            std::cout << "--mc: " << mc << std::endl;
            std::cout << "(" << b_names[f[j]] << "," << b_names[w] << ")\n";
        }
    }
}
@}

@d Assign $v$ to some vertex in $V_2 - S$
@{
BGL_FORALL_ADJACENT_T(f[u], y, Gb, GraphB) {
    if (ADEG(v) == BDEG(y) && f_inv_assigned[y] == false) {
        f[v] = y; f_assigned[v] = true;
        std::cout << "f(" << a_names[v] << ")=" << b_names[y] << std::endl;;
        f_inv[y] = v; f_inv_assigned[y] = true;
        mc = 1;
        std::cout << "(f(u),y): (" << b_names[f[u]] << "," << b_names[y]
                    << ")" << std::endl;
        std::cout << "mc = 1" << std::endl;
        if (match(next(iter)))
            return true;
        f_assigned[v] = false;
        f_inv_assigned[y] = false;
    }
}	    
@}



@d Check to see if there is an edge in $G_2$ to match $(u,v)$
@{
std::cout << "** case 3" << std::endl;
bool verify = false;
assert(f_assigned[u] == true);
BGL_FORALL_ADJACENT_T(f[u], y, Gb, GraphB) {
    std::cout << "y: " << b_names[y] << std::endl;
    assert(f_assigned[v] == true);
    if (y == f[v]) {    
        std::cout << "found match, (" << b_names[f[u]] 
                << "," << b_names[y] << ")" << std::endl;
        verify = true;
        break;
    }
}
if (verify == true) {
    ++mc; // out or in edge of k
    std::cout << "++mc: " << mc << std::endl;
    if (match(next(iter)))
    return true;
}
@}

@d Match function
@{
bool match(edge_iter iter)
{
std::cout << "*** entering match" << std::endl;
if (iter != ordered_edges.end()) {
    ordered_edge edge = *iter;
    size_type edge_order_num = edge.order;
    vertex_a u;
    if (edge.from != -1) // might be a ficticious edge
        u = dfs_vertices[edge.from];
    vertex_a v = dfs_vertices[edge.to];
    std::cout << "edge: (";
    if (edge.from == -1)
        std::cout << "root";
    else
        std::cout << a_names[dfs_vertices[edge.from]];
    std::cout << "," << a_names[dfs_vertices[edge.to]] << ")" << std::endl;
    if (edge.from == -1) { // root node
        @<$v$ is a DFS tree root@>
    } else if (f_assigned[v] == false) {
        @<$v$ is an unmatched vertex, $(u,v)$ is a tree edge@>
    } else {
        @<Check to see if there is an edge in $G_2$ to match $(u,v)$@>
    }
} else 
    return true;
std::cout << "returning false" << std::endl;
return false;
} // match()
@}


@o isomorphism-v2.hpp
@{
#ifndef BOOST_GRAPH_ISOMORPHISM_HPP
#define BOOST_GRAPH_ISOMORPHISM_HPP

#include <iostream>
#include <utility>
#include <vector>
#include <iterator>
#include <algorithm>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

namespace boost {

namespace detail {
    
template <typename GraphA, typename GraphB>
class isomorphism_algo
{
    typedef isomorphism_algo self;
    typedef typename graph_traits<GraphA>::vertex_descriptor vertex_a;
    typedef typename graph_traits<GraphB>::vertex_descriptor vertex_b;
    typedef typename graph_traits<GraphA>::edge_descriptor edge_a;
    typedef typename graph_traits<GraphA>::vertices_size_type size_type;

    GraphA& Ga;
    GraphB& Gb;
    friend struct cmp_multiplicity;
    @<Invariant multiplicity comparison functor@>
    typedef std::vector<vertex_a>::iterator vertex_iter;

    @<State used inside the DFS Visitor@>
    @<DFS visitor to record vertex and edge order@>
    std::vector<vertex_a> dfs_vertices;
    std::vector<size_type> dfs_number;
public:
    char* a_names;
    char* b_names;

    @<Ordered edge class@>
  
    std::vector<ordered_edge> ordered_edges;
    typedef std::vector<ordered_edge>::iterator edge_iter;
public:

    isomorphism_algo(GraphA& Ga, GraphB& Gb)
	: Ga(Ga), Gb(Gb) { }

    bool test_isomorphism()
    {
        @<Quick return if the vertex invariants do not match up@>
        @<Sort vertices according to invariant multiplicity@>
        @<Order vertices and edges by DFS@>
        @<Sort edges according to vertex DFS order@>
        
        f.resize(num_vertices(Ga));
        f_assigned.resize(num_vertices(Ga));
        f_inv.resize(num_vertices(Ga));
        f_inv_assigned.resize(num_vertices(Ga));

	return this->match(ordered_edges.begin());
    } // test_isomorphism

    std::vector<vertex_b> f;
    std::vector<vertex_a> f_inv;
    std::vector<bool> f_assigned;
    std::vector<bool> f_inv_assigned;
    int mc; // #edges incident on k

    @<Match function@>
	
    void print_ordered_edges() {
	std::cout << "ordered edges=";
	for (edge_iter i = ordered_edges.begin(); i != ordered_edges.end(); ++i)
	  std::cout << "[" << a_names[dfs_vertices[i->from]]
                    << "(" << i->from << ")"
                    << "," << a_names[dfs_vertices[i->to]] << "(" << i->to << ")" 
                    << " : " << i->order << ")";
	std::cout << std::endl;
    }

    void print_dfs_numbers() {
	std::cout << "dfs numbers=";
	std::copy(dfs_number.begin(), dfs_number.end(),
		  std::ostream_iterator<vertex_a>(std::cout, " "));
	std::cout << std::endl;
    }

private:

    size_type ADEG(vertex_a i) const {
	return (num_vertices(Ga) + 1) * out_degree(i, Ga) + in_degree(i, Ga);
    }
    size_type BDEG(vertex_b i) const {
	return (num_vertices(Gb) + 1) * out_degree(i, Gb) + in_degree(i, Gb);
    }
    size_type max_DEG() const {
	return (num_vertices(Ga) + 1) * num_vertices(Ga) + num_vertices(Ga);
    }
};
    
} // namespace detail

} // namespace boost

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_ISOMORPHISM_HPP
@}



\bibliographystyle{abbrv}
\bibliography{ggcl}

\end{document}
% LocalWords:  Isomorphism Siek isomorphism adjacency subgraph subgraphs OM DFS
% LocalWords:  ISOMORPH Invariants invariants typename IndexMapping bool const
% LocalWords:  VertexInvariant VertexIndexMap iterator typedef VertexG Idx num
% LocalWords:  InvarValue struct invar vec iter tmp_matches mult inserter permute ui
% LocalWords:  dfs cmp isomorph VertexIter edge_iter_t IndexMap desc RPH ATCH pre

% LocalWords:  iterators VertexListGraph EdgeListGraph BidirectionalGraph tmp
% LocalWords:  ReadWritePropertyMap VertexListGraphConcept EdgeListGraphConcept
% LocalWords:  BidirectionalGraphConcept ReadWritePropertyMapConcept indices ei
% LocalWords:  IndexMappingValue ReadablePropertyMapConcept namespace InvarFun
% LocalWords:  MultMap vip inline bitset typedefs fj hpp ifndef adaptor params
% LocalWords:  bgl param pmap endif

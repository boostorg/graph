//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// A script to generate simple pairs of graphs of (possibly) different
// size, such that there exists (at least) one (sub)graph isomorphism mapping
// between the two graphs. The graphs are written to files graph_small.dot
// and graph_large.dot using the Graphviz DOT language http://www.graphviz.org.  
// The following parameters can be used to control the output:
//
// - nbig:     Dimension of the large adjacency matrix
// - nsmall:   Dimension of the small adjacency matrix
// - density:  Density of the non-zero entries (of an initial square
//             matrix with dimension nbig)
// - directed: If set to one, a pair of directed graphs is generated,
//             otherwise undirected graphs are produced.
// - loops:    If set to one, self-loops are allowed, otherwise self-loops
//             are excluded.
//
// The generated dot-files specifying the graphs can be given as command line
// arguments to the executable test program (vf2_sub_graph_iso_gviz_example.cpp), 
// which uses boost's GraphViz input parser to build the graphs. 

clear;

directed=0;           // Set to 1 to generate a directed graph, otherwise an 
                      // undirected graph is generated

loops=1;              // Set to 1 to allow self-loops, otherwise loops are excluded

nbig=6; density=0.4;  // Size and density of non-zero elements of the large matrix
nsmall=4;             // Size of the small matrix: nsmall<=nbig

// Create a matrix with ~density * nbig^2 non-zero elements
M=full(sprand(nbig, nbig, density, "uniform"));
NZ=find(M<>0);
M(NZ)=1;

if directed <> 1 then
    M=triu(M);
end

if loops <> 1 then
    M=M-eye(M).*M    
end

indices=linspace(1, nbig, nbig)';

// Random row and column permutations
indices_perm=grand(1, 'prm', indices);

M_perm=M(indices_perm, indices_perm);
M_perm=M_perm(1:nsmall, 1:nsmall);

function write_digraph(file_name, Mat)
    fd = mopen(file_name, "w");
    n = size(Mat, "r");
    mfprintf(fd, "digraph G {\n");
    for i = 1:n
        for j = 1:n
            if Mat(i,j)<>0 then
                mfprintf(fd, "node%u -> node%u;\n", i, j);
            end
        end
    end
    mfprintf(fd, "}\n");
    mclose(fd);    
endfunction

function write_graph(file_name, Mat)
    fd = mopen(file_name, "w");
    n = size(Mat, "r");
    mfprintf(fd, "graph G {\n");
    for i = 1:n
        for j = 1:n
            if Mat(i,j)<>0 then
                mfprintf(fd, "node%u -- node%u;\n", i, j);
            end
        end
    end
    mfprintf(fd, "}\n");
    mclose(fd);    
endfunction

// Write graphs:
if directed <> 1 then
    write_graph("graph_large.dot", M);
    write_graph("graph_small.dot", M_perm);
else
    write_digraph("graph_large.dot", M);
    write_digraph("graph_small.dot", M_perm);
end

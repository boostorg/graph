%{
//=======================================================================
// Copyright 2001 University of Notre Dame.
// Author: Lie-Quan Lee
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Include this first, since it declares YYSTYPE which will
// be defined in "*parser.h" if not previosly declared here.


#include "yystype.h"
#define YY_DECL   int yylex(YYSTYPE* lvalp)

// When GRAPHVIZ_DIRECTED is declared it means Boost.Build is used.
// Headers for directed and undirected graphs are both generated, and
// have different names.
#ifdef GRAPHVIZ_DIRECTED
    #if GRAPHVIZ_DIRECTED == 0
        #define yyerror bgl_undir_error
        #include "graphviz_graph_parser.hpp"
    #else
        #define yyerror bgl_dir_error
        #include "graphviz_digraph_parser.hpp"
    #endif
#else
    #error Need to define the GRAPHVIZ_DIRECTED macro to either 0 or 1
#endif


#include <string>
#include <iostream>

static std::string literal;
static void begin_literal();
static void add_string(const char* str);
static void end_literal();

%}

%option noyywrap
%x Comment
%x Literal

DIGIT     [0-9]
ALPHABET  [a-zA-Z_]
NUMBER    [-+]?({DIGIT}+(\.{DIGIT}*)?|\.{DIGIT}+)([eE][-+]?{DIGIT}+)?
VARIABLE  {ALPHABET}+({DIGIT}|{ALPHABET})*
ID        {VARIABLE}|{NUMBER}

%%

"/*"            { BEGIN Comment; }
<Comment>"*/"   { BEGIN INITIAL; }
<Comment>.*     ;
"//".*$         ;
"#".*$          ;
"digraph"       { return DIGRAPH_T; }
"graph"         { return GRAPH_T; }
"node"          { return NODE_T; }
"edge"          { return EDGE_T; }
"strict"        ;
"subgraph"      { return SUBGRAPH_T; }
"->"|"--"       { return EDGEOP_T; }
{ID}            { lvalp->ptr = (void*)(new std::string(yytext)); return ID_T; }
["]             { BEGIN Literal; begin_literal();  }
<Literal>["]     { BEGIN INITIAL; end_literal(); lvalp->ptr = (void*)(new std::string(literal)); return ID_T; }
<Literal>[\\][\n]     ;
<Literal>([^"\\]*|[\\].)  { add_string(yytext); }
[ \t]           ;
\n              ;
\r              ;
.               { return yytext[0]; }
%%


void begin_literal() {
   literal = "";
}

void add_string(const char* str) {
   literal += str;
}

void end_literal() {
}

void yyerror(char *str)
{
  std::cout << str << std::endl;
}

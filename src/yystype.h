//  (C) Copyright Jeremy Siek 2004 
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_YYSTYPE_H
#define BOOST_GRAPH_YYSTYPE_H

union yystype {
  int         i;
  void*       ptr;
};
#define YYSTYPE yystype

#endif // BOOST_GRAPH_YYSTYPE_H

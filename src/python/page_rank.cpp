// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "digraph.hpp"
#include <boost/graph/page_rank.hpp>
#include "done.hpp"

namespace boost { namespace graph { namespace python {



template<typename Graph>
void 
page_rank_ri
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& rank,
   int iterations)
{
  boost::graph::page_rank(g, rank, graph::n_iterations(20));
}

template<typename Graph>
void 
page_rank_i(Graph& g, int iterations)
{
  boost::graph::page_rank(g, g.template get_vertex_map<double>("rank"),
                          graph::n_iterations(20));
}

struct page_rank_wrap_done
{
  page_rank_wrap_done(const done& d) : d(d) { }

  template<typename RankMap, typename Graph>
  bool
  operator()(const RankMap&, const Graph&) const
  { 
    return d();
  }

  const done& d;
};

template<typename Graph>
void 
page_rank_rd
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& rank,
   const done& d)
{
  boost::graph::page_rank(g, rank, page_rank_wrap_done(d));
}

template<typename Graph>
void page_rank_d(Graph& g, const done& d)
{
  boost::graph::page_rank(g, g.template get_vertex_map<double>("rank"), 
                          page_rank_wrap_done(d));
}

void export_page_rank()
{
  def("page_rank", &page_rank_i<Digraph>);
  def("page_rank", &page_rank_ri<Digraph>);
  def("page_rank", &page_rank_rd<Digraph>);
  def("page_rank", &page_rank_d<Digraph>);
}


} } } // end namespace boost::graph::python

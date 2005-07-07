// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "digraph.hpp"
#include <boost/graph/page_rank.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
page_rank_iterations
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>* in_rank,
   int iterations)
{
  typedef vector_property_map<double, typename Graph::VertexIndexMap> 
    RankMap;

  RankMap rank = 
    in_rank? *in_rank : g.template get_vertex_map<double>("pagerank");

  boost::graph::page_rank(g, rank, graph::n_iterations(20));
}

struct page_rank_wrap_done
{
  page_rank_wrap_done(boost::python::object done) : done(done) { }

  template<typename RankMap, typename Graph>
  bool
  operator()(const RankMap& rank, const Graph& g) const
  { 
    using boost::python::extract;
    return extract<bool>(done(rank, g));
  }

private:
  boost::python::object done;
};


template<typename Graph>
void 
page_rank_done
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>* in_rank,
   boost::python::object done)
{
  typedef vector_property_map<double, typename Graph::VertexIndexMap> 
    RankMap;

  RankMap rank = 
    in_rank? *in_rank : g.template get_vertex_map<double>("pagerank");

  boost::graph::page_rank(g, rank, page_rank_wrap_done(done));
}

void export_page_rank()
{
  using boost::python::arg;
  using boost::python::def;

  def("page_rank", &page_rank_iterations<Digraph>,
      (arg("graph"), 
       arg("rank_map") = 
         (vector_property_map<double, Digraph::VertexIndexMap>*)0,
       arg("iterations") = 20));
  def("page_rank", &page_rank_done<Digraph>,
      (arg("graph"), 
       arg("rank_map") = 
         (vector_property_map<double, Digraph::VertexIndexMap>*)0,
       arg("done")));
}


} } } // end namespace boost::graph::python

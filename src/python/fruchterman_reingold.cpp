// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/random_layout.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "point2d.hpp"
#include <boost/random/linear_congruential.hpp>
#include <ctime>

namespace boost { namespace graph { namespace python {

template<typename F, typename Result = double>
struct python_or_functor
{
  explicit python_or_functor(boost::python::object callable, const F& f = F()) 
    : callable(callable), f(f) { }

  // For cooling
  Result operator()()
  {
    using boost::python::object;
    using boost::python::extract;
    if (callable != object()) return extract<Result>(callable());
    else return f();
  }

  // For the attractive_force
  template<typename Graph>
  Result 
  operator()(typename graph_traits<Graph>::edge_descriptor e,
             double k, double dist, const Graph& g) const
  {
    using boost::python::object;
    using boost::python::extract;
    if (callable != object()) return extract<Result>(callable(e, k, dist, g));
    else return f(e, k, dist, g);
  }

  // For the repulsive_force
  template<typename Graph>
  Result 
  operator()(typename graph_traits<Graph>::vertex_descriptor u,
             typename graph_traits<Graph>::vertex_descriptor v,
             double k, double dist, const Graph& g) const
  {
    using boost::python::object;
    using boost::python::extract;
    if (callable != object()) 
      return extract<Result>(callable(u, v, k, dist, g));
    else return f(u, v, k, dist, g);
  }

private:
  boost::python::object callable;
  F f;
};


template<typename Graph>
void 
fruchterman_reingold_force_directed_layout
  (Graph& g,
   const vector_property_map<point2d, typename Graph::VertexIndexMap>* in_pos,
   double width, double height,
   boost::python::object attractive_force,
   boost::python::object repulsive_force,
   // TBD: force pairs?
   boost::python::object cooling,
   bool progressive)
{
  using boost::python::object;

  typedef vector_property_map<point2d, typename Graph::VertexIndexMap> 
    PositionMap;

  PositionMap pos = 
    in_pos? *in_pos : g.template get_vertex_map<point2d>("position");

  if (!progressive) {
    minstd_rand gen(std::time(0));
    random_graph_layout(g, pos, -width/2, width/2, -height/2, height/2, gen);
  }
  
  python_or_functor<linear_cooling<double> > cool(cooling, 100);

  if (attractive_force != object() || repulsive_force != object()) {
    python_or_functor<square_distance_attractive_force> fa(attractive_force);
    python_or_functor<square_distance_repulsive_force> fr(repulsive_force);

    boost::fruchterman_reingold_force_directed_layout
      (g, pos, width, height, 
       boost::vertex_index_map(g.get_vertex_index_map()).
       attractive_force(fa).repulsive_force(fr).
       cooling(cool));
  } else {
    if (cooling != object()) {
      boost::fruchterman_reingold_force_directed_layout
        (g, pos, width, height, 
         boost::vertex_index_map(g.get_vertex_index_map()).
         cooling(cool));
    } else {
      boost::fruchterman_reingold_force_directed_layout
        (g, pos, width, height,
         vertex_index_map(g.get_vertex_index_map()));
    }
  }
}

void export_fruchterman_reingold_force_directed_layout()
{
  using boost::python::arg;
  using boost::python::def;
  using boost::python::object;

  def("fruchterman_reingold_force_directed_layout", 
      &fruchterman_reingold_force_directed_layout<Graph>,
      (arg("graph"), 
       arg("position") = 
         (vector_property_map<point2d, Graph::VertexIndexMap>*)0,
       arg("width") = 500.0,
       arg("height") = 500.0,
       arg("attractive_force") = object(),
       arg("repulsive_force") = object(),
       arg("cooling") = object(),
       arg("progressive") = false));

}

} } } // end namespace boost::graph::python

//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================

#ifndef BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP
#define BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP

namespace boost {

  namespace bgl_detail {
    struct default_param {
      default_param() { }
      template <typename T>
      default_param(const T&) { }
    };
  }

  // Choose the parameter or the default.
  template <typename Default>
  const Default& 
  choose_param(const bgl_detail::default_param&, const Default& d)
    { return d; }
  template <class P, class Default> 
  const P&
  choose_param(const P& param, const Default&) { return param; }

  bool is_default_param(const bgl_detail::default_param&) { return true; }
  template <typename T>
  bool is_default_param(const T&) { return false; }
  
  template <typename WeightMap = bgl_detail::default_param,
            typename DistanceMap = bgl_detail::default_param,
            typename ColorMap = bgl_detail::default_param,
            typename IndexMap = bgl_detail::default_param,
            typename Visitor = bgl_detail::default_param>
  struct bgl_named_params
  {
    bgl_named_params(const WeightMap& wm, const DistanceMap& dm,
		     const ColorMap& cm, const IndexMap& im, 
		     const Visitor& v)
      : m_weight_map(wm), m_dist_map(dm), m_color_map(cm),
	m_vertex_index_map(im), m_visitor(v) { }
    
    template <typename WMap>
    bgl_named_params<WMap,DistanceMap,ColorMap,IndexMap,Visitor>
    weight_map(const WMap& w_map) {
      return bgl_named_params<WMap,DistanceMap,ColorMap,IndexMap,Visitor>
	(w_map, m_dist_map, m_color_map, m_vertex_index_map, m_visitor);
    }
    template <typename DMap>
    bgl_named_params<WeightMap,DMap,ColorMap,IndexMap,Visitor>
    distance_map(const DMap& d_map) {
      return bgl_named_params<WeightMap,DMap,ColorMap,IndexMap,Visitor>
	(m_weight_map, d_map, m_color_map, m_vertex_index_map, m_visitor);
    }
    template <typename CMap>
    bgl_named_params<WeightMap,DistanceMap,CMap,IndexMap,Visitor>
    color_map(const CMap& c_map) {
      return bgl_named_params<WeightMap,DistanceMap,CMap,IndexMap,Visitor>
	(m_weight_map, distance_map, c_map, m_vertex_index_map, m_visitor);
    }
    template <typename IMap>
    bgl_named_params<WeightMap,DistanceMap,ColorMap,IMap,Visitor>
    vertex_index_map(const IMap& i_map) {
      return bgl_named_params<WeightMap,DistanceMap,ColorMap,IMap,Visitor>
	(m_weight_map, distance_map, color_map, i_map, m_visitor);
    }
    template <typename Vis>
    bgl_named_params<WeightMap,DistanceMap,ColorMap,IndexMap,Vis>
    visitor(const Vis& vis) {
      return bgl_named_params<WeightMap,DistanceMap,ColorMap,IndexMap,Vis>
	(m_weight_map, m_dist_map, m_color_map, m_vertex_index_map, vis);
    }
    const WeightMap& m_weight_map;
    const DistanceMap& m_dist_map;
    const ColorMap& m_color_map;
    const IndexMap& m_vertex_index_map;
    const Visitor& m_visitor;
  };
  
  
  template <typename WeightMap>
  bgl_named_params<WeightMap>
  weight_map(const WeightMap& w_map) {
    return bgl_named_params<WeightMap>(w_map);
  }
  template <typename DistMap>
  bgl_named_params<bgl_detail::default_param, DistMap>
  distance_map(const DistMap& d_map)
  {
    bgl_detail::default_param def;
    return bgl_named_params<bgl_detail::default_param, DistMap>
      (def, d_map, def, def, def);
  }
  template <typename ColorMap>
  bgl_named_params<bgl_detail::default_param, bgl_detail::default_param, 
    ColorMap>
  color_map(const ColorMap& c_map)
  {
    bgl_detail::default_param def;
    return bgl_named_params<bgl_detail::default_param, 
      bgl_detail::default_param, ColorMap>(def, def, c_map, def, def);
  }
  template <typename IndexMap>
  bgl_named_params<bgl_detail::default_param, bgl_detail::default_param, 
    bgl_detail::default_param, IndexMap>
  vertex_index_map(const IndexMap& i_map)
  {
    bgl_detail::default_param def;
    return bgl_named_params<bgl_detail::default_param, 
      bgl_detail::default_param, bgl_detail::default_param, IndexMap>
      (def, def, def, i_map, def);
  }
  template <typename Visitor>
  bgl_named_params<bgl_detail::default_param, bgl_detail::default_param, 
    bgl_detail::default_param, bgl_detail::default_param, Visitor>
  use_visitor(const Visitor& vis)
  {
    bgl_detail::default_param def;
    return bgl_named_params<bgl_detail::default_param, 
      bgl_detail::default_param, bgl_detail::default_param, 
      bgl_detail::default_param, Visitor>
      (def, def, def, def, vis);
  }

} // namespace boost

#endif // BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP

//=======================================================================
// Copyright 2001 University of Notre Dame.
// Author: Lie-Quan Lee
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
#ifndef BOOST_GRAPHVIZ_HPP
#define BOOST_GRAPHVIZ_HPP

#include <boost/config.hpp>
#include <string>
#include <map>
#include <iosfwd>
#include <boost/property_map.hpp>
#include <boost/utility.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace boost {

  template <typename directed_category>
  struct graphviz_io_traits {
    static std::string name() {
      return "digraph";
    }
    static std::string delimiter() {
      return "->";
    }  };
  
  struct graphviz_io_traits <boost::undirected_tag> {
    static std::string name() {
      return "graph";
    }
    static std::string delimiter() {
      return "--";
    }
  };


  struct default_graph_writer {
    template <class Graph>
    void operator()(std::ostream&, const Graph&) const {
    }
  };

  struct default_writer {
    template <class VorE, class Graph>
    void operator()(std::ostream&, const VorE&, const Graph&) const {
    }
  };

  template <class Name>
  class labeled_vertex_writer {
  public:
    labeled_vertex_writer(Name _name) : name(_name) {}
    template <class Vertex, class Graph>
    void operator()(std::ostream& out, const Vertex& v, const Graph&) const {
      out << v << " [label=\"" << name[v] << "\"];" << std::endl;
    }
  private:
    Name name;
  };

  enum edge_attribute_t        { edge_attribute        = 1111 };
  enum vertex_attribute_t      { vertex_attribute      = 2222 };
  enum graph_graph_attribute_t { graph_graph_attribute = 3333 };
  enum graph_node_attribute_t  { graph_node_attribute  = 4444 };
  enum graph_edge_attribute_t  { graph_edge_attribute  = 5555 };
  
  BOOST_INSTALL_PROPERTY(edge, attribute);
  BOOST_INSTALL_PROPERTY(vertex, attribute);
  BOOST_INSTALL_PROPERTY(graph, graph_attribute);
  BOOST_INSTALL_PROPERTY(graph, node_attribute);
  BOOST_INSTALL_PROPERTY(graph, edge_attribute);


  template <class Attribute>
  inline void write_attribute(const Attribute& attr, std::ostream& out) {
    typename Attribute::const_iterator i, iend;
    i    = attr.begin();
    iend = attr.end();
    
    bool need_print_pre_post = false;
    if ( i != iend ) 
      need_print_pre_post = true;

    if ( need_print_pre_post )
      out << "[";
    
    while ( i != iend ) {
      out << i->first << "=" << i->second;
      ++i;
      if ( i != iend ) 
        out << ", ";
    }
    
    if ( need_print_pre_post )
      out << "]";
  }

  template <typename GraphGraphAttributes,
            typename GraphNodeAttributes,
            typename GraphEdgeAttributes>
  struct graph_attributes_writer
  {
    graph_attributes_writer(GraphGraphAttributes gg,
                           GraphNodeAttributes gn,
                           GraphEdgeAttributes ge)
      : g_attributes(gg), n_attributes(gn), e_attributes(ge) { }

    void operator()(std::ostream& out) const {
      
      typename GraphGraphAttributes::const_iterator
        gi = g_attributes.begin(),
        gend = g_attributes.end();

      if ( gi != gend ) {
        out << "graph ";
        write_attribute(g_attributes, out);
        out << ";" << std::endl;
      }

      //write node [......]; if need
      typename GraphNodeAttributes::const_iterator
        ni = n_attributes.begin(),
        nend = n_attributes.end();

      if ( ni != nend ) {
        out << "node ";
        write_attribute(n_attributes, out);
        out << ";" << std::endl;
      }

      //write edge[....]; if need
      typename GraphEdgeAttributes::const_iterator 
        ei = e_attributes.begin(),
        eend = e_attributes.end();

      if ( ei != eend ) {
        out << "edge ";
        write_attribute(e_attributes, out);
        out << ";" << std::endl;
      }
    }
    GraphGraphAttributes g_attributes;
    GraphNodeAttributes n_attributes;
    GraphEdgeAttributes e_attributes;
  };

  template <typename GAttrMap, typename NAttrMap, typename EAttrMap>
  graph_attributes_writer<GAttrMap, NAttrMap, EAttrMap>
  make_graph_attributes_writer(const GAttrMap& g_attr, const NAttrMap& n_attr, 
                              const EAttrMap& e_attr) {
    return graph_attributes_writer<GAttrMap, NAttrMap, EAttrMap>
      (g_attr, n_attr, e_attr);
  }


  template <typename Graph>
  graph_attributes_writer
    <typename graph_property<Graph, graph_graph_attribute_t>::type,
     typename graph_property<Graph, graph_node_attribute_t>::type,
     typename graph_property<Graph, graph_edge_attribute_t>::type>
  make_graph_attributes_writer(const Graph& g)
  {
    typedef typename graph_property<Graph, graph_graph_attribute_t>::type 
      GAttrMap;
    typedef typename graph_property<Graph, graph_node_attribute_t>::type
      NAttrMap;
    typedef typename graph_property<Graph, graph_edge_attribute_t>::type
      EAttrMap;
    return graph_attributes_writer<GAttrMap, NAttrMap, EAttrMap>
      (get_property(g, graph_graph_attribute),
       get_property(g, graph_node_attribute),
       get_property(g, graph_edge_attribute));
  }

  template <typename EdgeAttributeMap>
  struct edge_attributes_writer {
    edge_attributes_writer(EdgeAttributeMap e_attr)
      : e_attributes(e_attr) { }

    template <class Edge>
    void operator()(std::ostream& out, const Edge& e) const {
      write_attribute(e_attributes[e], out);
    }
    EdgeAttributeMap e_attributes;
  };

  template <typename Graph>
  edge_attributes_writer
    <typename property_map<Graph, edge_attribute_t>::const_type>
  make_edge_attributes_writer(const Graph& g)
  {
    typedef typename property_map<Graph, edge_attribute_t>::const_type
      EdgeAttributeMap;
    return edge_attributes_writer<EdgeAttributeMap>(get(edge_attribute, g));
  }

  template <typename VertexAttributeMap>
  struct vertex_attributes_writer {
    vertex_attributes_writer(VertexAttributeMap v_attr)
      : v_attributes(v_attr) { }

    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
      typename property_traits<VertexAttributeMap>::value_type::const_iterator 
        v_a_i = v_attributes[v].begin(),
        v_a_end = v_attributes[v].end();

      if ( v_a_i !=  v_a_end ) {
        //print attribute of the vertex
        out << v;
        write_attribute(v_attributes[v], out);
        out << ";" << std::endl;
      }
    }
    VertexAttributeMap v_attributes;
  };

  template <typename Graph>
  vertex_attributes_writer
    <typename property_map<Graph, vertex_attribute_t>::const_type>
  make_vertex_attributes_writer(const Graph& g)
  {
    typedef typename property_map<Graph, vertex_attribute_t>::const_type 
      VertexAttributeMap;
    return vertex_attributes_writer<VertexAttributeMap>
      (get(vertex_attribute, g));
  }

  template <typename Graph, typename VertexPropertiesWriter,
            typename EdgePropertiesWriter, typename GraphPropertiesWriter>

  inline void write_graphviz(std::ostream& out, const Graph& g,
                             VertexPropertiesWriter vpw,
                             EdgePropertiesWriter epw,
                             GraphPropertiesWriter gpw) {
    
    typedef typename boost::graph_traits<Graph>::directed_category CAT;
    typedef graphviz_io_traits<CAT> Traits;
    std::string function_name = "G";
    out << Traits::name() << " " << function_name << " {" << std::endl;

    gpw(out, g); //print graph properties
   
    typename boost::graph_traits<Graph>::vertex_iterator i, end;
    typename boost::graph_traits<Graph>::out_edge_iterator ei, edge_end;

    for(boost::tie(i,end) = boost::vertices(g); i != end; ++i) {
      boost::tie(ei,edge_end) = out_edges(*i, g);
      bool node_need_printing = false;
      
      if (ei == edge_end )
        node_need_printing = true;

      while ( ei != edge_end ) {
        out << *i << " " << Traits::delimiter() << " " << target(*ei, g);
        epw(out, *ei, g); //print edge properties
        out << ";" << std::endl;
        ++ei;
      }

      if ( node_need_printing )
        out << *i << ";" << std::endl;

      vpw(out, *i, g); //print vertex properties 
    } // the loop for vertices
    

    out << "}" << std::endl;
  }

  template <typename Graph>
  inline void 
  write_graphviz(std::ostream& out, const Graph& g) {
    default_writer dw;
    default_graph_writer gw;
    write_graphviz(out, g, dw, dw, gw);
  }

  template <typename Graph, typename VertexWriter>
  inline void 
  write_graphviz(std::ostream& out, const Graph& g, VertexWriter vw) {
    default_writer dw;
    default_graph_writer gw;
    write_graphviz(out, g, vw, dw, gw);
  }

  template <typename Graph, typename VertexWriter, typename EdgeWriter>
  inline void
  write_graphviz(std::ostream& out, const Graph& g, 
                 VertexWriter vw, EdgeWriter ew) {
    default_graph_writer gw;
    write_graphviz(out, g, vw, ew, gw);
  }

  typedef std::map<std::string, std::string> GraphvizAttrList;
  typedef boost::subgraph<boost::adjacency_list<boost::vecS, 
    boost::vecS, boost::directedS, 
    boost::property<boost::vertex_attribute_t, GraphvizAttrList>, 
    boost::property<boost::edge_attribute_t, GraphvizAttrList, 
    boost::property<boost::edge_index_t, int> >, 
    boost::property<boost::graph_graph_attribute_t, GraphvizAttrList, 
    boost::property<boost::graph_node_attribute_t, GraphvizAttrList, 
    boost::property<boost::graph_edge_attribute_t, GraphvizAttrList, 
    boost::property<boost::graph_name_t, std::string> > > > > >
  GraphvizGraph;

  extern void read_graphviz(GraphvizGraph& g, const std::string& file);
  extern void write_graphviz(const GraphvizGraph& g, const std::string& file);

} //namespace boost
  
#endif // BOOST_GRAPHVIZ_HPP

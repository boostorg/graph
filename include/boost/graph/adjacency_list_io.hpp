// François Faure, iMAGIS-GRAVIR / UJF, 2001
#ifndef ______adj_list_io_______
#define ______adj_list_io_______

#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>

// Method parse to read an adjacency list from an input stream,
// example: cin >> read( G );
//
// Method print to write an adjacency list to an output stream,
// example: cout << write( G );
//
// The adjacency list may include interior vertex and edge properties.

namespace boost {

//===========================================================================
// basic io
template<class Tag, class Value, class Next>
std::ostream& operator << ( std::ostream& out, const property<Tag,Value,Next>& p )
{
  out << p.m_value << " " << Next(p);
  return out;
}

template<class Tag, class Value>
std::ostream& operator << ( std::ostream& out, const property<Tag,Value,no_property>& p )
{
  out << p.m_value;
  return out;
}

std::ostream& operator << ( std::ostream& out, const no_property& )
{
  return out;
}


template<class Tag, class Value, class Next>
std::istream& operator >> ( std::istream& in, property<Tag,Value,Next>& p )
{
  in >> p.m_value >> *(static_cast<Next*>(&p)); // houpla !!
  return in;
}

template<class Tag, class Value>
std::istream& operator >> ( std::istream& in, property<Tag,Value,no_property>& p )
{
  in >> p.m_value;
  return in;
}

std::istream& operator >> ( std::istream& in, no_property& )
{
  return in;
}

// basic io
//===========================================================================
// graph parser

template<class Graph_t, class VertexProperty, class EdgeProperty>
struct GraphParser
{

  typedef Graph_t Graph;
  
  GraphParser( Graph* g ): graph(g)
  {}  
  
  GraphParser& operator () ( std::istream& in )
  {
    typedef graph_traits<Graph>::vertex_descriptor Vertex;
    std::vector<Vertex> nodes;

    typedef enum{ PARSE_NUM_NODES, PARSE_VERTEX, PARSE_EDGE } State;
    State state = PARSE_VERTEX;

    unsigned int numLine = 1;
    char c;
    while ( in.get(c) )
    {
      if( c== '#' ) skip(in);
      else if( c== 'n' ) state = PARSE_NUM_NODES;
      else if( c== 'v' ) state = PARSE_VERTEX;
      else if( c== 'e' ) state = PARSE_EDGE;
      else if( c== '\n' ) numLine++;
      else if( !isspace(c) ){
        in.putback(c);
        if( state == PARSE_VERTEX ){
          VertexProperty vp;
          if( in >> vp )
            nodes.push_back( add_vertex(vp, *graph) );
          else
            cerr<<"readVertex, parse error at line "<<numLine<<endl;
        }
        else if( state == PARSE_EDGE ) {
          int source, target;
          EdgeProperty ep;
          in >> source >> target;
          if( in >> ep ) 
            add_edge(nodes[source], nodes[target], ep, *graph);
          else
            cerr<<"readEdge, parse error at line "<<numLine<<endl;
        }
        else { // state == PARSE_NUM_NODES
          int n;
          if( in >> n ){
            for( int i=0; i<n; ++i )
              nodes.push_back( add_vertex( *graph ) );
          }
          else 
            cerr<<"read num_nodes, parse error at line " << numLine << endl;
        }
      }
    }
  return (*this);
  }
  
  
protected:

  Graph* graph;
  
  void skip( std::istream& in )
  {
    char c;
    while( c!='\n' && !in.eof() ) 
      in.get(c);
    in.putback(c);
  }
};

template<class Graph, class VP, class EP>
std::istream& operator >> ( std::istream& in, GraphParser<Graph,VP,EP> gp ) 
{ 
  gp(in); 
  return in; 
}

template<class EL, class VL, class D, class VP, class EP, class GP>
GraphParser<adjacency_list<EL,VL,D,VP,EP,GP>,VP,EP> 
read( adjacency_list<EL,VL,D,VP,EP,GP>& g )
{
  return GraphParser<adjacency_list<EL,VL,D,VP,EP,GP>,VP,EP>(&g);
}

// parser
//=======================================================================
// property printer

template<class Graph, class Property>
struct PropertyPrinter
{
  typedef typename Property::value_type Value;
  typedef typename Property::tag_type Tag;
  typedef typename Property::next_type Next;
  
  PropertyPrinter( Graph& g ):graph(&g){}
  
  template<class Iterator>
  PropertyPrinter& operator () ( std::ostream& out, Iterator it )
  {
    typename property_map<Graph,Tag>::type ps = get(Tag(), *graph);
    out << ps[ *it ] <<", ";
    PropertyPrinter<Graph,Next> print(*graph);
    print(out, it);
    return (*this);
  }
private:
  Graph* graph;
};
template<class Graph>
struct PropertyPrinter<Graph, no_property>
{
  PropertyPrinter( Graph& ){}

  template<class Iterator>
  PropertyPrinter& operator () ( std::ostream&, Iterator it ){ return *this; }
};

// property printer
//=========================================================================
// printer

template<class Graph_t, class VertexProperty, class EdgeProperty>
struct EdgePrinter
{

  typedef Graph_t Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  
  EdgePrinter( Graph& g )
    : graph(g)
  {}  
  
  const EdgePrinter& operator () ( std::ostream& out ) const
  {
    // assign indices to vertices
    std::map<Vertex,int> indices;
    int num = 0;
    graph_traits<Graph>::vertex_iterator vi;
    for (vi = vertices(graph).first; vi != vertices(graph).second; ++vi){
      indices[*vi] = num++;
    }

    // write edges
    PropertyPrinter<Graph, EdgeProperty> print_Edge(graph);
    out << "e" << endl;
    graph_traits<Graph>::edge_iterator ei;
    for (ei = edges(graph).first; ei != edges(graph).second; ++ei){
      out << indices[source(*ei,graph)] <<  " " << indices[target(*ei,graph)] << ", "; 
      print_Edge(out,ei); 
      out << endl;
    }
    out << endl;    
    return (*this);
  }
  
protected:

  Graph& graph;
  
};

template<class Graph, class V, class E>
struct GraphPrinter: public EdgePrinter<Graph,V,E>
{
  GraphPrinter( Graph& g )
    : EdgePrinter<Graph,V,E>(g)
  {}
  
  const GraphPrinter& operator () ( std::ostream& out ) const
  {
    PropertyPrinter<Graph, V> printNode(graph);
    out << "v"<<endl;
    graph_traits<Graph>::vertex_iterator vi;
    for (vi = vertices(graph).first; vi != vertices(graph).second; ++vi){
      printNode(out,vi); 
      out << endl;
    }
    out << endl;
    
    EdgePrinter<Graph,V,E>::operator ()( out );
    return (*this);
  }
};

template<class G, class E>
struct GraphPrinter<G,no_property,E> 
  : public EdgePrinter<G,no_property,E>
{
  GraphPrinter( G& g )
    : EdgePrinter<G,no_property,E>(g)
  {}
  
  const GraphPrinter& operator () ( std::ostream& out ) const
  {
    out << "n "<< num_vertices(graph) << endl;
    EdgePrinter<G,no_property,E>::operator ()( out );
    return (*this);
  }
};

template<class Graph, class VP, class EP>
std::ostream& operator << ( std::ostream& out, const GraphPrinter<Graph,VP,EP>& gp ) 
{ 
  gp(out); 
  return out; 
}

template<class EL, class VL, class D, class VP, class EP, class GP>
GraphPrinter<adjacency_list<EL,VL,D,VP,EP,GP>,VP,EP> 
write( adjacency_list<EL,VL,D,VP,EP,GP>& g )
{
  return GraphPrinter<adjacency_list<EL,VL,D,VP,EP,GP>,VP,EP>(g);
}
// printer
//=========================================================================

}// boost



#endif

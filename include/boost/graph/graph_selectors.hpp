#ifndef BOOST_GRAPH_SELECTORS_HPP
#define BOOST_GRAPH_SELECTORS_HPP

namespace boost {

  //===========================================================================
  // Selectors for the Directed template parameter of adjacency_list
  // and adjacency_matrix.

  struct directedS { enum { is_directed = true, is_bidir = false }; 
    typedef true_type is_directed_t; 
    typedef false_type is_bidir_t;
  };
  struct undirectedS { 
    enum { is_directed = false, is_bidir = false }; 
    typedef false_type is_directed_t;
    typedef false_type is_bidir_t;
  };
  struct bidirectionalS { 
    enum { is_directed = true, is_bidir = true }; 
    typedef true_type is_directed_t;
    typedef true_type is_bidir_t;
  };

} // namespace boost

#endif // BOOST_GRAPH_SELECTORS_HPP

#ifndef BOOST_GRAPH_DETAIL_CONTAINER_TRAITS_H
#define BOOST_GRAPH_DETAIL_CONTAINER_TRAITS_H

// Sure would be nice to be able to forward declare these
// instead of pulling in all the headers. Too bad that
// is not legal. There ought to be a standard <stlfwd> header. -JGS 

#include <vector>
#include <list>
#include <map>
#include <set>
#ifndef BOOST_NO_SLIST
#  include <slist>
#endif
#ifndef BOOST_NO_HASH
#  include <hash_map>
#  include <hash_set>
#endif

namespace boost {

  //======================================================================
  // Container Category Tags
  //
  //   They use virtual inheritance because there are lots of
  //   inheritance diamonds.

  struct container_tag { };
  struct forward_container_tag : virtual public container_tag { };
  struct reversible_container_tag : virtual public forward_container_tag { };
  struct random_access_container_tag
    : virtual public reversible_container_tag { };
  
  struct sequence_tag : virtual public forward_container_tag { };

  struct associative_container_tag : virtual public forward_container_tag { };

  struct sorted_associative_container_tag 
    : virtual public associative_container_tag,
      virtual public reversible_container_tag { };

  struct front_insertion_sequence_tag : virtual public sequence_tag { };
  struct back_insertion_sequence_tag : virtual public sequence_tag { };

  struct unique_associative_container_tag 
    : virtual public associative_container_tag { };
  struct multiple_associative_container_tag 
    : virtual public associative_container_tag { };
  struct simple_associative_container_tag 
    : virtual public associative_container_tag { };
  struct pair_associative_container_tag 
    : virtual public associative_container_tag { };


  //======================================================================
  // Container Traits Class and container_category() function

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  // don't use this unless there is partial specialization 
  template <class Container>
  struct container_traits {
    typedef typename Container::category category;
  };
#endif

  // std::vector
  struct vector_tag :
    virtual public random_access_container_tag,
    virtual public back_insertion_sequence_tag { };

  template <class T, class Alloc>
  vector_tag container_category(const std::vector<T,Alloc>&)
    { return vector_tag(); }

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class T, class Alloc>
  struct container_traits< std::vector<T,Alloc> > {
    typedef vector_tag category;
  };
#endif

  // std::list
  struct list_tag :
    virtual public reversible_container_tag,
    virtual public back_insertion_sequence_tag
    // this causes problems for push_dispatch...
    //    virtual public front_insertion_sequence_tag
    { };

  template <class T, class Alloc>
  list_tag container_category(const std::list<T,Alloc>&)
    { return list_tag(); }

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class T, class Alloc>
  struct container_traits< std::list<T,Alloc> > {
    typedef list_tag category;
  };
#endif


  // std::slist
#ifndef BOOST_NO_SLIST
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class T, class Alloc>
  struct container_traits< std::slist<T,Alloc> > {
    typedef front_insertion_sequence_tag category;
  };
#endif
  template <class T, class Alloc>
  front_insertion_sequence_tag container_category(const std::slist<T,Alloc>&)
    { return front_insertion_sequence_tag(); }
#endif


  // std::set
  struct set_tag :
    virtual public sorted_associative_container_tag,
    virtual public simple_associative_container_tag,
    virtual public unique_associative_container_tag 
    { };

  template <class Key, class Cmp, class Alloc> 
  set_tag container_category(const std::set<Key,Cmp,Alloc>&)
  { return set_tag(); }

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class Key, class Cmp, class Alloc> 
  struct container_traits< std::set<Key,Cmp,Alloc> > {
    typedef set_tag category;
  };
#endif

  // deque

  // std::map
  struct map_tag :
    virtual public sorted_associative_container_tag,
    virtual public pair_associative_container_tag,
    virtual public multiple_associative_container_tag 
    { };

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class Key, class T, class Cmp, class Alloc> 
  struct container_traits< std::map<Key,T,Cmp,Alloc> > {
    typedef map_tag category;
  };
#endif

  template <class Key, class T, class Cmp, class Alloc> 
  map_tag container_category(const std::map<Key,T,Cmp,Alloc>&)
  { return map_tag(); }


 // hash_set, hash_map

#ifndef BOOST_NO_HASH
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class Key, class Eq, class Hash, class Alloc> 
  struct container_traits< std::hash_set<Key,Eq,Hash,Alloc> > {
    typedef set_tag category;
  };
  template <class Key, class T, class Eq, class Hash, class Alloc>
  struct container_traits< std::hash_map<Key,T,Eq,Hash,Alloc> > {
    typedef map_tag category;
  };
#endif
  template <class Key, class Eq, class Hash, class Alloc>
  set_tag container_category(const std::hash_set<Key,Eq,Hash,Alloc>&)
  { return set_tag(); }

  template <class Key, class T, class Eq, class Hash, class Alloc>
  map_tag container_category(const std::hash_map<Key,T,Eq,Hash,Alloc>&)
  { return map_tag(); }
#endif



  //===========================================================================
  // Generalized Container Functions


  // Erase
  template <class Sequence, class T>
  void erase_dispatch(Sequence& c, const T& x, 
                      sequence_tag)
  {
    c.erase(std::remove(c.begin(), c.end(), x), c.end());
  }

  template <class AssociativeContainer, class T>
  void erase_dispatch(AssociativeContainer& c, const T& x, 
		      associative_container_tag)
  {
    c.erase(x);
  }
  template <class Container, class T>
  void erase(Container& c, const T& x)
  {
    erase_dispatch(c, x, container_category(c));
  }

  // Push
  template <class Container, class T>
  std::pair<typename Container::iterator, bool>
  push_dispatch(Container& c, const T& v, back_insertion_sequence_tag)
  {
    c.push_back(v);
    return std::make_pair(boost::prior(c.end()), true);
  }

  template <class Container, class T>
  std::pair<typename Container::iterator, bool>
  push_dispatch(Container& c, const T& v, front_insertion_sequence_tag)
  {
    c.push_front(v);
    return std::make_pair(c.begin(), true);
  }

  template <class AssociativeContainer, class T>
  std::pair<typename AssociativeContainer::iterator, bool>
  push_dispatch(AssociativeContainer& c, const T& v, associative_container_tag)
  {
    return c.insert(v);
  }

  template <class Container, class T>
  std::pair<typename Container::iterator,bool>
  push(Container& c, const T& v)
  {
    return push_dispatch(c, v, container_category(c));
  }

} // namespace boost

#endif // BOOST_GRAPH_DETAIL_CONTAINER_TRAITS_H

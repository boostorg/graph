#ifndef BOOST_ALGORITHM_HPP
#define BOOST_ALGORITHM_HPP

#include <algorithm>

namespace boost {

  template <typename Iter1, typename Iter2>
  Iter1 begin(const std::pair<Iter1, Iter2>& p) { return p.first; }

  template <typename Iter1, typename Iter2>
  Iter2 end(const std::pair<Iter1, Iter2>& p) { return p.second; }

  template <typename Iter1, typename Iter2>
  typename std::iterator_traits<Iter1>::difference_type
  size(const std::pair<Iter1, Iter2>& p) {
    return std::distance(p.first, p.second);
  }

#if 0
  // These seem to interfere with the std::pair overloads :(
  template <typename Container>
  typename Container::iterator
  begin(Container& c) { return c.begin(); }

  template <typename Container>
  typename Container::const_iterator
  begin(const Container& c) { return c.begin(); }

  template <typename Container>
  typename Container::iterator
  end(Container& c) { return c.end(); }

  template <typename Container>
  typename Container::const_iterator
  end(const Container& c) { return c.end(); }

  template <typename Container>
  typename Container::size_type
  size(const Container& c) { return c.size(); }
#else
  template <typename T>
  typename std::vector<T>::iterator
  begin(std::vector<T>& c) { return c.begin(); }

  template <typename T>
  typename std::vector<T>::const_iterator
  begin(const std::vector<T>& c) { return c.begin(); }

  template <typename T>
  typename std::vector<T>::iterator
  end(std::vector<T>& c) { return c.end(); }

  template <typename T>
  typename std::vector<T>::const_iterator
  end(const std::vector<T>& c) { return c.end(); }

  template <typename T>
  typename std::vector<T>::size_type
  size(const std::vector<T>& c) { return c.size(); }
#endif
  
  // Also do version with 2nd container?
  template <typename Container, typename OutIter>
  OutIter copy(const Container& c, OutIter result) {
    return std::copy(begin(c), end(c), result);
  }

  template <typename Container1, typename Container2>
  bool equal(const Container1& c1, const Container2& c2)
  {
    if (size(c1) != size(c2))
      return false;
    return std::equal(begin(c1), end(c1), begin(c2));
  }

  template <typename Container>
  void sort(Container& c) { std::sort(begin(c), end(c)); }

  template <typename Container, typename Predicate>
  void sort(Container& c, const Predicate& p) { 
    std::sort(begin(c), end(c), p);
  }

  template <typename Container>
  void stable_sort(Container& c) { std::stable_sort(begin(c), end(c)); }

  template <typename Container, typename Predicate>
  void stable_sort(Container& c, const Predicate& p) { 
    std::stable_sort(begin(c), end(c), p);
  }

  template <typename InputIterator, typename T>
  bool any_equal(InputIterator first, InputIterator last, T value)
  {
    return std::find(first, last, value) != last;
  }

  template <typename Container, typename T>
  bool any_equal(const Container& c, const T& value)
  {
    return any_equal(begin(c), end(c), value);
  }


  template <typename Container, typename T>
  std::size_t count(const Container& c, const T& value)
  {
    return std::count(begin(c), end(c), value);
  }

  template <typename Container, typename Predicate>
  std::size_t count_if(const Container& c, Predicate p)
  {
    return std::count_if(begin(c), end(c), p);
  }

} // namespace boost

#endif // BOOST_ALGORITHM_HPP
